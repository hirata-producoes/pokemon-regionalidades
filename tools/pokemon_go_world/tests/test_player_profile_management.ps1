param()

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$runner = Join-Path $repoRoot 'tools\pokemon_go_world\run_player_profile_pc.ps1'
$profileUi = Join-Path $repoRoot 'tools\pokemon_go_world\open_player_profiles_pc.ps1'
$testRoot = Join-Path ([IO.Path]::GetTempPath()) ('regionalidades-profile-management-' + [guid]::NewGuid().ToString('N'))
$sourceSave = Join-Path $testRoot 'source.sav'

try {
    [IO.Directory]::CreateDirectory($testRoot) | Out-Null
    [IO.File]::WriteAllBytes($sourceSave, (New-Object byte[] 131072))
    $sourceHash = (Get-FileHash -LiteralPath $sourceSave -Algorithm SHA256).Hash

    $defaultInfo = & $runner -Profile 1 -DataRoot $testRoot -GetProfileInfo -PassThru
    if ($defaultInfo.DisplayName -ne 'Perfil 1' -or $defaultInfo.HasActiveSave) {
        throw 'O perfil vazio nao apresentou o estado padrao esperado.'
    }

    [void](& $runner -Profile 1 -DataRoot $testRoot -SetProfileName 'Minha Jornada' -PassThru)
    try {
        [void](& $runner -Profile 1 -DataRoot $testRoot -SetProfileName ('X' * 33) -PassThru)
        throw 'Um nome de perfil longo demais foi aceito.'
    } catch {
        if ($_.Exception.Message -eq 'Um nome de perfil longo demais foi aceito.') {
            throw
        }
    }

    & $runner -Profile 1 -DataRoot $testRoot -ImportSave $sourceSave -PrepareOnly
    & $profileUi -DataRoot $testRoot -ValidateOnly

    $beforeReset = & $runner -Profile 1 -DataRoot $testRoot -GetProfileInfo -PassThru
    if ($beforeReset.DisplayName -ne 'Minha Jornada' -or -not $beforeReset.HasActiveSave) {
        throw 'O perfil renomeado nao reconheceu o save importado.'
    }

    $reset = & $runner -Profile 1 -DataRoot $testRoot -ResetProfile -PassThru
    & $profileUi -DataRoot $testRoot -ValidateOnly
    $afterReset = & $runner -Profile 1 -DataRoot $testRoot -GetProfileInfo -PassThru
    $backupSave = Join-Path $reset.BackupPath 'pokemon_regionalidades.pgrsave'
    if ($afterReset.DisplayName -ne 'Minha Jornada' -or $afterReset.HasActiveSave -or
        -not (Test-Path -LiteralPath $backupSave -PathType Leaf) -or
        (Get-FileHash -LiteralPath $sourceSave -Algorithm SHA256).Hash -ne $sourceHash) {
        throw 'O reinicio nao preservou corretamente o nome, o backup ou a origem.'
    }

    Write-Output 'PLAYER_PROFILE_MANAGEMENT_TEST_OK'
}
finally {
    $resolvedTestRoot = [IO.Path]::GetFullPath($testRoot)
    $resolvedTempRoot = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
    if ($resolvedTestRoot.StartsWith($resolvedTempRoot, [StringComparison]::OrdinalIgnoreCase) -and
        (Test-Path -LiteralPath $resolvedTestRoot -PathType Container)) {
        [IO.Directory]::Delete($resolvedTestRoot, $true)
    }
}
