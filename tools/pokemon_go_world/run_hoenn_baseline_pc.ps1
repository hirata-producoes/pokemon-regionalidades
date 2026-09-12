param(
    [switch]$NewSession
)

$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$runDir = Join-Path $repoRoot 'build\baseline-hoenn'
$save = Join-Path $runDir 'pokemon_regionalidades.pgrsave'
$legacySave = Join-Path $runDir 'pokemon_regionalidades.sav'
. (Join-Path $PSScriptRoot 'dev_session_common_pc.ps1')
. (Join-Path $PSScriptRoot 'save_container_common_pc.ps1')

if ($NewSession -and ((Test-Path -LiteralPath $save -PathType Leaf) -or
    (Test-Path -LiteralPath $legacySave -PathType Leaf))) {
    $backupDir = Join-Path $runDir 'backups'
    New-Item -ItemType Directory -Path $backupDir -Force | Out-Null
    $backupStamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    foreach ($existingSave in @($save, $legacySave)) {
        if (Test-Path -LiteralPath $existingSave -PathType Leaf) {
            $backupName = ([IO.Path]::GetFileNameWithoutExtension($existingSave)) + '-' + $backupStamp + [IO.Path]::GetExtension($existingSave)
            $backupPath = Join-Path $backupDir $backupName
            Move-Item -LiteralPath $existingSave -Destination $backupPath
            Write-Host "Save narrativo anterior preservado em: $backupPath"
        }
    }
}

if ((Test-Path -LiteralPath $save -PathType Leaf) -and -not (Test-RegionalidadesSave -Path $save)) {
    throw "O save narrativo nao tem um formato reconhecido: $save"
}

$manifestPath = Sync-RegionalidadesDevSessionArtifacts `
    -RepoRoot $repoRoot `
    -RunDir $runDir `
    -SessionKind 'baseline-hoenn-narrativa-real'

$executable = Join-Path $runDir 'pokemon_regionalidades-pc.exe'
$runtimeLog = Join-Path $runDir 'runtime-last.log'
$environmentNames = @(
    'POKEMON_REGIONALIDADES_DEV_SESSION',
    'POKEMON_REGIONALIDADES_APPLY_MOBILITY_PROFILE',
    'POKEMON_GO_WORLD_AUTOPLAY',
    'POKEMON_REGIONALIDADES_LOG_PATH'
)
$oldEnvironment = @{}

try {
    foreach ($name in $environmentNames) {
        $oldEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
        [Environment]::SetEnvironmentVariable($name, $null, 'Process')
    }
    [Environment]::SetEnvironmentVariable('POKEMON_REGIONALIDADES_LOG_PATH', $runtimeLog, 'Process')

    Start-Process -FilePath $executable -WorkingDirectory $runDir | Out-Null
}
finally {
    foreach ($name in $environmentNames) {
        [Environment]::SetEnvironmentVariable($name, $oldEnvironment[$name], 'Process')
    }
}

Write-Host 'Baseline narrativa de Hoenn aberta sem modificacao de progresso.'
Write-Host "Save isolado: $save"
Write-Host "Registro da sessao: $runtimeLog"
Write-Host "Manifesto dos artefatos: $manifestPath"
