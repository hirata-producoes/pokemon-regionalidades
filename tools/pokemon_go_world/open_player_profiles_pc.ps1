param(
    [string]$DataRoot,
    [switch]$ValidateOnly
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName Microsoft.VisualBasic

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$profileRunner = Join-Path $PSScriptRoot 'run_player_profile_pc.ps1'
$settingsUi = Join-Path $PSScriptRoot 'open_pc_settings.ps1'
if ([string]::IsNullOrWhiteSpace($DataRoot)) {
    $DataRoot = Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::LocalApplicationData)) 'Pokemon Regionalidades'
} else {
    $DataRoot = [IO.Path]::GetFullPath($DataRoot)
}
$profilesRoot = Join-Path $DataRoot 'profiles'
$background = [Drawing.Color]::FromArgb(28, 35, 48)
$cardColor = [Drawing.Color]::FromArgb(42, 52, 68)
$buttonColor = [Drawing.Color]::FromArgb(71, 85, 105)
$primaryColor = [Drawing.Color]::FromArgb(59, 130, 246)
$textColor = [Drawing.Color]::FromArgb(241, 245, 249)
$mutedColor = [Drawing.Color]::FromArgb(184, 196, 210)

$form = New-Object Windows.Forms.Form
$form.Text = 'Pokémon Regionalidades — Perfis'
$form.ClientSize = New-Object Drawing.Size(970, 625)
$form.StartPosition = 'CenterScreen'
$form.FormBorderStyle = 'FixedSingle'
$form.MaximizeBox = $false
$form.BackColor = $background
$form.ForeColor = $textColor
$form.Font = New-Object Drawing.Font('Segoe UI', 10)
$form.AutoScaleMode = 'Dpi'

function Add-Label {
    param([string]$Value, [int]$X, [int]$Y, [int]$Width, [int]$Height, [int]$Size = 10)
    $label = New-Object Windows.Forms.Label
    $label.Text = $Value
    $label.Location = New-Object Drawing.Point($X, $Y)
    $label.Size = New-Object Drawing.Size($Width, $Height)
    $label.ForeColor = $textColor
    $label.Font = New-Object Drawing.Font('Segoe UI', $Size)
    $label.AutoEllipsis = $true
    $form.Controls.Add($label)
    return $label
}

function Add-Button {
    param([string]$Value, [int]$X, [int]$Y, [int]$Width, [int]$Height, [bool]$Primary = $false)
    $button = New-Object Windows.Forms.Button
    $button.Text = $Value
    $button.Location = New-Object Drawing.Point($X, $Y)
    $button.Size = New-Object Drawing.Size($Width, $Height)
    $button.FlatStyle = 'Flat'
    $button.BackColor = if ($Primary) { $primaryColor } else { $buttonColor }
    $button.ForeColor = $textColor
    $button.FlatAppearance.BorderSize = 0
    $form.Controls.Add($button)
    return $button
}

function Show-ProfileError {
    param([string]$Message)
    [void][Windows.Forms.MessageBox]::Show($form, $Message, 'Pokémon Regionalidades',
        [Windows.Forms.MessageBoxButtons]::OK, [Windows.Forms.MessageBoxIcon]::Error)
}

function Show-ProfileNotice {
    param([string]$Message)
    [void][Windows.Forms.MessageBox]::Show($form, $Message, 'Pokémon Regionalidades',
        [Windows.Forms.MessageBoxButtons]::OK, [Windows.Forms.MessageBoxIcon]::Information)
}

function Confirm-ProfileAction {
    param([string]$Message)
    return [Windows.Forms.MessageBox]::Show($form, $Message, 'Confirmar',
        [Windows.Forms.MessageBoxButtons]::YesNo, [Windows.Forms.MessageBoxIcon]::Warning) -eq
        [Windows.Forms.DialogResult]::Yes
}

function Get-ExistingProfileIds {
    if (-not (Test-Path -LiteralPath $profilesRoot -PathType Container)) { return @() }
    $ids = foreach ($directory in Get-ChildItem -LiteralPath $profilesRoot -Directory) {
        if ($directory.Name -match '^profile-([1-9][0-9]*)$') {
            [int]$id = 0
            if ([int]::TryParse($Matches[1], [ref]$id)) { $id }
        }
    }
    return @($ids | Sort-Object)
}

function Get-SelectedProfileId {
    if ($null -eq $profileList.SelectedItem) { return $null }
    return [int]$profileList.SelectedItem.Id
}

function Get-SelectedSave {
    return $saveList.SelectedItem
}

