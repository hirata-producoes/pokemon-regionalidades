param(
    [string]$ToolchainRoot
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$workspaceRoot = Split-Path $repoRoot -Parent
if ([string]::IsNullOrWhiteSpace($ToolchainRoot)) {
    $ToolchainRoot = Join-Path $workspaceRoot 'toolchains\winlibs-i686-r4-tar\mingw32'
}
$compiler = Join-Path $ToolchainRoot 'bin\gcc.exe'
if (-not (Test-Path -LiteralPath $compiler -PathType Leaf)) {
    throw "Compilador do porte PC nao encontrado: $compiler"
}

$testRoot = Join-Path ([IO.Path]::GetTempPath()) ('regionalidades-save-container-' + [guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($testRoot) | Out-Null
$containerTest = Join-Path $testRoot 'pc_save_container_test.exe'
$atomicTest = Join-Path $testRoot 'save_file_test.exe'
$containerPath = Join-Path $testRoot 'campaign.pgrsave'
$worldContainerPath = Join-Path $testRoot 'world-inspection.pgrsave'
$atomicPath = Join-Path $testRoot 'atomic.sav'

try {
    & $compiler -std=c17 -Wall -Wextra -Werror -DPORTABLE=1 `
        -iquote (Join-Path $repoRoot 'include') `
        (Join-Path $PSScriptRoot 'pc_save_container_test.c') `
        (Join-Path $repoRoot 'src\platform\pc_inventory_state.c') `
        (Join-Path $repoRoot 'src\platform\pc_save_container.c') `
        (Join-Path $repoRoot 'src\platform\pc_rotomdex_state.c') `
        (Join-Path $repoRoot 'src\platform\pc_world_state.c') `
        (Join-Path $repoRoot 'src\platform\save_file.c') `
        -o $containerTest
    if ($LASTEXITCODE -ne 0) {
        throw "A compilacao do teste do conteiner terminou com codigo $LASTEXITCODE."
    }
    & $containerTest $containerPath $worldContainerPath
    if ($LASTEXITCODE -ne 0) {
        throw "O teste do conteiner terminou com codigo $LASTEXITCODE."
    }

    . (Join-Path $repoRoot 'tools\pokemon_go_world\save_container_common_pc.ps1')
    $worldDescriptor = Get-RegionalidadesSaveDescriptor -Path $worldContainerPath
    if ($null -eq $worldDescriptor -or $worldDescriptor.Kind -ne 'Native' -or
        $worldDescriptor.ContainerGeneration -ne 2 -or $worldDescriptor.ChunkCount -ne 2) {
        throw 'As ferramentas de perfil nao reconheceram o bloco WORLD obrigatorio.'
    }
    Write-Output 'PC_WORLD_PROFILE_TOOLS_TEST_OK'

    & $compiler -std=c17 -Wall -Wextra -Werror `
        -iquote (Join-Path $repoRoot 'include') `
        (Join-Path $PSScriptRoot 'save_file_test.c') `
        (Join-Path $repoRoot 'src\platform\save_file.c') `
        -o $atomicTest
    if ($LASTEXITCODE -ne 0) {
        throw "A compilacao do teste atomico terminou com codigo $LASTEXITCODE."
    }
    & $atomicTest $atomicPath
    if ($LASTEXITCODE -ne 0) {
        throw "O teste atomico terminou com codigo $LASTEXITCODE."
    }
    Write-Output 'PC_NATIVE_SAVE_TESTS_OK'
}
finally {
    if ([IO.Path]::GetFullPath($testRoot).StartsWith([IO.Path]::GetTempPath(), [StringComparison]::OrdinalIgnoreCase)) {
        [IO.Directory]::Delete($testRoot, $true)
    }
}
