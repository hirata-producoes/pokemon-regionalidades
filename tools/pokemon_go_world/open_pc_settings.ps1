param(
    [string]$DataRoot,
    [switch]$ValidateOnly
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

if ([string]::IsNullOrWhiteSpace($DataRoot)) {
    $localData = [Environment]::GetFolderPath([Environment+SpecialFolder]::LocalApplicationData)
    if ([string]::IsNullOrWhiteSpace($localData)) {
        throw 'O Windows não informou o diretório local de dados do usuário.'
    }
    $DataRoot = Join-Path $localData 'Pokemon Regionalidades'
} else {
    $DataRoot = [IO.Path]::GetFullPath($DataRoot)
}

$configDir = Join-Path $DataRoot 'config'
$configPath = Join-Path $configDir 'pokemon_regionalidades.cfg'
$previewState = @{
    OriginalExists = Test-Path -LiteralPath $configPath -PathType Leaf
    OriginalBytes = $null
    TemporaryApplied = $false
    Saved = $false
}
if ($previewState.OriginalExists) {
    $previewState.OriginalBytes = [IO.File]::ReadAllBytes($configPath)
}

$defaults = [ordered]@{
    keyA = 'Z'; keyB = 'X'; keyStart = 'Return'; keySelect = 'Backspace'
    keyL = 'A'; keyR = 'S'; keyUp = 'Up'; keyDown = 'Down'
    keyLeft = 'Left'; keyRight = 'Right'; keySpeed = 'Space'
    controllerA = 'A'; controllerB = 'X'; controllerStart = 'Start'
    controllerSelect = 'Back'; controllerL = 'LB'; controllerR = 'RB'
    controllerSpeed = 'RT'; speedMultiplier = '5'
    fullscreen = '0'; windowScale = '4'; windowResizable = '1'
    integerScale = '0'; vsync = '1'; border = '1'
    volume = '10'; musicVolume = '10'; effectsVolume = '10'
}

$keyboardActions = [ordered]@{
    keyA = 'A — confirmar e interagir'
    keyB = 'B — voltar e correr'
    keyStart = 'Start — menu do jogo'
    keySelect = 'Select'
    keyL = 'L'
    keyR = 'R'
    keyUp = 'Direção para cima'
    keyDown = 'Direção para baixo'
    keyLeft = 'Direção para esquerda'
    keyRight = 'Direção para direita'
    keySpeed = 'Acelerar enquanto pressionada'
}

$controllerActions = [ordered]@{
    controllerA = 'A — confirmar e interagir'
    controllerB = 'B — voltar e correr'
    controllerStart = 'Start — menu do jogo'
    controllerSelect = 'Select'
    controllerL = 'L'
    controllerR = 'R'
    controllerSpeed = 'Acelerar enquanto pressionado'
}

$keyboardChoices = @(
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    '0','1','2','3','4','5','6','7','8','9',
    'Up','Down','Left','Right','Return','Backspace','Space','Tab','Escape',
    'Left Shift','Right Shift','Left Ctrl','Right Ctrl','Left Alt','Right Alt',
    'Insert','Delete','Home','End','PageUp','PageDown'
)
$controllerChoices = @('A','B','X','Y','Start','Back','LB','RB','LeftStick','RightStick','LT','RT')

function Read-RegionalidadesConfig {
    $values = [ordered]@{}
    if (Test-Path -LiteralPath $configPath -PathType Leaf) {
        foreach ($line in [IO.File]::ReadAllLines($configPath)) {
            $separator = $line.IndexOf('=')
            if ($separator -gt 0) {
                $values[$line.Substring(0, $separator)] = $line.Substring($separator + 1)
            }
        }
    }
    foreach ($key in $defaults.Keys) {
        if (-not $values.Contains($key) -or [string]::IsNullOrWhiteSpace([string]$values[$key])) {
            $values[$key] = $defaults[$key]
        }
    }
    return $values
}

function Write-RegionalidadesConfig {
    param(
        [Collections.Specialized.OrderedDictionary]$Values,
        [switch]$Temporary
    )

    New-Item -ItemType Directory -Path $configDir -Force | Out-Null
    $lines = foreach ($key in $Values.Keys) { "$key=$($Values[$key])" }
    $pending = "$configPath.pending-$([guid]::NewGuid().ToString('N'))"
    [IO.File]::WriteAllLines($pending, $lines, [Text.UTF8Encoding]::new($false))
    if (Test-Path -LiteralPath $configPath -PathType Leaf) {
        $backup = if ($Temporary) {
            "$configPath.preview-backup-$([guid]::NewGuid().ToString('N'))"
        } else {
            "$configPath.before-settings-$(Get-Date -Format 'yyyyMMdd-HHmmssfff')"
        }
        try {
            [IO.File]::Replace($pending, $configPath, $backup, $true)
        } finally {
            if ($Temporary -and (Test-Path -LiteralPath $backup -PathType Leaf)) {
                Remove-Item -LiteralPath $backup -Force
            }
            if (Test-Path -LiteralPath $pending -PathType Leaf) {
                Remove-Item -LiteralPath $pending -Force
            }
        }
    } else {
        Move-Item -LiteralPath $pending -Destination $configPath
    }
}

function Restore-OriginalConfig {
    if ($previewState.OriginalExists) {
        New-Item -ItemType Directory -Path $configDir -Force | Out-Null
        $pending = "$configPath.pending-$([guid]::NewGuid().ToString('N'))"
        [IO.File]::WriteAllBytes($pending, $previewState.OriginalBytes)
        if (Test-Path -LiteralPath $configPath -PathType Leaf) {
            $discardedBackup = "$configPath.preview-backup-$([guid]::NewGuid().ToString('N'))"
            try {
                [IO.File]::Replace($pending, $configPath, $discardedBackup, $true)
            } finally {
                if (Test-Path -LiteralPath $discardedBackup -PathType Leaf) {
                    Remove-Item -LiteralPath $discardedBackup -Force
                }
                if (Test-Path -LiteralPath $pending -PathType Leaf) {
                    Remove-Item -LiteralPath $pending -Force
                }
            }
        } else {
            Move-Item -LiteralPath $pending -Destination $configPath
        }
    } elseif (Test-Path -LiteralPath $configPath -PathType Leaf) {
        Remove-Item -LiteralPath $configPath -Force
    }
}

$values = Read-RegionalidadesConfig
$colorBackground = [Drawing.Color]::FromArgb(28, 35, 48)
$colorCard = [Drawing.Color]::FromArgb(42, 52, 68)
$colorPrimary = [Drawing.Color]::FromArgb(59, 130, 246)
$colorSecondary = [Drawing.Color]::FromArgb(71, 85, 105)
$colorText = [Drawing.Color]::FromArgb(241, 245, 249)
$colorMuted = [Drawing.Color]::FromArgb(184, 196, 210)

$form = New-Object Windows.Forms.Form
$form.Text = 'Pokémon Regionalidades — Configurações'
$form.ClientSize = New-Object Drawing.Size(900, 660)
$form.StartPosition = 'CenterScreen'
$form.FormBorderStyle = 'FixedSingle'
$form.MaximizeBox = $false
$form.BackColor = $colorBackground
$form.ForeColor = $colorText
$form.Font = New-Object Drawing.Font('Segoe UI', 10)
$form.AutoScaleMode = 'Dpi'

$title = New-Object Windows.Forms.Label
$title.Text = 'Configurações do jogo'
$title.Font = New-Object Drawing.Font('Segoe UI Semibold', 22)
$title.Location = New-Object Drawing.Point(28, 20)
$title.AutoSize = $true
$form.Controls.Add($title)

$subtitle = New-Object Windows.Forms.Label
$subtitle.Text = 'As configurações são compartilhadas pelos dois perfis e entram em vigor no jogo aberto em até um segundo.'
$subtitle.ForeColor = $colorMuted
$subtitle.Location = New-Object Drawing.Point(31, 65)
$subtitle.AutoSize = $true
$form.Controls.Add($subtitle)

$tabs = New-Object Windows.Forms.TabControl
$tabs.Location = New-Object Drawing.Point(28, 105)
$tabs.Size = New-Object Drawing.Size(844, 450)
$form.Controls.Add($tabs)

$keyboardTab = New-Object Windows.Forms.TabPage
$keyboardTab.Text = 'Teclado'
$keyboardTab.BackColor = $colorCard
$keyboardTab.ForeColor = $colorText
$tabs.TabPages.Add($keyboardTab)

$controllerTab = New-Object Windows.Forms.TabPage
$controllerTab.Text = 'Controle Xbox/XInput'
$controllerTab.BackColor = $colorCard
$controllerTab.ForeColor = $colorText
$tabs.TabPages.Add($controllerTab)

$videoTab = New-Object Windows.Forms.TabPage
$videoTab.Text = 'Vídeo'
$videoTab.BackColor = $colorCard
$videoTab.ForeColor = $colorText
$tabs.TabPages.Add($videoTab)

$audioTab = New-Object Windows.Forms.TabPage
$audioTab.Text = 'Áudio'
$audioTab.BackColor = $colorCard
$audioTab.ForeColor = $colorText
$tabs.TabPages.Add($audioTab)

$selectors = @{}

function Add-Selector {
    param($Parent, [string]$Key, [string]$Label, [string[]]$Choices, [int]$Column, [int]$Row)
    $left = 24 + ($Column * 402)
    $top = 22 + ($Row * 62)
    $labelControl = New-Object Windows.Forms.Label
    $labelControl.Text = $Label
    $labelControl.Location = New-Object Drawing.Point($left, $top)
    $labelControl.Size = New-Object Drawing.Size(235, 24)
    $Parent.Controls.Add($labelControl)
    $combo = New-Object Windows.Forms.ComboBox
    $combo.DropDownStyle = 'DropDownList'
    $combo.Location = New-Object Drawing.Point(($left + 242), ($top - 3))
    $combo.Size = New-Object Drawing.Size(125, 30)
    [void]$combo.Items.AddRange($Choices)
    $combo.SelectedItem = [string]$values[$Key]
    if ($combo.SelectedIndex -lt 0) { $combo.SelectedItem = [string]$defaults[$Key] }
    $Parent.Controls.Add($combo)
    $selectors[$Key] = $combo
}

$index = 0
foreach ($entry in $keyboardActions.GetEnumerator()) {
    Add-Selector -Parent $keyboardTab -Key $entry.Key -Label $entry.Value -Choices $keyboardChoices -Column ([Math]::Floor($index / 6)) -Row ($index % 6)
    $index++
}

$index = 0
foreach ($entry in $controllerActions.GetEnumerator()) {
    Add-Selector -Parent $controllerTab -Key $entry.Key -Label $entry.Value -Choices $controllerChoices -Column ([Math]::Floor($index / 4)) -Row ($index % 4)
    $index++
}

$controllerHelp = New-Object Windows.Forms.Label
$controllerHelp.Text = 'O direcional digital e o analógico esquerdo continuam responsáveis pelo movimento nesta primeira versão.'
$controllerHelp.ForeColor = $colorMuted
$controllerHelp.Location = New-Object Drawing.Point(26, 330)
$controllerHelp.Size = New-Object Drawing.Size(770, 45)
$controllerTab.Controls.Add($controllerHelp)

$videoModeLabel = New-Object Windows.Forms.Label
$videoModeLabel.Text = 'Comportamento da janela'
$videoModeLabel.Location = New-Object Drawing.Point(34, 38)
$videoModeLabel.Size = New-Object Drawing.Size(245, 24)
$videoTab.Controls.Add($videoModeLabel)

$videoMode = New-Object Windows.Forms.ComboBox
$videoMode.DropDownStyle = 'DropDownList'
$videoMode.Location = New-Object Drawing.Point(290, 34)
$videoMode.Size = New-Object Drawing.Size(225, 30)
[void]$videoMode.Items.AddRange(@('Redimensionável', 'Tamanho fixo'))
$videoMode.SelectedItem = if ([int]$values.windowResizable -ne 0) { 'Redimensionável' } else { 'Tamanho fixo' }
$videoTab.Controls.Add($videoMode)

$videoScaleValues = [ordered]@{
    '2× — 640 × 360' = '2'
    '3× — 960 × 540' = '3'
    '4× — 1280 × 720 (recomendado)' = '4'
    '5× — 1600 × 900' = '5'
}
$videoScaleLabel = New-Object Windows.Forms.Label
$videoScaleLabel.Text = 'Tamanho predefinido'
$videoScaleLabel.Location = New-Object Drawing.Point(34, 92)
$videoScaleLabel.Size = New-Object Drawing.Size(245, 24)
$videoTab.Controls.Add($videoScaleLabel)

$videoScale = New-Object Windows.Forms.ComboBox
$videoScale.DropDownStyle = 'DropDownList'
$videoScale.Location = New-Object Drawing.Point(290, 88)
$videoScale.Size = New-Object Drawing.Size(225, 30)
[void]$videoScale.Items.AddRange([string[]]$videoScaleValues.Keys)
$videoScale.SelectedItem = [string]($videoScaleValues.GetEnumerator() | Where-Object { $_.Value -eq [string]$values.windowScale } | Select-Object -First 1).Key
if ($videoScale.SelectedIndex -lt 0) { $videoScale.SelectedItem = '4× — 1280 × 720 (recomendado)' }
$videoTab.Controls.Add($videoScale)

$fullscreenCheck = New-Object Windows.Forms.CheckBox
$fullscreenCheck.Text = 'Usar tela cheia — Alt + Enter para voltar à janela'
$fullscreenCheck.Location = New-Object Drawing.Point(38, 154)
$fullscreenCheck.Size = New-Object Drawing.Size(370, 28)
$fullscreenCheck.Checked = [int]$values.fullscreen -ne 0
$videoTab.Controls.Add($fullscreenCheck)

$integerScaleCheck = New-Object Windows.Forms.CheckBox
$integerScaleCheck.Text = 'Usar escala inteira para preservar pixels'
$integerScaleCheck.Location = New-Object Drawing.Point(38, 198)
$integerScaleCheck.Size = New-Object Drawing.Size(360, 28)
$integerScaleCheck.Checked = [int]$values.integerScale -ne 0
$videoTab.Controls.Add($integerScaleCheck)

$vsyncCheck = New-Object Windows.Forms.CheckBox
$vsyncCheck.Text = 'Sincronização vertical (VSync)'
$vsyncCheck.Location = New-Object Drawing.Point(430, 154)
$vsyncCheck.Size = New-Object Drawing.Size(320, 28)
$vsyncCheck.Checked = [int]$values.vsync -ne 0
$videoTab.Controls.Add($vsyncCheck)

$borderCheck = New-Object Windows.Forms.CheckBox
$borderCheck.Text = 'Exibir moldura do jogo'
$borderCheck.Location = New-Object Drawing.Point(430, 198)
$borderCheck.Size = New-Object Drawing.Size(320, 28)
$borderCheck.Checked = [int]$values.border -ne 0
$videoTab.Controls.Add($borderCheck)

$videoHelp = New-Object Windows.Forms.Label
$videoHelp.Text = 'O tamanho 4× é recomendado. No modo redimensionável, a imagem usa a maior área possível sem deformar. Escala inteira pode criar margens para manter pixels exatos.'
$videoHelp.ForeColor = $colorMuted
$videoHelp.Location = New-Object Drawing.Point(38, 270)
$videoHelp.Size = New-Object Drawing.Size(750, 55)
$videoTab.Controls.Add($videoHelp)

function Add-VolumeSlider {
    param($Parent, [string]$Label, [int]$Top, [int]$Value)
    $labelControl = New-Object Windows.Forms.Label
    $labelControl.Text = $Label
    $labelControl.Location = New-Object Drawing.Point(42, $Top)
    $labelControl.Size = New-Object Drawing.Size(220, 26)
    $Parent.Controls.Add($labelControl)
    $slider = New-Object Windows.Forms.TrackBar
    $slider.Minimum = 0
    $slider.Maximum = 10
    $slider.TickFrequency = 1
    $slider.SmallChange = 1
    $slider.LargeChange = 1
    $slider.Value = [Math]::Min(10, [Math]::Max(0, $Value))
    $slider.Location = New-Object Drawing.Point(270, ($Top - 8))
    $slider.Size = New-Object Drawing.Size(420, 50)
    $Parent.Controls.Add($slider)
    $valueLabel = New-Object Windows.Forms.Label
    $valueLabel.Text = [string]$slider.Value
    $valueLabel.Location = New-Object Drawing.Point(710, $Top)
    $valueLabel.Size = New-Object Drawing.Size(45, 26)
    $Parent.Controls.Add($valueLabel)
    $slider.Add_ValueChanged({ $valueLabel.Text = [string]$slider.Value }.GetNewClosure())
    $slider.Add_MouseDown({
        param($sender, $eventArgs)
        if ($eventArgs.Button -eq [Windows.Forms.MouseButtons]::Left) {
            $thumbMargin = 10
            $usableWidth = [Math]::Max(1, $sender.ClientSize.Width - (2 * $thumbMargin))
            $pointX = [Math]::Min($usableWidth, [Math]::Max(0, $eventArgs.X - $thumbMargin))
            $range = $sender.Maximum - $sender.Minimum
            $sender.Value = $sender.Minimum + [int][Math]::Round(($pointX * $range) / $usableWidth)
        }
    })
    return $slider
}

$masterVolume = Add-VolumeSlider -Parent $audioTab -Label 'Volume geral' -Top 55 -Value ([int]$values.volume)
$musicVolume = Add-VolumeSlider -Parent $audioTab -Label 'Música' -Top 135 -Value ([int]$values.musicVolume)
$effectsVolume = Add-VolumeSlider -Parent $audioTab -Label 'Efeitos e gritos de Pokémon' -Top 215 -Value ([int]$values.effectsVolume)

$audioHelp = New-Object Windows.Forms.Label
$audioHelp.Text = '0 deixa a categoria muda e 10 usa o volume máximo. O volume geral é aplicado depois dos controles separados.'
$audioHelp.ForeColor = $colorMuted
$audioHelp.Location = New-Object Drawing.Point(42, 310)
$audioHelp.Size = New-Object Drawing.Size(750, 45)
$audioTab.Controls.Add($audioHelp)

$speedLabel = New-Object Windows.Forms.Label
$speedLabel.Text = 'Velocidade da aceleração:'
$speedLabel.Location = New-Object Drawing.Point(32, 578)
$speedLabel.AutoSize = $true
$form.Controls.Add($speedLabel)

$speed = New-Object Windows.Forms.NumericUpDown
$speed.Minimum = 2
$speed.Maximum = 10
$speed.Value = [Math]::Min(10, [Math]::Max(2, [int]$values.speedMultiplier))
$speed.Location = New-Object Drawing.Point(205, 574)
$speed.Size = New-Object Drawing.Size(62, 30)
$form.Controls.Add($speed)

$speedSuffix = New-Object Windows.Forms.Label
$speedSuffix.Text = 'vezes'
$speedSuffix.Location = New-Object Drawing.Point(275, 578)
$speedSuffix.AutoSize = $true
$form.Controls.Add($speedSuffix)

$defaultsButton = New-Object Windows.Forms.Button
$defaultsButton.Text = 'Restaurar padrões'
$defaultsButton.Location = New-Object Drawing.Point(370, 570)
$defaultsButton.Size = New-Object Drawing.Size(145, 38)
$defaultsButton.BackColor = $colorSecondary
$defaultsButton.ForeColor = $colorText
$form.Controls.Add($defaultsButton)

$applyButton = New-Object Windows.Forms.Button
$applyButton.Text = 'Aplicar'
$applyButton.Location = New-Object Drawing.Point(525, 570)
$applyButton.Size = New-Object Drawing.Size(100, 38)
$applyButton.BackColor = $colorSecondary
$applyButton.ForeColor = $colorText
$form.Controls.Add($applyButton)

$cancelButton = New-Object Windows.Forms.Button
$cancelButton.Text = 'Cancelar'
$cancelButton.Location = New-Object Drawing.Point(635, 570)
$cancelButton.Size = New-Object Drawing.Size(100, 38)
$cancelButton.BackColor = $colorSecondary
$cancelButton.ForeColor = $colorText
$form.Controls.Add($cancelButton)

$saveButton = New-Object Windows.Forms.Button
$saveButton.Text = 'Salvar'
$saveButton.Location = New-Object Drawing.Point(745, 570)
$saveButton.Size = New-Object Drawing.Size(115, 38)
$saveButton.BackColor = $colorPrimary
$saveButton.ForeColor = [Drawing.Color]::White
$form.Controls.Add($saveButton)

$applyStatus = New-Object Windows.Forms.Label
$applyStatus.Text = 'Aplicar permite testar sem salvar. Cancelar desfaz as alterações temporárias.'
$applyStatus.ForeColor = $colorMuted
$applyStatus.Location = New-Object Drawing.Point(32, 620)
$applyStatus.Size = New-Object Drawing.Size(828, 24)
$form.Controls.Add($applyStatus)

function Update-ValuesFromForm {
    $keyboardSelected = @($keyboardActions.Keys | ForEach-Object { [string]$selectors[$_].SelectedItem })
    if (@($keyboardSelected | Sort-Object -Unique).Count -ne $keyboardSelected.Count) {
        throw 'Cada função do teclado precisa usar uma tecla diferente.'
    }
    $controllerSelected = @($controllerActions.Keys | ForEach-Object { [string]$selectors[$_].SelectedItem })
    if (@($controllerSelected | Sort-Object -Unique).Count -ne $controllerSelected.Count) {
        throw 'Cada função do controle precisa usar um botão diferente.'
    }
    foreach ($key in $selectors.Keys) { $values[$key] = [string]$selectors[$key].SelectedItem }
    $values.speedMultiplier = [string][int]$speed.Value
    $values.windowResizable = if ($videoMode.SelectedItem -eq 'Redimensionável') { '1' } else { '0' }
    $values.windowScale = [string]$videoScaleValues[[string]$videoScale.SelectedItem]
    $values.fullscreen = if ($fullscreenCheck.Checked) { '1' } else { '0' }
    $values.integerScale = if ($integerScaleCheck.Checked) { '1' } else { '0' }
    $values.vsync = if ($vsyncCheck.Checked) { '1' } else { '0' }
    $values.border = if ($borderCheck.Checked) { '1' } else { '0' }
    $values.volume = [string]$masterVolume.Value
    $values.musicVolume = [string]$musicVolume.Value
    $values.effectsVolume = [string]$effectsVolume.Value
}

$defaultsButton.Add_Click({
    foreach ($key in $selectors.Keys) { $selectors[$key].SelectedItem = [string]$defaults[$key] }
    $speed.Value = [int]$defaults.speedMultiplier
    $videoMode.SelectedItem = 'Redimensionável'
    $videoScale.SelectedItem = '4× — 1280 × 720 (recomendado)'
    $fullscreenCheck.Checked = $false
    $integerScaleCheck.Checked = $false
    $vsyncCheck.Checked = $true
    $borderCheck.Checked = $true
    $masterVolume.Value = 10
    $musicVolume.Value = 10
    $effectsVolume.Value = 10
})
$cancelButton.Add_Click({ $form.Close() })
$applyButton.Add_Click({
    try {
        Update-ValuesFromForm
        Write-RegionalidadesConfig -Values $values -Temporary
        $previewState.TemporaryApplied = $true
        $applyStatus.Text = 'Alterações aplicadas temporariamente. Salve para mantê-las ou cancele para desfazer.'
        $applyStatus.ForeColor = [Drawing.Color]::FromArgb(134, 239, 172)
    } catch {
        [void][Windows.Forms.MessageBox]::Show($form, $_.Exception.Message, 'Pokémon Regionalidades', 'OK', 'Error')
    }
})
$saveButton.Add_Click({
    try {
        Update-ValuesFromForm
        if ($previewState.TemporaryApplied) {
            Restore-OriginalConfig
            $previewState.TemporaryApplied = $false
        }
        Write-RegionalidadesConfig -Values $values
        $previewState.Saved = $true
        [void][Windows.Forms.MessageBox]::Show($form, 'Configurações salvas.', 'Pokémon Regionalidades', 'OK', 'Information')
        $form.Close()
    } catch {
        [void][Windows.Forms.MessageBox]::Show($form, $_.Exception.Message, 'Pokémon Regionalidades', 'OK', 'Error')
    }
})

$form.Add_FormClosing({
    param($sender, $eventArgs)
    if ($previewState.TemporaryApplied -and -not $previewState.Saved) {
        try {
            Restore-OriginalConfig
            $previewState.TemporaryApplied = $false
        } catch {
            $eventArgs.Cancel = $true
            [void][Windows.Forms.MessageBox]::Show($form, 'Não foi possível desfazer a configuração temporária: ' + $_.Exception.Message, 'Pokémon Regionalidades', 'OK', 'Error')
        }
    }
})

if ($ValidateOnly) {
    if ($selectors.Count -ne 18 -or $tabs.TabPages.Count -ne 4 -or $speed.Minimum -ne 2 -or $speed.Maximum -ne 10 -or
        $videoMode.Items.Count -ne 2 -or $videoScale.Items.Count -ne 4 -or
        $masterVolume.Minimum -ne 0 -or $masterVolume.Maximum -ne 10 -or
        $musicVolume.Minimum -ne 0 -or $effectsVolume.Maximum -ne 10 -or
        $applyButton.Text -ne 'Aplicar') {
        throw 'A validação estrutural da tela de configurações falhou.'
    }
    foreach ($selectorEntry in $selectors.GetEnumerator()) {
        $selector = $selectorEntry.Value
        if ($selector.Right -gt ($tabs.Width - 16) -or $selector.Bottom -gt ($tabs.Height - 40)) {
            throw "A tela de configurações possui um seletor fora da área visível: $($selectorEntry.Key)."
        }
    }
    $previewValues = [ordered]@{}
    foreach ($key in $values.Keys) { $previewValues[$key] = [string]$values[$key] }
    $previewValues.speedMultiplier = if ($previewValues.speedMultiplier -eq '9') { '8' } else { '9' }
    Write-RegionalidadesConfig -Values $previewValues -Temporary
    if ((Read-RegionalidadesConfig).speedMultiplier -ne $previewValues.speedMultiplier) {
        throw 'A configuração temporária não foi aplicada durante a validação.'
    }
    Restore-OriginalConfig
    if ($previewState.OriginalExists) {
        $restoredBytes = [IO.File]::ReadAllBytes($configPath)
        if ([Convert]::ToBase64String($restoredBytes) -ne [Convert]::ToBase64String($previewState.OriginalBytes)) {
            throw 'O cancelamento não restaurou exatamente a configuração original.'
        }
    } elseif (Test-Path -LiteralPath $configPath -PathType Leaf) {
        throw 'O cancelamento deixou uma configuração temporária que não existia antes.'
    }
    $form.Dispose()
    Write-Output 'PC_SETTINGS_UI_TEST_OK'
    return
}

[void]$form.ShowDialog()
$form.Dispose()