function Refresh-ProfileList {
    param([int]$SelectId = 0)
    $profileList.BeginUpdate()
    try {
        $profileList.Items.Clear()
        $selectedIndex = -1
        foreach ($id in @(Get-ExistingProfileIds)) {
            $info = & $profileRunner -Profile $id -DataRoot $DataRoot -GetProfileInfo -PassThru
            $item = [pscustomobject]@{ Id = $id; Text = $info.DisplayName }
            $index = $profileList.Items.Add($item)
            if ($id -eq $SelectId) { $selectedIndex = $index }
        }
        if ($selectedIndex -ge 0) { $profileList.SelectedIndex = $selectedIndex }
        elseif ($profileList.Items.Count -gt 0) { $profileList.SelectedIndex = 0 }
    } finally {
        $profileList.EndUpdate()
    }
    Refresh-ProfileDetails
}

function Refresh-ProfileDetails {
    $id = Get-SelectedProfileId
    $hasProfile = $null -ne $id
    $saveList.Items.Clear()
    $profileName.Text = if ($hasProfile) { $profileList.SelectedItem.Text } else { 'Nenhum perfil selecionado' }
    $profileStatus.Text = if ($hasProfile) { 'Sem campanha iniciada' } else { 'Crie um perfil para começar.' }
    $hasActive = $false
    $favoriteCount = 0
    if ($hasProfile) {
        $entries = @(& $profileRunner -Profile $id -DataRoot $DataRoot -ListRecoveries -PassThru)
        foreach ($entry in $entries) {
            if ($entry.Kind -eq 'Active') { $hasActive = $true }
            if ($entry.Kind -eq 'Favorite') { $favoriteCount++ }
            $generation = if ($null -eq $entry.Geracao) { 'desconhecida' } else { $entry.Geracao }
            $item = [pscustomobject]@{
                Text = "$($entry.Estado) — geração $generation — $($entry.Modificado.ToString('dd/MM/yyyy HH:mm'))"
                Kind = $entry.Kind
                Slot = $entry.Slot
            }
            [void]$saveList.Items.Add($item)
        }
        if ($hasActive) {
            $profileStatus.Text = 'Campanha ativa. Selecione um save abaixo para recuperar ou favoritar.'
        }
    }
    if ($saveList.Items.Count -gt 0) { $saveList.SelectedIndex = 0 }
    $openButton.Enabled = $hasProfile
    $renameButton.Enabled = $hasProfile
    $deleteButton.Enabled = $hasProfile
    $importButton.Enabled = $hasProfile -and -not $hasActive
    $exportButton.Enabled = $hasActive
    $resetButton.Enabled = $hasActive
    $script:currentFavoriteCount = $favoriteCount
    Refresh-SaveButtons
}

function Refresh-SaveButtons {
    $selected = Get-SelectedSave
    $restoreButton.Enabled = $null -ne $selected -and $selected.Kind -ne 'Active'
    $favoriteButton.Enabled = $null -ne $selected -and
        $selected.Kind -ne 'Favorite' -and $script:currentFavoriteCount -lt 5
    $removeFavoriteButton.Enabled = $null -ne $selected -and $selected.Kind -eq 'Favorite'
}

