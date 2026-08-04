#Requires -RunAsAdministrator

$ErrorActionPreference = "Stop"

Write-Host "Installing WSL2 with Ubuntu..."
wsl.exe --install -d Ubuntu

Write-Host ""
Write-Host "If Windows asks for a restart, restart before continuing."
Write-Host "After Ubuntu finishes its first-run setup, run:"
Write-Host "  powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_rom.ps1 -InstallDependencies"
