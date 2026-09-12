param(
    [string]$SourceSave,
    [switch]$Recreate,
    [switch]$PrepareOnly
)

$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$runDir = Join-Path $repoRoot 'build\dev-save-exploracao'
$targetSave = Join-Path $runDir 'pokemon_regionalidades.pgrsave'
$legacyTargetSave = Join-Path $runDir 'pokemon_regionalidades.sav'
$runtimeLog = Join-Path $runDir ('mobility-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.log')
. (Join-Path $PSScriptRoot 'dev_session_common_pc.ps1')
. (Join-Path $PSScriptRoot 'save_container_common_pc.ps1')

if ([string]::IsNullOrWhiteSpace($SourceSave)) {
    $nativeSource = Join-Path $repoRoot 'pokemon_regionalidades.pgrsave'
    $SourceSave = if (-not $Recreate -and (Test-Path -LiteralPath $legacyTargetSave -PathType Leaf)) {
        $legacyTargetSave
    } elseif (Test-Path -LiteralPath $nativeSource -PathType Leaf) {
        $nativeSource
    } else {
        Join-Path $repoRoot 'pokemon_regionalidades.sav'
    }
}

$SourceSave = (Resolve-Path -LiteralPath $SourceSave).Path
if (-not (Test-RegionalidadesSave -Path $SourceSave)) {
    throw "O save de origem nao tem um formato reconhecido: $SourceSave"
}

$manifestPath = Sync-RegionalidadesDevSessionArtifacts `
    -RepoRoot $repoRoot `
    -RunDir $runDir `
    -SessionKind 'save-exploracao'

if ($Recreate -or -not (Test-Path -LiteralPath $targetSave)) {
    if (Test-Path -LiteralPath $targetSave) {
        Copy-Item -LiteralPath $targetSave -Destination ($targetSave + '.' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.bak')
    }
    $sourceDescriptor = Get-RegionalidadesSaveDescriptor -Path $SourceSave
    if ($sourceDescriptor.Kind -eq 'Native') {
        Copy-Item -LiteralPath $SourceSave -Destination $targetSave -Force
    } else {
        $nativeBytes = Convert-RegionalidadesLegacyToNativeBytes -LegacyBytes ([IO.File]::ReadAllBytes($SourceSave))
        [IO.File]::WriteAllBytes($targetSave, $nativeBytes)
    }
}

Write-Host "Save de exploracao: $targetSave"
Write-Host 'O perfil preserva o Mudkip, a posicao e o progresso e acrescenta cinco Pokemon.'

if ($PrepareOnly) {
    Write-Host 'Arquivos preparados. O perfil ainda nao foi aplicado porque -PrepareOnly foi usado.'
    return
}

$oldProfile = [Environment]::GetEnvironmentVariable('POKEMON_REGIONALIDADES_APPLY_MOBILITY_PROFILE', 'Process')
$oldDevSession = [Environment]::GetEnvironmentVariable('POKEMON_REGIONALIDADES_DEV_SESSION', 'Process')
$oldTechnicalMobility = [Environment]::GetEnvironmentVariable('POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY', 'Process')
$oldLogPath = [Environment]::GetEnvironmentVariable('POKEMON_REGIONALIDADES_LOG_PATH', 'Process')
$oldAutoplay = [Environment]::GetEnvironmentVariable('POKEMON_GO_WORLD_AUTOPLAY', 'Process')
try {
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_DEV_SESSION', '1', 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_APPLY_MOBILITY_PROFILE', '1', 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY', '1', 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_LOG_PATH', $runtimeLog, 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_GO_WORLD_AUTOPLAY', '1', 'Process')
    $process = Start-Process `
        -FilePath (Join-Path $runDir 'pokemon_regionalidades-pc.exe') `
        -WorkingDirectory $runDir `
        -WindowStyle Hidden `
        -PassThru
}
finally {
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_APPLY_MOBILITY_PROFILE', $oldProfile, 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_DEV_SESSION', $oldDevSession, 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY', $oldTechnicalMobility, 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_LOG_PATH', $oldLogPath, 'Process')
    [Environment]::SetEnvironmentVariable('POKEMON_GO_WORLD_AUTOPLAY', $oldAutoplay, 'Process')
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
    throw "Nao foi possivel confirmar a gravacao do save. Consulte: $runtimeLog"
}

Write-Host 'Perfil aplicado e gravado com sucesso.'
Write-Host "Registro tecnico: $runtimeLog"
Write-Host "Manifesto da sessao: $manifestPath"
