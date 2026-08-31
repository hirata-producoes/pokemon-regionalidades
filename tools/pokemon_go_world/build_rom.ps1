param(
    [switch]$InstallDependencies,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path

$wslPath = (& wsl.exe -d Ubuntu -- wslpath -a $projectRoot).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($wslPath)) {
    throw "Ubuntu/WSL2 is not ready. Run install_wsl2.ps1 as Administrator first."
}

if ($InstallDependencies) {
    Write-Host "Installing the build dependencies inside Ubuntu..."
    & wsl.exe -d Ubuntu -- bash -lc "sudo apt update && sudo apt install -y build-essential binutils-arm-none-eabi gcc-arm-none-eabi libnewlib-arm-none-eabi git libpng-dev pkg-config python3"
    if ($LASTEXITCODE -ne 0) {
        throw "Dependency installation failed."
    }
}

$buildAction = if ($Clean) { "make clean && make -j`$(nproc)" } else { "make -j`$(nproc)" }
Write-Host "Building Pokemon Regionalidades..."
& wsl.exe -d Ubuntu -- bash -lc "cd '$wslPath' && $buildAction"
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
