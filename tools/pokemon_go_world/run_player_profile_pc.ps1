param(
    [ValidateRange(1, 2147483647)]
    [int]$Profile = 1,
    [string]$ImportSave,
    [string]$ExportSave,
    [switch]$AllowExportOverwrite,
    [string]$DataRoot,
    [ValidateRange(0, 3)]
    [int]$RestoreRecovery = 0,
    [switch]$ListRecoveries,
    [switch]$PassThru,
    [switch]$GetProfileInfo,
    [string]$SetProfileName,
    [switch]$ResetProfile,
    [switch]$CreateProfile,
    [switch]$DeleteProfile,
    [ValidateRange(-1, 3)]
    [int]$FavoriteFromSlot = -1,
    [ValidateRange(0, 5)]
    [int]$RestoreFavorite = 0,
    [ValidateRange(0, 5)]
    [int]$RemoveFavorite = 0,
    [switch]$PrepareOnly
)

$ErrorActionPreference = 'Stop'

. (Join-Path $PSScriptRoot 'save_container_common_pc.ps1')

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$executable = Join-Path $repoRoot 'pokemon_regionalidades-pc.exe'
$resourcePack = Join-Path $repoRoot 'pokemon_regionalidades.pak'
$sdl = Join-Path $repoRoot 'SDL2.dll'

foreach ($requiredFile in @($executable, $resourcePack, $sdl)) {
    if (-not (Test-Path -LiteralPath $requiredFile -PathType Leaf)) {
        throw "Arquivo necessario nao encontrado: $requiredFile"
    }
}

if ([string]::IsNullOrWhiteSpace($DataRoot)) {
    $localData = [Environment]::GetFolderPath([Environment+SpecialFolder]::LocalApplicationData)
    if ([string]::IsNullOrWhiteSpace($localData)) {
        throw 'O Windows nao informou o diretorio local de dados do usuario.'
    }
    $DataRoot = Join-Path $localData 'Pokemon Regionalidades'
} else {
    $DataRoot = [IO.Path]::GetFullPath($DataRoot)
}

$profileDir = Join-Path $DataRoot ("profiles\profile-$Profile")
$configDir = Join-Path $DataRoot 'config'
$savePath = Join-Path $profileDir 'pokemon_regionalidades.pgrsave'
$legacySavePath = Join-Path $profileDir 'pokemon_regionalidades.sav'
$configPath = Join-Path $configDir 'pokemon_regionalidades.cfg'
$runtimeLog = Join-Path $profileDir 'runtime-last.log'
$runtimeHistoryDir = Join-Path $profileDir 'runtime-history'
$profileMetadataPath = Join-Path $profileDir 'profile.json'
$favoritesDir = Join-Path $profileDir 'favorites'

if ($CreateProfile -and (Test-Path -LiteralPath $profileDir)) {
    throw "O perfil $Profile ja existe. Nenhum dado foi alterado."
}
if ($DeleteProfile -and -not (Test-Path -LiteralPath $profileDir -PathType Container)) {
    throw "O perfil $Profile nao existe."
}

New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
New-Item -ItemType Directory -Path $configDir -Force | Out-Null

function Get-ProfileMetadata {
    $defaultName = "Perfil $Profile"
    if (-not (Test-Path -LiteralPath $profileMetadataPath -PathType Leaf)) {
        $oldTestProfile = $Profile -eq 1 -and
            ((Test-Path -LiteralPath $savePath -PathType Leaf) -or
             (Test-Path -LiteralPath $legacySavePath -PathType Leaf))
        return [pscustomobject]@{ SchemaVersion = 1; DisplayName = $defaultName; TechnicalMobility = $oldTestProfile }
    }
    try {
        $metadata = Get-Content -Raw -LiteralPath $profileMetadataPath | ConvertFrom-Json
        if ($metadata.SchemaVersion -ne 1 -or
            [string]::IsNullOrWhiteSpace([string]$metadata.DisplayName) -or
            ([string]$metadata.DisplayName).Length -gt 32) {
            throw 'Metadados invalidos.'
        }
        return [pscustomobject]@{
            SchemaVersion = 1
            DisplayName = ([string]$metadata.DisplayName).Trim()
            TechnicalMobility = if ($null -eq $metadata.TechnicalMobility) { $Profile -eq 1 } else { [bool]$metadata.TechnicalMobility }
        }
    } catch {
        throw "Os metadados do perfil $Profile estao invalidos: $profileMetadataPath"
    }
}

