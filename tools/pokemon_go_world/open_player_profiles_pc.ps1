param(
    [string]$DataRoot,
    [switch]$ValidateOnly
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$profileRunner = Join-Path $PSScriptRoot 'run_player_profile_pc.ps1'
$settingsUi = Join-Path $PSScriptRoot 'open_pc_settings.ps1'

if ([string]::IsNullOrWhiteSpace($DataRoot)) {
    $localData = [Environment]::GetFolderPath([Environment+SpecialFolder]::LocalApplicationData)
    if ([string]::IsNullOrWhiteSpace($localData)) {
        throw 'O Windows nao informou o diretorio local de dados do usuario.'
    }
    $DataRoot = Join-Path $localData 'Pokemon Regionalidades'
} else {
    $DataRoot = [IO.Path]::GetFullPath($DataRoot)
}

$colorBackground = [Drawing.Color]::FromArgb(28, 35, 48)
$colorCard = [Drawing.Color]::FromArgb(42, 52, 68)
$colorPrimary = [Drawing.Color]::FromArgb(59, 130, 246)
$colorSecondary = [Drawing.Color]::FromArgb(71, 85, 105)
$colorText = [Drawing.Color]::FromArgb(241, 245, 249)
$colorMuted = [Drawing.Color]::FromArgb(184, 196, 210)

$form = New-Object Windows.Forms.Form
$form.Text = 'Pokémon Regionalidades — Perfis'
$form.ClientSize = New-Object Drawing.Size(820, 500)
$form.StartPosition = 'CenterScreen'
$form.FormBorderStyle = 'FixedSingle'
$form.MaximizeBox = $false
$form.BackColor = $colorBackground
$form.ForeColor = $colorText
$form.Font = New-Object Drawing.Font('Segoe UI', 10)
$form.AutoScaleMode = 'Dpi'

$title = New-Object Windows.Forms.Label
$title.Text = 'Pokémon Regionalidades'
$title.Font = New-Object Drawing.Font('Segoe UI Semibold', 22)
$title.ForeColor = $colorText
$title.Location = New-Object Drawing.Point(28, 20)
$title.AutoSize = $true
$form.Controls.Add($title)

$subtitle = New-Object Windows.Forms.Label
$subtitle.Text = 'Escolha uma campanha para continuar'
$subtitle.Font = New-Object Drawing.Font('Segoe UI', 11)
$subtitle.ForeColor = $colorMuted
$subtitle.Location = New-Object Drawing.Point(31, 64)
$subtitle.AutoSize = $true
$form.Controls.Add($subtitle)

$settingsButton = New-Object Windows.Forms.Button
$settingsButton.Text = 'Configurações'
$settingsButton.Location = New-Object Drawing.Point(500, 35)
$settingsButton.Size = New-Object Drawing.Size(140, 34)
$settingsButton.FlatStyle = 'Flat'
$settingsButton.BackColor = $colorSecondary
$settingsButton.ForeColor = $colorText
$form.Controls.Add($settingsButton)

$shortcutButton = New-Object Windows.Forms.Button
$shortcutButton.Text = 'Criar atalho'
$shortcutButton.Location = New-Object Drawing.Point(650, 35)
$shortcutButton.Size = New-Object Drawing.Size(140, 34)
$shortcutButton.FlatStyle = 'Flat'
$shortcutButton.BackColor = $colorSecondary
$shortcutButton.ForeColor = $colorText
$form.Controls.Add($shortcutButton)

$settingsButton.Add_Click({
    try {
        & $settingsUi -DataRoot $DataRoot
    } catch {
        Show-ProfileError -Message $_.Exception.Message
    }
})

$cards = @{}

function Show-ProfileError {
    param([string]$Message)
    [void][Windows.Forms.MessageBox]::Show(
        $form,
        $Message,
        'Pokémon Regionalidades',
        [Windows.Forms.MessageBoxButtons]::OK,
        [Windows.Forms.MessageBoxIcon]::Error)
}

function Get-ProfileEntries {
    param([int]$Profile)
    return @(& $profileRunner -Profile $Profile -DataRoot $DataRoot -ListRecoveries -PassThru)
}

function New-RegionalidadesShortcut {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ShortcutPath
    )

    $powershell = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'
    $shell = New-Object -ComObject WScript.Shell
    $shortcut = $shell.CreateShortcut($ShortcutPath)
    $shortcut.TargetPath = $powershell
    $shortcut.Arguments = "-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File `"$PSCommandPath`""
    $shortcut.WorkingDirectory = $repoRoot
    $shortcut.IconLocation = "$(Join-Path $repoRoot 'pokemon_regionalidades-pc.exe'),0"
    $shortcut.Description = 'Abrir os perfis de Pokémon Regionalidades'
    $shortcut.Save()
    [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($shortcut)
    [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($shell)
}

$shortcutButton.Add_Click({
    try {
        $desktop = [Environment]::GetFolderPath([Environment+SpecialFolder]::DesktopDirectory)
        if ([string]::IsNullOrWhiteSpace($desktop)) {
            throw 'O Windows não informou o caminho da área de trabalho.'
        }
        $shortcutPath = Join-Path $desktop 'Pokémon Regionalidades.lnk'
        New-RegionalidadesShortcut -ShortcutPath $shortcutPath
        [void][Windows.Forms.MessageBox]::Show(
            $form,
            "Atalho criado em:`r`n$shortcutPath",
            'Atalho criado',
            [Windows.Forms.MessageBoxButtons]::OK,
            [Windows.Forms.MessageBoxIcon]::Information)
    } catch {
        Show-ProfileError -Message $_.Exception.Message
    }
})

function Refresh-ProfileCard {
    param([int]$Profile)

    $card = $cards[$Profile]
    $entries = @(Get-ProfileEntries -Profile $Profile)
    $active = @($entries | Where-Object Slot -eq 0)
    $recoveries = @($entries | Where-Object Slot -gt 0 | Sort-Object Slot)

    if ($active.Count -eq 0) {
        $card.Status.Text = 'Novo perfil — nenhuma campanha iniciada'
        $card.Import.Enabled = $true
        $card.Export.Enabled = $false
    } else {
        $generation = if ($null -eq $active[0].Geracao) { 'desconhecida' } else { $active[0].Geracao }
        $card.Status.Text = "Save ativo — geração $generation`r`nAtualizado em $($active[0].Modificado.ToString('dd/MM/yyyy HH:mm:ss'))"
        $card.Import.Enabled = $false
        $card.Export.Enabled = $true
    }

    $card.Recoveries.Items.Clear()
    foreach ($recovery in $recoveries) {
        $generation = if ($null -eq $recovery.Geracao) { 'desconhecida' } else { $recovery.Geracao }
        $item = [pscustomobject]@{
            Text = "Recuperação $($recovery.Slot) — geração $generation"
            Slot = $recovery.Slot
        }
        [void]$card.Recoveries.Items.Add($item)
    }
    if ($card.Recoveries.Items.Count -gt 0) {
        $card.Recoveries.SelectedIndex = 0
    }
    $card.Restore.Enabled = $card.Recoveries.Items.Count -gt 0
}

function New-ProfileCard {
    param(
        [int]$Profile,
        [int]$Left
    )

    $panel = New-Object Windows.Forms.Panel
    $panel.Location = New-Object Drawing.Point($Left, 105)
    $panel.Size = New-Object Drawing.Size(370, 310)
    $panel.BackColor = $colorCard
    $panel.BorderStyle = 'FixedSingle'
    $form.Controls.Add($panel)

    $heading = New-Object Windows.Forms.Label
    $profilePurpose = if ($Profile -eq 1) { 'Exploração e testes' } else { 'Campanha de Hoenn' }
    $heading.Text = "Perfil $Profile — $profilePurpose"
    $heading.Font = New-Object Drawing.Font('Segoe UI Semibold', 16)
    $heading.ForeColor = $colorText
    $heading.Location = New-Object Drawing.Point(20, 18)
    $heading.AutoSize = $true
    $panel.Controls.Add($heading)

    $status = New-Object Windows.Forms.Label
    $status.ForeColor = $colorMuted
    $status.Location = New-Object Drawing.Point(22, 58)
    $status.Size = New-Object Drawing.Size(325, 52)
    $panel.Controls.Add($status)

    $openButton = New-Object Windows.Forms.Button
    $openButton.Text = 'Abrir perfil'
    $openButton.Location = New-Object Drawing.Point(22, 120)
    $openButton.Size = New-Object Drawing.Size(325, 40)
    $openButton.FlatStyle = 'Flat'
    $openButton.BackColor = $colorPrimary
    $openButton.ForeColor = [Drawing.Color]::White
    $openButton.FlatAppearance.BorderSize = 0
    $panel.Controls.Add($openButton)

    $importButton = New-Object Windows.Forms.Button
    $importButton.Text = 'Importar save'
    $importButton.Location = New-Object Drawing.Point(22, 172)
    $importButton.Size = New-Object Drawing.Size(155, 34)
    $importButton.FlatStyle = 'Flat'
    $importButton.BackColor = $colorSecondary
    $importButton.ForeColor = $colorText
    $panel.Controls.Add($importButton)

    $exportButton = New-Object Windows.Forms.Button
    $exportButton.Text = 'Exportar save'
    $exportButton.Location = New-Object Drawing.Point(192, 172)
    $exportButton.Size = New-Object Drawing.Size(155, 34)
    $exportButton.FlatStyle = 'Flat'
    $exportButton.BackColor = $colorSecondary
    $exportButton.ForeColor = $colorText
    $panel.Controls.Add($exportButton)

    $recoveryLabel = New-Object Windows.Forms.Label
    $recoveryLabel.Text = 'Recuperações disponíveis'
    $recoveryLabel.ForeColor = $colorMuted
    $recoveryLabel.Location = New-Object Drawing.Point(22, 218)
    $recoveryLabel.AutoSize = $true
    $panel.Controls.Add($recoveryLabel)

    $recoveries = New-Object Windows.Forms.ComboBox
    $recoveries.DropDownStyle = 'DropDownList'
    $recoveries.DisplayMember = 'Text'
    $recoveries.Location = New-Object Drawing.Point(22, 244)
    $recoveries.Size = New-Object Drawing.Size(211, 32)
    $panel.Controls.Add($recoveries)

    $restoreButton = New-Object Windows.Forms.Button
    $restoreButton.Text = 'Restaurar'
    $restoreButton.Location = New-Object Drawing.Point(243, 243)
    $restoreButton.Size = New-Object Drawing.Size(104, 32)
    $restoreButton.FlatStyle = 'Flat'
    $restoreButton.BackColor = $colorSecondary
    $restoreButton.ForeColor = $colorText
    $panel.Controls.Add($restoreButton)

    $cards[$Profile] = [pscustomobject]@{
        Panel = $panel
        Status = $status
        Open = $openButton
        Import = $importButton
        Export = $exportButton
        Recoveries = $recoveries
        Restore = $restoreButton
    }

    $profileId = $Profile
    $openButton.Add_Click({
        try {
            & $profileRunner -Profile $profileId -DataRoot $DataRoot
            $form.Close()
        } catch {
            Show-ProfileError -Message $_.Exception.Message
        }
    }.GetNewClosure())

    $importButton.Add_Click({
        $dialog = New-Object Windows.Forms.OpenFileDialog
        $dialog.Title = "Importar save para o perfil $profileId"
        $dialog.Filter = 'Save de Pokémon (*.sav)|*.sav|Todos os arquivos (*.*)|*.*'
        if ($dialog.ShowDialog($form) -eq [Windows.Forms.DialogResult]::OK) {
            try {
                & $profileRunner -Profile $profileId -DataRoot $DataRoot -ImportSave $dialog.FileName -PrepareOnly
                Refresh-ProfileCard -Profile $profileId
                [void][Windows.Forms.MessageBox]::Show(
                    $form,
                    'O save foi copiado. O arquivo original continua preservado.',
                    'Importação concluída',
                    [Windows.Forms.MessageBoxButtons]::OK,
                    [Windows.Forms.MessageBoxIcon]::Information)
            } catch {
                Show-ProfileError -Message $_.Exception.Message
            }
        }
        $dialog.Dispose()
    }.GetNewClosure())

    $exportButton.Add_Click({
        $dialog = New-Object Windows.Forms.SaveFileDialog
        $dialog.Title = "Exportar save do perfil $profileId"
        $dialog.Filter = 'Save de Pokémon (*.sav)|*.sav'
        $dialog.DefaultExt = 'sav'
        $dialog.AddExtension = $true
        $dialog.OverwritePrompt = $true
        $dialog.FileName = "pokemon-regionalidades-perfil-$profileId-$(Get-Date -Format 'yyyyMMdd-HHmm').sav"
        if ($dialog.ShowDialog($form) -eq [Windows.Forms.DialogResult]::OK) {
            try {
                & $profileRunner -Profile $profileId -DataRoot $DataRoot -ExportSave $dialog.FileName -AllowExportOverwrite -PrepareOnly
                [void][Windows.Forms.MessageBox]::Show(
                    $form,
                    "O save foi exportado para:`r`n$($dialog.FileName)",
                    'Exportação concluída',
                    [Windows.Forms.MessageBoxButtons]::OK,
                    [Windows.Forms.MessageBoxIcon]::Information)
            } catch {
                Show-ProfileError -Message $_.Exception.Message
            }
        }
        $dialog.Dispose()
    }.GetNewClosure())

    $restoreButton.Add_Click({
        $selected = $cards[$profileId].Recoveries.SelectedItem
        if ($null -eq $selected) {
            return
        }
        $answer = [Windows.Forms.MessageBox]::Show(
            $form,
            "Restaurar $($selected.Text)?`r`n`r`nO save ativo atual também será preservado.",
            'Confirmar restauração',
            [Windows.Forms.MessageBoxButtons]::YesNo,
            [Windows.Forms.MessageBoxIcon]::Question)
        if ($answer -ne [Windows.Forms.DialogResult]::Yes) {
            return
        }
        try {
            & $profileRunner -Profile $profileId -DataRoot $DataRoot -RestoreRecovery $selected.Slot -PrepareOnly
            Refresh-ProfileCard -Profile $profileId
            [void][Windows.Forms.MessageBox]::Show(
                $form,
                'A recuperação foi restaurada e o save ativo anterior foi preservado.',
                'Restauração concluída',
                [Windows.Forms.MessageBoxButtons]::OK,
                [Windows.Forms.MessageBoxIcon]::Information)
        } catch {
            Show-ProfileError -Message $_.Exception.Message
        }
    }.GetNewClosure())
}

New-ProfileCard -Profile 1 -Left 28
New-ProfileCard -Profile 2 -Left 422
Refresh-ProfileCard -Profile 1
Refresh-ProfileCard -Profile 2

$footer = New-Object Windows.Forms.Label
$footer.Text = 'Cada perfil mantém sua própria campanha e até três recuperações. As configurações do programa são compartilhadas.'
$footer.ForeColor = $colorMuted
$footer.Location = New-Object Drawing.Point(30, 440)
$footer.Size = New-Object Drawing.Size(755, 42)
$footer.TextAlign = 'MiddleCenter'
$form.Controls.Add($footer)

if ($ValidateOnly) {
    if ($cards.Count -ne 2 -or $cards[1].Open.Text -ne 'Abrir perfil' -or
        $cards[2].Recoveries.DisplayMember -ne 'Text' -or $shortcutButton.Text -ne 'Criar atalho' -or
        $settingsButton.Text -ne 'Configurações') {
        throw 'A validacao estrutural da interface de perfis falhou.'
    }
    $profile1Save = Join-Path $DataRoot 'profiles\profile-1\pokemon_regionalidades.sav'
    if ((Test-Path -LiteralPath $profile1Save -PathType Leaf) -and
        ($cards[1].Status.Text -notlike 'Save ativo*' -or $cards[1].Import.Enabled -or -not $cards[1].Export.Enabled)) {
        throw 'A interface nao reconheceu corretamente o save ativo do perfil 1.'
    }
    $expectedRecoveries = @(Get-ChildItem -LiteralPath (Split-Path $profile1Save) -Filter 'pokemon_regionalidades.sav.recovery-*' -ErrorAction SilentlyContinue).Count
    if ($cards[1].Recoveries.Items.Count -ne $expectedRecoveries) {
        throw 'A interface nao apresentou todas as recuperacoes do perfil 1.'
    }
    $testShortcutPath = Join-Path $DataRoot 'profile-ui-validation.lnk'
    New-RegionalidadesShortcut -ShortcutPath $testShortcutPath
    $testShell = New-Object -ComObject WScript.Shell
    $testShortcut = $testShell.CreateShortcut($testShortcutPath)
    $expectedTarget = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'
    if ($testShortcut.TargetPath -ne $expectedTarget -or
        $testShortcut.Arguments -notlike '*open_player_profiles_pc.ps1*') {
        throw 'A validacao do atalho da interface falhou.'
    }
    [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($testShortcut)
    [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($testShell)
    Remove-Item -LiteralPath $testShortcutPath -Force
    $form.Dispose()
    Write-Output 'PROFILE_UI_TEST_OK'
    return
}

[void]$form.ShowDialog()
$form.Dispose()
