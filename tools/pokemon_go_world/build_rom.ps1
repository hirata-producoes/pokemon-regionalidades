param(
    [switch]$InstallDependencies,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path

function ConvertTo-WslPath([string]$WindowsPath) {
    $output = & wsl.exe -d Ubuntu --exec wslpath -a -- $WindowsPath 2>&1
    $exitCode = $LASTEXITCODE
    $convertedPath = ($output | Out-String).Trim()
    if ($exitCode -ne 0 -or [string]::IsNullOrWhiteSpace($convertedPath)) {
        $details = if ([string]::IsNullOrWhiteSpace($convertedPath)) { "sem detalhes" } else { $convertedPath }
        throw "Nao foi possivel converter o caminho '$WindowsPath' para o WSL. Confirme que a distribuicao Ubuntu esta instalada e funcionando. Detalhes: $details"
    }

    return $convertedPath
}

$gitDirOutput = & git -C $projectRoot rev-parse --absolute-git-dir 2>&1
$gitDirExitCode = $LASTEXITCODE
$gitDir = ($gitDirOutput | Out-String).Trim()
if ($gitDirExitCode -ne 0 -or [string]::IsNullOrWhiteSpace($gitDir)) {
    $details = if ([string]::IsNullOrWhiteSpace($gitDir)) { "sem detalhes" } else { $gitDir }
    throw "O projeto nao foi reconhecido como um repositorio Git no Windows. Detalhes: $details"
}

$wslPath = ConvertTo-WslPath $projectRoot
$wslGitDir = ConvertTo-WslPath $gitDir

if ($InstallDependencies) {
    Write-Host "Installing the build dependencies inside Ubuntu..."
    & wsl.exe -d Ubuntu -- bash -lc "sudo apt update && sudo apt install -y build-essential binutils-arm-none-eabi gcc-arm-none-eabi libnewlib-arm-none-eabi git libpng-dev pkg-config python3"
    if ($LASTEXITCODE -ne 0) {
        throw "Dependency installation failed."
    }
}

$buildAction = if ($Clean) {
    'cd "$1" && export GIT_DIR="$2" GIT_WORK_TREE="$1" && make clean && make -j"$(nproc)"'
} else {
    'cd "$1" && export GIT_DIR="$2" GIT_WORK_TREE="$1" && make -j"$(nproc)"'
}
Write-Host "Building Pokemon Regionalidades..."
& wsl.exe -d Ubuntu --exec bash -lc $buildAction bash $wslPath $wslGitDir
if ($LASTEXITCODE -ne 0) {
    throw "ROM build failed."
}

$romPath = Join-Path $projectRoot "pokemon_regionalidades.gba"
if (-not (Test-Path $romPath)) {
    throw "The build finished but pokemon_regionalidades.gba was not found."
}

Write-Host "ROM ready: $romPath"

$memoryReport = Join-Path $PSScriptRoot "memory_report.ps1"
if (Test-Path -LiteralPath $memoryReport) {
    & $memoryReport
}