function Set-ProfileMetadataName {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Nullable[bool]]$TechnicalMobility = $null
    )

    $normalized = $Name.Trim()
    if ([string]::IsNullOrWhiteSpace($normalized) -or $normalized.Length -gt 32 -or
        $normalized.IndexOfAny([char[]]"`r`n`t") -ge 0) {
        throw 'O nome do perfil deve ter entre 1 e 32 caracteres e ocupar uma unica linha.'
    }
    $previous = Get-ProfileMetadata
    $useTechnicalMobility = if ($null -ne $TechnicalMobility) { [bool]$TechnicalMobility } else { [bool]$previous.TechnicalMobility }
    $metadata = [ordered]@{
        SchemaVersion = 1
        DisplayName = $normalized
        TechnicalMobility = $useTechnicalMobility
    }
    $pendingPath = "$profileMetadataPath.pending"
    $json = $metadata | ConvertTo-Json
    $utf8Bom = New-Object Text.UTF8Encoding($true)
    [IO.File]::WriteAllText($pendingPath, $json, $utf8Bom)
    try {
        if (Test-Path -LiteralPath $profileMetadataPath -PathType Leaf) {
            $metadataBackup = "$profileMetadataPath.before-update-$(Get-Date -Format 'yyyyMMdd-HHmmssfff')"
            [IO.File]::Replace($pendingPath, $profileMetadataPath, $metadataBackup, $true)
        } else {
            Move-Item -LiteralPath $pendingPath -Destination $profileMetadataPath
        }
    } finally {
        if (Test-Path -LiteralPath $pendingPath -PathType Leaf) {
            Remove-Item -LiteralPath $pendingPath -Force
        }
    }
    return Get-ProfileMetadata
}

function Resolve-ProfileSavePath {
    param([int]$Recovery = 0)
    $suffix = if ($Recovery -eq 0) { '' } else { ".recovery-$Recovery" }
    $nativeCandidate = "$savePath$suffix"
    $legacyCandidate = "$legacySavePath$suffix"
    if (Test-RegionalidadesSave -Path $nativeCandidate) {
        return $nativeCandidate
    }
    if (Test-RegionalidadesSave -Path $legacyCandidate) {
        return $legacyCandidate
    }
    return $null
}

function Resolve-FavoritePath {
    param([ValidateRange(1, 5)][int]$Slot)
    return Join-Path $favoritesDir ("favorite-$Slot.pgrsave")
}

