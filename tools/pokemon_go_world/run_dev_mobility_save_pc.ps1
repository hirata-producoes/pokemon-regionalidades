$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$runDir = Join-Path $repoRoot 'build\dev-save-exploracao'
$executable = Join-Path $runDir 'pokemon_regionalidades-pc.exe'
$save = Join-Path $runDir 'pokemon_regionalidades.pgrsave'
$legacySave = Join-Path $runDir 'pokemon_regionalidades.sav'
. (Join-Path $PSScriptRoot 'dev_session_common_pc.ps1')
. (Join-Path $PSScriptRoot 'save_container_common_pc.ps1')

$activeSave = if (Test-RegionalidadesSave -Path $save) { $save } elseif (Test-RegionalidadesSave -Path $legacySave) { $legacySave } else { $null }
if ($null -eq $activeSave) {
    throw 'O save de exploracao ainda nao foi preparado. Execute create_dev_mobility_save_pc.ps1 primeiro.'
}

$manifestPath = Sync-RegionalidadesDevSessionArtifacts `
    -RepoRoot $repoRoot `
    -RunDir $runDir `
    -SessionKind 'save-exploracao'

$oldTechnicalMobility = [Environment]::GetEnvironmentVariable('POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY', 'Process')
try {
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY', '1', 'Process')
    Start-Process -FilePath $executable -WorkingDirectory $runDir | Out-Null
}
finally {
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY', $oldTechnicalMobility, 'Process')
}
Write-Host "Save de exploracao aberto: $activeSave"
Write-Host "Manifesto da sessao: $manifestPath"
