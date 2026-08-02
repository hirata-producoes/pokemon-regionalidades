param(
    [string]$ToolchainRoot,
    [string]$SdlRoot,
    [string]$PythonPath,
    [int]$Jobs = [Environment]::ProcessorCount
)

$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$workspaceRoot = Split-Path $repoRoot -Parent

if (-not $ToolchainRoot) {
    $ToolchainRoot = Join-Path $workspaceRoot 'toolchains\winlibs-i686-r4-tar\mingw32'
}
if (-not $SdlRoot) {
    $SdlRoot = Join-Path $workspaceRoot 'toolchains\SDL2-2.30.7\SDL2-2.30.7\i686-w64-mingw32'
}
if (-not $PythonPath) {
    $PythonPath = 'C:\Users\Rafael\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
}

$toolBin = Join-Path $ToolchainRoot 'bin'
$make = Join-Path $toolBin 'mingw32-make.exe'
$sdlDll = Join-Path $SdlRoot 'bin\SDL2.dll'
$gitUsrBin = 'C:\Program Files\Git\usr\bin'

foreach ($required in @($make, $sdlDll, $PythonPath)) {
    if (-not (Test-Path -LiteralPath $required)) {
        throw "Dependência do porte PC não encontrada: $required"
    }
}

$env:Path = "$toolBin;$gitUsrBin;$env:Path"
$env:PYTHONUTF8 = '1'
$sdlMakePath = $SdlRoot.Replace('\', '/')
$pythonMakePath = $PythonPath.Replace('\', '/')
$toolBinMakePath = $toolBin.Replace('\', '/')

$makeArgs = @(
    '-j', [Math]::Max(1, $Jobs),
    'pc',
    'PREFIX=',
    'ARMCC=true',
    "CC=$toolBinMakePath/gcc.exe",
    "CXX=$toolBinMakePath/g++.exe",
    "CPP=$toolBinMakePath/cpp.exe",
    "AS=$toolBinMakePath/as.exe",
    "LD=$toolBinMakePath/ld.exe",
    "OBJCOPY=$toolBinMakePath/objcopy.exe",
    "OBJDUMP=$toolBinMakePath/objdump.exe",
    "WINDRES=$toolBinMakePath/windres.exe",
    "SDL_DIR=$sdlMakePath",
    "PYTHON=$pythonMakePath"
)

Push-Location $repoRoot
try {
    & $make @makeArgs
    if ($LASTEXITCODE -ne 0) {
        throw "A compilação PC terminou com código $LASTEXITCODE."
    }

    Copy-Item -LiteralPath $sdlDll -Destination (Join-Path $repoRoot 'SDL2.dll') -Force
    Write-Host 'Porte PC compilado com sucesso.' -ForegroundColor Green
}
finally {
    Pop-Location
}