function Restore-ProfileSave {
    param(
        [Parameter(Mandatory = $true)][string]$SourcePath,
        [Parameter(Mandatory = $true)][string]$Description
    )
    if (Get-Process -Name 'pokemon_regionalidades-pc' -ErrorAction SilentlyContinue) {
        throw 'Feche todas as instancias do jogo antes de restaurar um save.'
    }
    $activePath = Resolve-ProfileSavePath
    if ($null -eq $activePath -or -not (Test-RegionalidadesSave -Path $activePath)) {
        throw "O perfil $Profile nao possui um save ativo valido."
    }
    if (-not (Test-RegionalidadesSave -Path $SourcePath)) {
        throw "$Description nao existe ou e invalido."
    }
    if ($activePath.Equals($legacySavePath, [StringComparison]::OrdinalIgnoreCase) -and
        (Get-RegionalidadesSaveDescriptor -Path $SourcePath).Kind -eq 'Native') {
        throw 'Abra o perfil para migrar o save legado antes de restaurar um favorito nativo.'
    }
    $activeHash = (Get-FileHash -LiteralPath $activePath -Algorithm SHA256).Hash
    $sourceHash = (Get-FileHash -LiteralPath $SourcePath -Algorithm SHA256).Hash
    $pendingRestore = "$activePath.restoring"
    $backupPath = "$activePath.before-restore-$(Get-Date -Format 'yyyyMMdd-HHmmssfff')"
    if (Test-Path -LiteralPath $pendingRestore) {
        throw 'Ha uma restauracao pendente. Revise os arquivos antes de continuar.'
    }
    Copy-Item -LiteralPath $SourcePath -Destination $pendingRestore
    if ((Get-FileHash -LiteralPath $pendingRestore -Algorithm SHA256).Hash -ne $sourceHash) {
        Remove-Item -LiteralPath $pendingRestore -Force
        throw 'A copia preparada para restauracao nao corresponde ao save escolhido.'
    }
    try {
        [IO.File]::Replace($pendingRestore, $activePath, $backupPath, $true)
    } catch {
        if (Test-Path -LiteralPath $pendingRestore -PathType Leaf) {
            Remove-Item -LiteralPath $pendingRestore -Force
        }
        throw
    }
    if ((Get-FileHash -LiteralPath $activePath -Algorithm SHA256).Hash -ne $sourceHash -or
        (Get-FileHash -LiteralPath $backupPath -Algorithm SHA256).Hash -ne $activeHash) {
        throw 'A verificacao posterior da restauracao falhou. Nao abra o jogo antes de revisar os arquivos.'
    }
    Write-Host "$Description restaurado no perfil $Profile. Save anterior preservado em: $backupPath"
}

if ($CreateProfile) {
    [void](Set-ProfileMetadataName -Name "Perfil $Profile")
    Write-Host "Perfil $Profile criado."
    return
}

