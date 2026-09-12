param(
    [string]$SourceSave
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$runDir = Join-Path $repoRoot 'build\test-pc-inventory-limit'
$targetSave = Join-Path $runDir 'pokemon_regionalidades.pgrsave'
$runtimeLog = Join-Path $runDir ('runtime-' + (Get-Date -Format 'yyyyMMdd-HHmmssfff') + '.log')
. (Join-Path $repoRoot 'tools\pokemon_go_world\dev_session_common_pc.ps1')
. (Join-Path $repoRoot 'tools\pokemon_go_world\save_container_common_pc.ps1')

if ([string]::IsNullOrWhiteSpace($SourceSave)) {
    $SourceSave = Join-Path $repoRoot 'build\dev-save-exploracao\pokemon_regionalidades.pgrsave'
}
$SourceSave = (Resolve-Path -LiteralPath $SourceSave).Path
if ((Get-RegionalidadesSaveDescriptor -Path $SourceSave).Kind -ne 'Native') {
    throw 'O teste exige um save nativo valido como origem.'
}

Sync-RegionalidadesDevSessionArtifacts `
    -RepoRoot $repoRoot `
    -RunDir $runDir `
    -SessionKind 'inventory-limit-test' | Out-Null
Copy-Item -LiteralPath $SourceSave -Destination $targetSave -Force

function Get-InventoryEntry {
    param([byte[]]$Bytes, [int]$Location = -1, [uint32]$Slot = [uint32]::MaxValue)

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
            $itemLocation = [int]$Bytes[$itemOffset]
            $itemSlot = [BitConverter]::ToUInt32($Bytes, $itemOffset + 4)
            if (($Location -lt 0 -or $itemLocation -eq $Location) -and
                ($Slot -eq [uint32]::MaxValue -or $itemSlot -eq $Slot)) {
                return [pscustomobject]@{
                    DirectoryOffset = $directoryOffset
                    PayloadOffset = $payloadOffset
                    PayloadSize = $payloadSize
                    ItemOffset = $itemOffset
                    Location = $itemLocation
                    Slot = $itemSlot
                    ItemId = [BitConverter]::ToUInt32($Bytes, $itemOffset + 8)
                    Quantity = [BitConverter]::ToUInt32($Bytes, $itemOffset + 12)
                }
            }
        }
    }
    return $null
}

$bytes = [IO.File]::ReadAllBytes($targetSave)
$entry = Get-InventoryEntry -Bytes $bytes
if ($null -eq $entry) {
    throw 'O bloco INVENT nao contem um item que possa ser usado pelo teste.'
}
[Array]::Copy([BitConverter]::GetBytes([uint32]99999), 0, $bytes, $entry.ItemOffset + 12, 4)
$payloadCrc = [RegionalidadesSaveCrc32]::Calculate($bytes, $entry.PayloadOffset, $entry.PayloadSize)
[Array]::Copy([BitConverter]::GetBytes($payloadCrc), 0, $bytes, $entry.DirectoryOffset + 40, 4)
$directorySize = [int]([BitConverter]::ToUInt32($bytes, 24) * [BitConverter]::ToUInt32($bytes, 28))
$directoryCrc = [RegionalidadesSaveCrc32]::Calculate($bytes, 40, $directorySize)
[Array]::Copy([BitConverter]::GetBytes($directoryCrc), 0, $bytes, 32, 4)
$headerCrc = [RegionalidadesSaveCrc32]::Calculate($bytes, 0, 36)
[Array]::Copy([BitConverter]::GetBytes($headerCrc), 0, $bytes, 36, 4)
[IO.File]::WriteAllBytes($targetSave, $bytes)
if ($null -eq (Get-RegionalidadesSaveDescriptor -Path $targetSave)) {
    throw 'O save preparado para o teste nao passou pela validacao estrutural.'
}

$oldValues = @{}
$environment = @{
    POKEMON_REGIONALIDADES_DEV_SESSION = '1'
    POKEMON_REGIONALIDADES_APPLY_MOBILITY_PROFILE = '1'
    POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY = '1'
    POKEMON_REGIONALIDADES_LOG_PATH = $runtimeLog
    POKEMON_GO_WORLD_AUTOPLAY = '1'
}
try {
    foreach ($name in $environment.Keys) {
        $oldValues[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
        [Environment]::SetEnvironmentVariable($name, $environment[$name], 'Process')
    }
    $process = Start-Process `
        -FilePath (Join-Path $runDir 'pokemon_regionalidades-pc.exe') `
        -WorkingDirectory $runDir `
        -WindowStyle Hidden `
        -PassThru
}
finally {
    foreach ($name in $environment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $oldValues[$name], 'Process')
    }
}

$deadline = (Get-Date).AddSeconds(30)
$saved = $false
while ((Get-Date) -lt $deadline -and -not $process.HasExited) {
    Start-Sleep -Milliseconds 200
    if (Test-Path -LiteralPath $runtimeLog) {
        $saved = Select-String -LiteralPath $runtimeLog -Pattern '^Development save: TrySavingData=1$' -Quiet
        if ($saved) {
            break
        }
    }
}
if (-not $process.HasExited) {
    Stop-Process -Id $process.Id
    $process.WaitForExit()
}
if (-not $saved) {
    throw "O jogo nao confirmou a gravacao. Consulte: $runtimeLog"
}

$savedBytes = [IO.File]::ReadAllBytes($targetSave)
$savedEntry = Get-InventoryEntry -Bytes $savedBytes -Location $entry.Location -Slot $entry.Slot
if ($null -eq $savedEntry -or $savedEntry.ItemId -ne $entry.ItemId -or $savedEntry.Quantity -ne 99999) {
    throw 'A quantidade nativa foi alterada durante o ciclo real de carregamento e gravacao.'
}
if ($null -eq (Get-RegionalidadesSaveDescriptor -Path $targetSave)) {
    throw 'O save gravado pelo jogo nao passou pela validacao estrutural.'
}

Write-Output 'PC_INVENTORY_GAMEPLAY_LIMIT_TEST_OK'