function New-RegionalidadesShortcut {
    param([string]$ShortcutPath)
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

$title = Add-Label 'Pokémon Regionalidades' 28 20 470 42 22
$subtitle = Add-Label 'Escolha um perfil na lista para ver sua campanha' 31 64 580 26 11
$subtitle.ForeColor = $mutedColor
$settingsButton = Add-Button 'Configurações' 650 32 140 36
$shortcutButton = Add-Button 'Criar atalho' 800 32 140 36

$profileList = New-Object Windows.Forms.ListBox
$profileList.Location = New-Object Drawing.Point(28, 112)
$profileList.Size = New-Object Drawing.Size(265, 410)
$profileList.DisplayMember = 'Text'
$profileList.BackColor = $cardColor
$profileList.ForeColor = $textColor
$profileList.IntegralHeight = $false
$form.Controls.Add($profileList)
$createButton = Add-Button 'Criar perfil' 28 535 265 38 $true

$profileName = Add-Label 'Nenhum perfil selecionado' 320 110 620 38 18
$profileStatus = Add-Label 'Crie um perfil para começar.' 320 154 620 52
$profileStatus.ForeColor = $mutedColor
$openButton = Add-Button 'Abrir perfil' 320 204 620 38 $true
$saveLabel = Add-Label 'Save atual, recuperações e favoritos' 320 256 450 26
$saveLabel.ForeColor = $mutedColor
$saveList = New-Object Windows.Forms.ListBox
$saveList.Location = New-Object Drawing.Point(320, 286)
$saveList.Size = New-Object Drawing.Size(460, 160)
$saveList.DisplayMember = 'Text'
$saveList.BackColor = $cardColor
$saveList.ForeColor = $textColor
$saveList.IntegralHeight = $false
$form.Controls.Add($saveList)
$restoreButton = Add-Button 'Restaurar' 792 286 148 36
$favoriteButton = Add-Button 'Favoritar' 792 329 148 36
$removeFavoriteButton = Add-Button 'Remover favorito' 792 372 148 36
$importButton = Add-Button 'Importar save' 320 460 145 36
$exportButton = Add-Button 'Exportar save' 474 460 145 36
$renameButton = Add-Button 'Renomear' 628 460 145 36
$resetButton = Add-Button 'Reiniciar campanha' 320 507 220 36
$deleteButton = Add-Button 'Apagar perfil' 550 507 220 36
$footer = Add-Label 'Cada perfil tem uma campanha, três recuperações rotativas e até cinco favoritos fixos.' 30 583 910 28
$footer.ForeColor = $mutedColor

$profileList.Add_SelectedIndexChanged({ Refresh-ProfileDetails })
$saveList.Add_SelectedIndexChanged({ Refresh-SaveButtons })
$settingsButton.Add_Click({
    try { & $settingsUi -DataRoot $DataRoot } catch { Show-ProfileError $_.Exception.Message }
})
$shortcutButton.Add_Click({
    try {
        $desktop = [Environment]::GetFolderPath([Environment+SpecialFolder]::DesktopDirectory)
        if ([string]::IsNullOrWhiteSpace($desktop)) { throw 'O Windows não informou a área de trabalho.' }
        $path = Join-Path $desktop 'Pokémon Regionalidades.lnk'
        New-RegionalidadesShortcut $path
        Show-ProfileNotice "Atalho criado em:`r`n$path"
    } catch { Show-ProfileError $_.Exception.Message }
})
$createButton.Add_Click({
    try {
        $ids = @(Get-ExistingProfileIds)
        $id = if ($ids.Count -eq 0) { 1 } else { [int]$ids[-1] + 1 }
        if ($id -lt 1) { throw 'O identificador de perfis atingiu o limite do sistema.' }
        $name = [Microsoft.VisualBasic.Interaction]::InputBox(
            'Nome do novo perfil (até 32 caracteres):', 'Criar perfil', "Perfil $id")
        if ([string]::IsNullOrWhiteSpace($name)) { return }
        if ($name.Trim().Length -gt 32 -or $name.IndexOfAny([char[]]"`r`n`t") -ge 0) {
            throw 'O nome precisa ter até 32 caracteres e ocupar uma linha.'
        }
        & $profileRunner -Profile $id -DataRoot $DataRoot -CreateProfile | Out-Null
        & $profileRunner -Profile $id -DataRoot $DataRoot -SetProfileName $name | Out-Null
        Refresh-ProfileList -SelectId $id
    } catch { Show-ProfileError $_.Exception.Message }
})
$openButton.Add_Click({
    try {
        $id = Get-SelectedProfileId
        if ($null -eq $id) { return }
        & $profileRunner -Profile $id -DataRoot $DataRoot
        $form.Close()
    } catch { Show-ProfileError $_.Exception.Message }
})
$renameButton.Add_Click({
    try {
        $id = Get-SelectedProfileId
        if ($null -eq $id) { return }
        $name = [Microsoft.VisualBasic.Interaction]::InputBox('Novo nome do perfil:',
            'Renomear perfil', $profileName.Text)
        if ([string]::IsNullOrWhiteSpace($name)) { return }
        & $profileRunner -Profile $id -DataRoot $DataRoot -SetProfileName $name | Out-Null
        Refresh-ProfileList -SelectId $id
    } catch { Show-ProfileError $_.Exception.Message }
})
$importButton.Add_Click({
    $dialog = New-Object Windows.Forms.OpenFileDialog
    try {
        $dialog.Title = 'Importar save'
        $dialog.Filter = 'Save de Pokémon Regionalidades (*.pgrsave;*.sav)|*.pgrsave;*.sav|Todos os arquivos (*.*)|*.*'
        if ($dialog.ShowDialog($form) -ne [Windows.Forms.DialogResult]::OK) { return }
        $id = Get-SelectedProfileId
        & $profileRunner -Profile $id -DataRoot $DataRoot -ImportSave $dialog.FileName -PrepareOnly | Out-Null
        Refresh-ProfileDetails
        Show-ProfileNotice 'Save importado. O arquivo original foi preservado.'
    } catch { Show-ProfileError $_.Exception.Message } finally { $dialog.Dispose() }
})
$exportButton.Add_Click({
    $dialog = New-Object Windows.Forms.SaveFileDialog
    try {
        $id = Get-SelectedProfileId
        $dialog.Title = 'Exportar save'
        $dialog.Filter = 'Save nativo (*.pgrsave)|*.pgrsave'
        $dialog.DefaultExt = 'pgrsave'
        $dialog.AddExtension = $true
        $dialog.OverwritePrompt = $true
        $dialog.FileName = "pokemon-regionalidades-perfil-$id-$(Get-Date -Format 'yyyyMMdd-HHmm').pgrsave"
        if ($dialog.ShowDialog($form) -ne [Windows.Forms.DialogResult]::OK) { return }
        & $profileRunner -Profile $id -DataRoot $DataRoot -ExportSave $dialog.FileName -AllowExportOverwrite -PrepareOnly | Out-Null
        Show-ProfileNotice "Save exportado para:`r`n$($dialog.FileName)"
    } catch { Show-ProfileError $_.Exception.Message } finally { $dialog.Dispose() }
})
$restoreButton.Add_Click({
    try {
        $id = Get-SelectedProfileId
        $entry = Get-SelectedSave
        if ($null -eq $entry -or -not (Confirm-ProfileAction "Restaurar $($entry.Text)? O save atual será preservado.")) { return }
        if ($entry.Kind -eq 'Recovery') {
            & $profileRunner -Profile $id -DataRoot $DataRoot -RestoreRecovery $entry.Slot | Out-Null
        } elseif ($entry.Kind -eq 'Favorite') {
            & $profileRunner -Profile $id -DataRoot $DataRoot -RestoreFavorite $entry.Slot | Out-Null
        }
        Refresh-ProfileDetails
        Show-ProfileNotice 'Save restaurado; o estado anterior foi preservado.'
    } catch { Show-ProfileError $_.Exception.Message }
})
$favoriteButton.Add_Click({
    try {
        $id = Get-SelectedProfileId
        $entry = Get-SelectedSave
        if ($null -eq $entry -or $entry.Kind -eq 'Favorite') { return }
        & $profileRunner -Profile $id -DataRoot $DataRoot -FavoriteFromSlot $entry.Slot | Out-Null
        Refresh-ProfileDetails
    } catch { Show-ProfileError $_.Exception.Message }
})
$removeFavoriteButton.Add_Click({
    try {
        $id = Get-SelectedProfileId
        $entry = Get-SelectedSave
        if ($null -eq $entry -or $entry.Kind -ne 'Favorite' -or
            -not (Confirm-ProfileAction "Remover $($entry.Text) da lista? Uma cópia recuperável será preservada.")) { return }
        & $profileRunner -Profile $id -DataRoot $DataRoot -RemoveFavorite $entry.Slot | Out-Null
        Refresh-ProfileDetails
    } catch { Show-ProfileError $_.Exception.Message }
})
$resetButton.Add_Click({
    try {
        $id = Get-SelectedProfileId
        if (-not (Confirm-ProfileAction 'Reiniciar esta campanha? O perfil e seu nome serão mantidos. Uma cópia completa será preservada.')) { return }
        $result = & $profileRunner -Profile $id -DataRoot $DataRoot -ResetProfile -PassThru
        Refresh-ProfileList -SelectId $id
        Show-ProfileNotice "Campanha reiniciada. Cópia recuperável:`r`n$($result.BackupPath)"
    } catch { Show-ProfileError $_.Exception.Message }
})
$deleteButton.Add_Click({
    try {
        $id = Get-SelectedProfileId
        if (-not (Confirm-ProfileAction "Apagar o perfil $id da lista? Sua pasta será arquivada e poderá ser recuperada manualmente.")) { return }
        $result = & $profileRunner -Profile $id -DataRoot $DataRoot -DeleteProfile -PassThru
        Refresh-ProfileList
        Show-ProfileNotice "Perfil retirado da lista. Cópia recuperável:`r`n$($result.BackupPath)"
    } catch { Show-ProfileError $_.Exception.Message }
})

Refresh-ProfileList
if ($ValidateOnly) {
    if ($profileList.DisplayMember -ne 'Text' -or $saveList.DisplayMember -ne 'Text' -or
        $createButton.Text -ne 'Criar perfil' -or $favoriteButton.Text -ne 'Favoritar' -or
        $settingsButton.Text -ne 'Configurações') {
        throw 'A validação estrutural da lista de perfis falhou.'
    }
    $form.Dispose()
    Write-Output 'PROFILE_UI_TEST_OK'
    return
}
[void]$form.ShowDialog()
$form.Dispose()