if ($DeleteProfile) {
    if (Get-Process -Name 'pokemon_regionalidades-pc' -ErrorAction SilentlyContinue) {
        throw 'Feche todas as instancias do jogo antes de apagar um perfil.'
    }
    $archiveRoot = Join-Path $DataRoot 'profiles-archived'
    $resolvedDataRoot = [IO.Path]::GetFullPath($DataRoot).TrimEnd([IO.Path]::DirectorySeparatorChar)
    $resolvedProfile = [IO.Path]::GetFullPath($profileDir)
    $resolvedArchive = [IO.Path]::GetFullPath($archiveRoot)
    $safePrefix = "$resolvedDataRoot$([IO.Path]::DirectorySeparatorChar)"
    if (-not $resolvedProfile.StartsWith($safePrefix, [StringComparison]::OrdinalIgnoreCase) -or
        -not $resolvedArchive.StartsWith($safePrefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Os caminhos do perfil e do arquivo nao estao dentro dos dados do jogo.'
    }
    New-Item -ItemType Directory -Path $archiveRoot -Force | Out-Null
    $archivePath = Join-Path $archiveRoot ("profile-$Profile-$(Get-Date -Format 'yyyyMMdd-HHmmssfff')-$([guid]::NewGuid().ToString('N'))")
    Move-Item -LiteralPath $profileDir -Destination $archivePath
    $result = [pscustomobject]@{ Profile = $Profile; BackupPath = $archivePath }
    if ($PassThru) { $result } else { Write-Host "Perfil $Profile arquivado em: $archivePath" }
    return
}

if ($FavoriteFromSlot -ge 0) {
    $sourcePath = Resolve-ProfileSavePath -Recovery $FavoriteFromSlot
    if ($null -eq $sourcePath) {
        throw 'O save escolhido para favoritar nao existe ou e invalido.'
    }
    $slot = 1
    while ($slot -le 5 -and (Test-Path -LiteralPath (Resolve-FavoritePath -Slot $slot))) { $slot++ }
    if ($slot -gt 5) { throw 'Este perfil ja possui cinco favoritos.' }
    New-Item -ItemType Directory -Path $favoritesDir -Force | Out-Null
    $destination = Resolve-FavoritePath -Slot $slot
    $pending = "$destination.pending"
    if (Test-Path -LiteralPath $pending) { throw 'Ha um favorito pendente. Revise os arquivos antes de continuar.' }
    $descriptor = Get-RegionalidadesSaveDescriptor -Path $sourcePath
    if ($descriptor.Kind -eq 'Native') {
        Copy-Item -LiteralPath $sourcePath -Destination $pending
    } else {
        $nativeBytes = Convert-RegionalidadesLegacyToNativeBytes -LegacyBytes ([IO.File]::ReadAllBytes($sourcePath))
        [IO.File]::WriteAllBytes($pending, $nativeBytes)
    }
    if (-not (Test-RegionalidadesSave -Path $pending)) {
        Remove-Item -LiteralPath $pending -Force
        throw 'O favorito preparado nao passou pela validacao.'
    }
    $hash = (Get-FileHash -LiteralPath $pending -Algorithm SHA256).Hash
    Move-Item -LiteralPath $pending -Destination $destination
    if ((Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash -ne $hash) {
        throw 'A verificacao posterior do favorito falhou.'
    }
    Write-Host "Favorito $slot criado no perfil $Profile."
    return
}

if ($RemoveFavorite -gt 0) {
    $favoritePath = Resolve-FavoritePath -Slot $RemoveFavorite
    if (-not (Test-Path -LiteralPath $favoritePath -PathType Leaf)) {
        throw 'O favorito escolhido nao existe.'
    }
    $removedDir = Join-Path $profileDir 'favorites-removed'
    New-Item -ItemType Directory -Path $removedDir -Force | Out-Null
    $removedPath = Join-Path $removedDir ("favorite-$RemoveFavorite-$(Get-Date -Format 'yyyyMMdd-HHmmssfff')-$([guid]::NewGuid().ToString('N')).pgrsave")
    Move-Item -LiteralPath $favoritePath -Destination $removedPath
    Write-Host "Favorito $RemoveFavorite removido da lista e preservado em: $removedPath"
    return
}

if ($GetProfileInfo) {
    $metadata = Get-ProfileMetadata
    $activePath = Resolve-ProfileSavePath
    $result = [pscustomobject]@{
        Profile = $Profile
        DisplayName = $metadata.DisplayName
        HasActiveSave = $null -ne $activePath
        ActiveSavePath = $activePath
    }
    if ($PassThru) {
        $result
    } else {
        $result | Format-List
    }
    return
}

if (-not [string]::IsNullOrWhiteSpace($SetProfileName)) {
    $result = Set-ProfileMetadataName -Name $SetProfileName
    if ($PassThru) {
        $result
    } else {
        Write-Host "Perfil $Profile renomeado para: $($result.DisplayName)"
    }
    return
}

if ($ResetProfile) {
    if (Get-Process -Name 'pokemon_regionalidades-pc' -ErrorAction SilentlyContinue) {
        throw 'Feche todas as instancias do jogo antes de reiniciar um perfil.'
    }
    $activePath = Resolve-ProfileSavePath
    if ($null -eq $activePath) {
        throw "O perfil $Profile ainda nao possui uma campanha para reiniciar."
    }

    $metadata = Get-ProfileMetadata
    $profilesRoot = Split-Path $profileDir
    $resetPath = Join-Path $profilesRoot ("profile-$Profile-reset-{0}" -f (Get-Date -Format 'yyyyMMdd-HHmmssfff'))
    Move-Item -LiteralPath $profileDir -Destination $resetPath
    try {
        New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
        [void](Set-ProfileMetadataName -Name $metadata.DisplayName -TechnicalMobility $metadata.TechnicalMobility)
    } catch {
        $resetFailurePath = "$resetPath.failed-new-profile"
        if (Test-Path -LiteralPath $profileDir -PathType Container) {
            Move-Item -LiteralPath $profileDir -Destination $resetFailurePath
        }
        Move-Item -LiteralPath $resetPath -Destination $profileDir
        throw 'O reinicio falhou e a campanha original foi restaurada.'
    }
    if ($null -ne (Resolve-ProfileSavePath)) {
        throw 'O reinicio nao conseguiu retirar todos os saves da campanha ativa.'
    }
    $result = [pscustomobject]@{ Profile = $Profile; BackupPath = $resetPath }
    if ($PassThru) {
        $result
    } else {
        Write-Host "Perfil $Profile reiniciado. A campanha anterior foi preservada em: $resetPath"
    }
    return
}

if (-not [string]::IsNullOrWhiteSpace($ImportSave)) {
    $sourceSave = (Resolve-Path -LiteralPath $ImportSave).Path
    if (-not (Test-RegionalidadesSave -Path $sourceSave)) {
        throw "O save de origem nao e um .sav Emerald ou .pgrsave reconhecido: $sourceSave"
    }
    if ($null -ne (Resolve-ProfileSavePath)) {
        throw "O perfil $Profile ja possui um save. A importacao foi cancelada sem substituir o arquivo existente."
    }

    $sourceDescriptor = Get-RegionalidadesSaveDescriptor -Path $sourceSave
    $importDestination = $savePath
    $pendingImport = "$importDestination.importing"
    $sourceHash = (Get-FileHash -LiteralPath $sourceSave -Algorithm SHA256).Hash
    if ($sourceDescriptor.Kind -eq 'Native') {
        Copy-Item -LiteralPath $sourceSave -Destination $pendingImport -Force
    } else {
        $nativeBytes = Convert-RegionalidadesLegacyToNativeBytes -LegacyBytes ([IO.File]::ReadAllBytes($sourceSave))
        [IO.File]::WriteAllBytes($pendingImport, $nativeBytes)
    }
    if (-not (Test-RegionalidadesSave -Path $pendingImport) -or
        (Get-FileHash -LiteralPath $sourceSave -Algorithm SHA256).Hash -ne $sourceHash) {
        Remove-Item -LiteralPath $pendingImport -Force
        throw 'A copia importada falhou na validacao ou alterou o save de origem.'
    }
    Move-Item -LiteralPath $pendingImport -Destination $importDestination
    Write-Host "Save copiado com seguranca para o perfil $Profile. O original foi preservado."
}

if (-not [string]::IsNullOrWhiteSpace($ExportSave)) {
    $activeSavePath = Resolve-ProfileSavePath
    if ($null -eq $activeSavePath) {
        throw "O perfil $Profile nao possui um save ativo valido para exportar."
    }

    $exportPath = [IO.Path]::GetFullPath($ExportSave)
    if ($exportPath.Equals([IO.Path]::GetFullPath($activeSavePath), [StringComparison]::OrdinalIgnoreCase)) {
        throw 'O destino da exportacao nao pode ser o proprio save ativo.'
    }
    $exportDir = Split-Path $exportPath
    if (-not (Test-Path -LiteralPath $exportDir -PathType Container)) {
        throw "A pasta escolhida para exportacao nao existe: $exportDir"
    }
    if ((Test-Path -LiteralPath $exportPath -PathType Leaf) -and -not $AllowExportOverwrite) {
        throw 'O arquivo de destino ja existe. A exportacao foi cancelada sem substitui-lo.'
    }

    $pendingExport = "$exportPath.exporting-$([guid]::NewGuid().ToString('N'))"
    $activeDescriptor = Get-RegionalidadesSaveDescriptor -Path $activeSavePath
    if ($activeDescriptor.Kind -eq 'Native') {
        Copy-Item -LiteralPath $activeSavePath -Destination $pendingExport
    } else {
        $nativeBytes = Convert-RegionalidadesLegacyToNativeBytes -LegacyBytes ([IO.File]::ReadAllBytes($activeSavePath))
        [IO.File]::WriteAllBytes($pendingExport, $nativeBytes)
    }
    if (-not (Test-RegionalidadesSave -Path $pendingExport)) {
        Remove-Item -LiteralPath $pendingExport -Force
        throw 'A copia preparada para exportacao nao e um save nativo valido.'
    }
    $sourceHash = (Get-FileHash -LiteralPath $pendingExport -Algorithm SHA256).Hash

    $previousExportBackup = $null
    if (Test-Path -LiteralPath $exportPath -PathType Leaf) {
        $previousExportBackup = "$exportPath.before-export-$(Get-Date -Format 'yyyyMMdd-HHmmssfff')"
        [IO.File]::Replace($pendingExport, $exportPath, $previousExportBackup, $true)
    } else {
        Move-Item -LiteralPath $pendingExport -Destination $exportPath
    }

    if (-not (Test-RegionalidadesSave -Path $exportPath) -or
        (Get-FileHash -LiteralPath $exportPath -Algorithm SHA256).Hash -ne $sourceHash) {
        throw 'A verificacao posterior da exportacao falhou.'
    }

    Write-Host "Save do perfil $Profile exportado para: $exportPath"
    if ($null -ne $previousExportBackup) {
        Write-Host "O arquivo exportado anterior foi preservado em: $previousExportBackup"
    }
    return
}

if ($RestoreRecovery -ne 0 -and $RestoreFavorite -ne 0) {
    throw 'Escolha somente uma origem para restaurar.'
}
if ($RestoreRecovery -ne 0) {
    $recoveryPath = Resolve-ProfileSavePath -Recovery $RestoreRecovery
    if ($null -eq $recoveryPath) { throw "A recuperacao $RestoreRecovery nao existe ou e invalida." }
    Restore-ProfileSave -SourcePath $recoveryPath -Description "Recuperacao $RestoreRecovery"
    if (-not $ListRecoveries) { return }
}
if ($RestoreFavorite -ne 0) {
    $favoritePath = Resolve-FavoritePath -Slot $RestoreFavorite
    Restore-ProfileSave -SourcePath $favoritePath -Description "Favorito $RestoreFavorite"
    if (-not $ListRecoveries) { return }
}

if ($ListRecoveries) {
    $entries = @(
        [pscustomobject]@{ Estado = 'Ativo'; Kind = 'Active'; Slot = 0; Caminho = (Resolve-ProfileSavePath) },
        [pscustomobject]@{ Estado = 'Recuperacao 1'; Kind = 'Recovery'; Slot = 1; Caminho = (Resolve-ProfileSavePath -Recovery 1) },
        [pscustomobject]@{ Estado = 'Recuperacao 2'; Kind = 'Recovery'; Slot = 2; Caminho = (Resolve-ProfileSavePath -Recovery 2) },
        [pscustomobject]@{ Estado = 'Recuperacao 3'; Kind = 'Recovery'; Slot = 3; Caminho = (Resolve-ProfileSavePath -Recovery 3) },
        [pscustomobject]@{ Estado = 'Favorito 1'; Kind = 'Favorite'; Slot = 1; Caminho = (Resolve-FavoritePath -Slot 1) },
        [pscustomobject]@{ Estado = 'Favorito 2'; Kind = 'Favorite'; Slot = 2; Caminho = (Resolve-FavoritePath -Slot 2) },
        [pscustomobject]@{ Estado = 'Favorito 3'; Kind = 'Favorite'; Slot = 3; Caminho = (Resolve-FavoritePath -Slot 3) },
        [pscustomobject]@{ Estado = 'Favorito 4'; Kind = 'Favorite'; Slot = 4; Caminho = (Resolve-FavoritePath -Slot 4) },
        [pscustomobject]@{ Estado = 'Favorito 5'; Kind = 'Favorite'; Slot = 5; Caminho = (Resolve-FavoritePath -Slot 5) }
    )
    $available = foreach ($entry in $entries) {
        if ($null -ne $entry.Caminho -and (Test-Path -LiteralPath $entry.Caminho -PathType Leaf)) {
            $file = Get-Item -LiteralPath $entry.Caminho
            [pscustomobject]@{
                Estado = $entry.Estado
                Kind = $entry.Kind
                Slot = $entry.Slot
                Geracao = Get-RegionalidadesSaveGeneration -Path $entry.Caminho
                Tamanho = $file.Length
                Modificado = $file.LastWriteTime
                SHA256 = (Get-FileHash -LiteralPath $entry.Caminho -Algorithm SHA256).Hash
            }
        }
    }
    if ($null -eq $available) {
        if (-not $PassThru) {
            Write-Host "O perfil $Profile ainda nao possui saves."
        }
    } elseif ($PassThru) {
        $available
    } else {
        $available | Format-Table -AutoSize
    }
    return
}

if ($PrepareOnly) {
    Write-Host "Perfil $Profile preparado sem abrir o jogo."
    Write-Host "Save: $savePath"
    Write-Host "Configuracao compartilhada: $configPath"
    return
}

if (Test-Path -LiteralPath $runtimeLog -PathType Leaf) {
    New-Item -ItemType Directory -Path $runtimeHistoryDir -Force | Out-Null
    $archiveLog = Join-Path $runtimeHistoryDir ("runtime-{0}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmssfff'))
    Copy-Item -LiteralPath $runtimeLog -Destination $archiveLog
    @(Get-ChildItem -LiteralPath $runtimeHistoryDir -Filter 'runtime-*.log' -File |
        Sort-Object LastWriteTime -Descending |
        Select-Object -Skip 5) | Remove-Item -Force
}

$environmentNames = @(
    'POKEMON_REGIONALIDADES_SAVE_PATH',
    'POKEMON_REGIONALIDADES_CONFIG_PATH',
    'POKEMON_REGIONALIDADES_LOG_PATH',
    'POKEMON_REGIONALIDADES_PROFILE_LAUNCHER',
    'POKEMON_REGIONALIDADES_PROFILE_RUNNER',
    'POKEMON_REGIONALIDADES_PROFILE_ID',
    'POKEMON_REGIONALIDADES_SETTINGS_SCRIPT',
    'POKEMON_REGIONALIDADES_DEV_SESSION',
    'POKEMON_REGIONALIDADES_APPLY_MOBILITY_PROFILE',
    'POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY',
    'POKEMON_GO_WORLD_AUTOPLAY'
)
$oldEnvironment = @{}

try {
    foreach ($name in $environmentNames) {
        $oldEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
        [Environment]::SetEnvironmentVariable($name, $null, 'Process')
    }
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_SAVE_PATH', $savePath, 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_CONFIG_PATH', $configPath, 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_LOG_PATH', $runtimeLog, 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_PROFILE_LAUNCHER', (Join-Path $PSScriptRoot 'open_player_profiles_pc.ps1'), 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_PROFILE_RUNNER', $PSCommandPath, 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_PROFILE_ID', [string]$Profile, 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_SETTINGS_SCRIPT', (Join-Path $PSScriptRoot 'open_pc_settings.ps1'), 'Process')
    if ((Get-ProfileMetadata).TechnicalMobility) {
        [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY', '1', 'Process')
    }

    Start-Process -FilePath $executable -WorkingDirectory $repoRoot | Out-Null
}
finally {
    foreach ($name in $environmentNames) {
        [Environment]::SetEnvironmentVariable($name, $oldEnvironment[$name], 'Process')
    }
}

Write-Host "Perfil $Profile aberto."
Write-Host "Save: $savePath"
Write-Host "Configuracao compartilhada: $configPath"
Write-Host "Registro: $runtimeLog"
