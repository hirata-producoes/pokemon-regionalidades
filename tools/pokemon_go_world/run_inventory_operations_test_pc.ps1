param(
    [string]$SourceSave,
    [switch]$PrepareOnly,
    [switch]$Recreate
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$runDir = Join-Path $repoRoot 'build\test-pc-inventory-operations'
$targetSave = Join-Path $runDir 'pokemon_regionalidades.pgrsave'
. (Join-Path $PSScriptRoot 'dev_session_common_pc.ps1')
. (Join-Path $PSScriptRoot 'save_container_common_pc.ps1')

if ([string]::IsNullOrWhiteSpace($SourceSave)) {
    $SourceSave = Join-Path $repoRoot 'build\dev-save-exploracao\pokemon_regionalidades.pgrsave'
}
$SourceSave = (Resolve-Path -LiteralPath $SourceSave).Path
if ((Get-RegionalidadesSaveDescriptor -Path $SourceSave).Kind -ne 'Native') {
    throw 'O teste exige um save nativo valido como origem.'
}

$manifestPath = Sync-RegionalidadesDevSessionArtifacts `
    -RepoRoot $repoRoot `
    -RunDir $runDir `
    -SessionKind 'inventory-operations-test'

function Get-InventoryEntry {
    param(
        [Parameter(Mandatory = $true)]
        [byte[]]$Bytes,
        [Parameter(Mandatory = $true)]
        [int]$Location,
        [Parameter(Mandatory = $true)]
        [uint32]$Slot
    )

    $headerSize = [BitConverter]::ToUInt32($Bytes, 12)
    $chunkCount = [BitConverter]::ToUInt32($Bytes, 24)
    $entrySize = [BitConverter]::ToUInt32($Bytes, 28)
    for ($index = 0; $index -lt $chunkCount; $index++) {
        $directoryOffset = [int]($headerSize + $index * $entrySize)
        $id = [Text.Encoding]::ASCII.GetString($Bytes, $directoryOffset, 8).TrimEnd([char]0)
        if ($id -ne 'INVENT') {
            continue
        }

        $payloadOffset = [int][BitConverter]::ToUInt64($Bytes, $directoryOffset + 16)
        $payloadSize = [int][BitConverter]::ToUInt64($Bytes, $directoryOffset + 24)
        $count = [BitConverter]::ToUInt32($Bytes, $payloadOffset + 12)
        for ($itemIndex = 0; $itemIndex -lt $count; $itemIndex++) {
            $itemOffset = $payloadOffset + 32 + $itemIndex * 16
            if ([int]$Bytes[$itemOffset] -eq $Location -and
                [BitConverter]::ToUInt32($Bytes, $itemOffset + 4) -eq $Slot) {
                return [pscustomobject]@{
                    DirectoryOffset = $directoryOffset
                    PayloadOffset = $payloadOffset
                    PayloadSize = $payloadSize
                    ItemOffset = $itemOffset
                    ItemId = [BitConverter]::ToUInt32($Bytes, $itemOffset + 8)
                }
            }
        }
    }
    return $null
}

$preparedNow = $Recreate -or -not (Test-Path -LiteralPath $targetSave -PathType Leaf)
if ($preparedNow) {
    Copy-Item -LiteralPath $SourceSave -Destination $targetSave -Force
    $bytes = [IO.File]::ReadAllBytes($targetSave)
    $pokeBallEntry = Get-InventoryEntry -Bytes $bytes -Location 1 -Slot 0
    $pcEntry = Get-InventoryEntry -Bytes $bytes -Location 5 -Slot 0
    if ($null -eq $pokeBallEntry -or $null -eq $pcEntry) {
        throw 'O save de origem precisa conter um item no primeiro espaco de Pokebolas e outro no primeiro espaco do PC.'
    }

    [Array]::Copy([BitConverter]::GetBytes([uint32]99999), 0, $bytes, $pokeBallEntry.ItemOffset + 12, 4)
    [Array]::Copy([BitConverter]::GetBytes([uint32]99999), 0, $bytes, $pcEntry.ItemOffset + 12, 4)
    $payloadCrc = [RegionalidadesSaveCrc32]::Calculate($bytes, $pokeBallEntry.PayloadOffset, $pokeBallEntry.PayloadSize)
    [Array]::Copy([BitConverter]::GetBytes($payloadCrc), 0, $bytes, $pokeBallEntry.DirectoryOffset + 40, 4)
    $directorySize = [int]([BitConverter]::ToUInt32($bytes, 24) * [BitConverter]::ToUInt32($bytes, 28))
    $directoryCrc = [RegionalidadesSaveCrc32]::Calculate($bytes, 40, $directorySize)
    [Array]::Copy([BitConverter]::GetBytes($directoryCrc), 0, $bytes, 32, 4)
    $headerCrc = [RegionalidadesSaveCrc32]::Calculate($bytes, 0, 36)
    [Array]::Copy([BitConverter]::GetBytes($headerCrc), 0, $bytes, 36, 4)
    [IO.File]::WriteAllBytes($targetSave, $bytes)
} else {
    $bytes = [IO.File]::ReadAllBytes($targetSave)
    $pokeBallEntry = Get-InventoryEntry -Bytes $bytes -Location 1 -Slot 0
    $pcEntry = Get-InventoryEntry -Bytes $bytes -Location 5 -Slot 0
}
if (-not (Test-RegionalidadesSave -Path $targetSave)) {
    throw 'O save isolado existente nao passou pela validacao estrutural. Use -Recreate para gerar uma nova copia.'
}

if (-not $PrepareOnly) {
    $oldTechnicalMobility = [Environment]::GetEnvironmentVariable('POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY', 'Process')
    try {
        [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY', '1', 'Process')
        Start-Process `
            -FilePath (Join-Path $runDir 'pokemon_regionalidades-pc.exe') `
            -WorkingDirectory $runDir | Out-Null
    }
    finally {
        [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY', $oldTechnicalMobility, 'Process')
    }
}

if (-not $preparedNow) {
    Write-Host 'Sessao isolada existente preservada e reaberta.'
} elseif ($PrepareOnly) {
    Write-Host 'Sessao isolada de operacoes do inventario preparada.'
} else {
    Write-Host 'Sessao isolada de operacoes do inventario aberta.'
}
Write-Host "Save isolado: $targetSave"
Write-Host "Manifesto dos artefatos: $manifestPath"
if ($preparedNow) {
    Write-Host "Pokebola preparada: item $($pokeBallEntry.ItemId), quantidade 99999"
    Write-Host "Item do PC preparado: item $($pcEntry.ItemId), quantidade 99999"
}
Write-Host 'Use -Recreate somente quando quiser descartar esta sessao e recomecar o teste.'
