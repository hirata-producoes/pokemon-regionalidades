$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$executable = Join-Path $repoRoot 'pokemon_regionalidades-pc.exe'
$resourcePack = Join-Path $repoRoot 'pokemon_regionalidades.pak'
$fixture = Join-Path $PSScriptRoot 'pc_controls_test.cfg'
$testRoot = Join-Path ([IO.Path]::GetTempPath()) ('regionalidades-controls-' + [guid]::NewGuid().ToString('N'))
$configPath = Join-Path $testRoot 'pokemon_regionalidades.cfg'
$savePath = Join-Path $testRoot 'pokemon_regionalidades.sav'
$logPath = Join-Path $testRoot 'runtime.log'

[IO.Directory]::CreateDirectory($testRoot) | Out-Null
Copy-Item -LiteralPath $fixture -Destination $configPath

$previous = @{}
foreach ($name in @('POKEMON_REGIONALIDADES_CONFIG_PATH','POKEMON_REGIONALIDADES_SAVE_PATH','POKEMON_REGIONALIDADES_LOG_PATH','POKEMON_REGIONALIDADES_RESOURCE_PACK')) {
    $previous[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
}

try {
    $env:POKEMON_REGIONALIDADES_CONFIG_PATH = $configPath
    $env:POKEMON_REGIONALIDADES_SAVE_PATH = $savePath
    $env:POKEMON_REGIONALIDADES_LOG_PATH = $logPath
    $env:POKEMON_REGIONALIDADES_RESOURCE_PACK = $resourcePack
    $process = Start-Process -FilePath $executable -WorkingDirectory $repoRoot -WindowStyle Hidden -PassThru
    Start-Sleep -Milliseconds 1800
    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id
        $process.WaitForExit()
    }
    $log = [IO.File]::ReadAllText($logPath)
    if ($log -notmatch 'PC controls: keyA=C keyB=V keySpeed=Left Shift speed=3x' -or
        $log -notmatch 'PC controller: A=B B=Y speed=LT') {
        throw "O executável não carregou os controles esperados.`r`n$log"
    }
    $storedConfig = [IO.File]::ReadAllText($configPath)
    foreach ($expected in @('keyA=C','keyB=V','keySpeed=Left Shift','controllerA=B','controllerB=Y','controllerSpeed=LT','speedMultiplier=3')) {
        if ($storedConfig -notmatch ('(?m)^' + [regex]::Escape($expected) + '\r?$')) {
            throw "O executável não preservou a configuração ao regravá-la: $expected"
        }
    }
    Write-Output 'PC_CONTROLS_CONFIG_TEST_OK'
}
finally {
    foreach ($name in $previous.Keys) {
        [Environment]::SetEnvironmentVariable($name, $previous[$name], 'Process')
    }
    if ([IO.Path]::GetFullPath($testRoot).StartsWith([IO.Path]::GetTempPath(), [StringComparison]::OrdinalIgnoreCase)) {
        [IO.Directory]::Delete($testRoot, $true)
    }
}
