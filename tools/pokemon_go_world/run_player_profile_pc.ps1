param(
    [ValidateSet(1, 2)]
    [int]$Profile = 1,
    [string]$ImportSave,
    [string]$ExportSave,
    [switch]$AllowExportOverwrite,
    [string]$DataRoot,
    [ValidateRange(0, 3)]
    [int]$RestoreRecovery = 0,
    [switch]$ListRecoveries,
    [switch]$PassThru,
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

New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
New-Item -ItemType Directory -Path $configDir -Force | Out-Null

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

if ($RestoreRecovery -ne 0) {
    if (Get-Process -Name 'pokemon_regionalidades-pc' -ErrorAction SilentlyContinue) {
        throw 'Feche todas as instancias do jogo antes de restaurar uma recuperacao.'
    }

    $activeSavePath = Resolve-ProfileSavePath
    $recoveryPath = Resolve-ProfileSavePath -Recovery $RestoreRecovery
    if ($null -eq $activeSavePath) {
        throw "O perfil $Profile ainda nao possui um save ativo."
    }
    if (-not (Test-RegionalidadesSave -Path $activeSavePath)) {
        throw "O save ativo do perfil $Profile nao tem um formato reconhecido."
    }
    if ($null -eq $recoveryPath -or -not (Test-RegionalidadesSave -Path $recoveryPath)) {
        throw "A recuperacao $RestoreRecovery do perfil $Profile nao existe ou e invalida."
    }

    $activeHash = (Get-FileHash -LiteralPath $activeSavePath -Algorithm SHA256).Hash
    $recoveryHash = (Get-FileHash -LiteralPath $recoveryPath -Algorithm SHA256).Hash
    $pendingRestore = "$activeSavePath.restoring"
    $backupPath = "$activeSavePath.before-restore-$(Get-Date -Format 'yyyyMMdd-HHmmssfff')"

    Copy-Item -LiteralPath $recoveryPath -Destination $pendingRestore -Force
    if ((Get-FileHash -LiteralPath $pendingRestore -Algorithm SHA256).Hash -ne $recoveryHash) {
        Remove-Item -LiteralPath $pendingRestore -Force
        throw 'A copia preparada para restauracao nao corresponde a recuperacao escolhida.'
    }

    try {
        [IO.File]::Replace($pendingRestore, $activeSavePath, $backupPath, $true)
    } catch {
        if (Test-Path -LiteralPath $pendingRestore -PathType Leaf) {
            Remove-Item -LiteralPath $pendingRestore -Force
        }
        throw
    }

    if ((Get-FileHash -LiteralPath $activeSavePath -Algorithm SHA256).Hash -ne $recoveryHash -or
        (Get-FileHash -LiteralPath $backupPath -Algorithm SHA256).Hash -ne $activeHash) {
        throw 'A verificacao posterior da restauracao falhou. Nao abra o jogo antes de revisar os arquivos.'
    }

    Write-Host "Recuperacao $RestoreRecovery restaurada no perfil $Profile."
    Write-Host "O save ativo anterior foi preservado em: $backupPath"
}

if ($ListRecoveries) {
    $entries = @(
        [pscustomobject]@{ Estado = 'Ativo'; Caminho = (Resolve-ProfileSavePath) },
        [pscustomobject]@{ Estado = 'Recuperacao 1'; Caminho = (Resolve-ProfileSavePath -Recovery 1) },
        [pscustomobject]@{ Estado = 'Recuperacao 2'; Caminho = (Resolve-ProfileSavePath -Recovery 2) },
        [pscustomobject]@{ Estado = 'Recuperacao 3'; Caminho = (Resolve-ProfileSavePath -Recovery 3) }
    )
    $available = foreach ($entry in $entries) {
        if ($null -ne $entry.Caminho -and (Test-Path -LiteralPath $entry.Caminho -PathType Leaf)) {
            $file = Get-Item -LiteralPath $entry.Caminho
            [pscustomobject]@{
                Estado = $entry.Estado
                Slot = [array]::IndexOf($entries, $entry)
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
    if ($Profile -eq 1) {
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
