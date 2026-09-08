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

$defaults = [ordered]@{
    keyA = 'Z'; keyB = 'X'; keyStart = 'Return'; keySelect = 'Backspace'
    keyL = 'A'; keyR = 'S'; keyUp = 'Up'; keyDown = 'Down'
    keyLeft = 'Left'; keyRight = 'Right'; keySpeed = 'Space'
    controllerA = 'A'; controllerB = 'X'; controllerStart = 'Start'
    controllerSelect = 'Back'; controllerL = 'LB'; controllerR = 'RB'
    controllerSpeed = 'RT'; speedMultiplier = '5'
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
    param([Collections.Specialized.OrderedDictionary]$Values)

    New-Item -ItemType Directory -Path $configDir -Force | Out-Null
    $lines = foreach ($key in $Values.Keys) { "$key=$($Values[$key])" }
    $pending = "$configPath.pending-$([guid]::NewGuid().ToString('N'))"
    [IO.File]::WriteAllLines($pending, $lines, [Text.UTF8Encoding]::new($false))
    if (Test-Path -LiteralPath $configPath -PathType Leaf) {
        $backup = "$configPath.before-settings-$(Get-Date -Format 'yyyyMMdd-HHmmssfff')"
        [IO.File]::Replace($pending, $configPath, $backup, $true)
    } else {
        Move-Item -LiteralPath $pending -Destination $configPath
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
$title.Text = 'Controles e aceleração'
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
$defaultsButton.Location = New-Object Drawing.Point(485, 570)
$defaultsButton.Size = New-Object Drawing.Size(155, 38)
$defaultsButton.BackColor = $colorSecondary
$defaultsButton.ForeColor = $colorText
$form.Controls.Add($defaultsButton)

$cancelButton = New-Object Windows.Forms.Button
$cancelButton.Text = 'Cancelar'
$cancelButton.Location = New-Object Drawing.Point(650, 570)
$cancelButton.Size = New-Object Drawing.Size(100, 38)
$cancelButton.BackColor = $colorSecondary
$cancelButton.ForeColor = $colorText
$form.Controls.Add($cancelButton)

$saveButton = New-Object Windows.Forms.Button
$saveButton.Text = 'Salvar'
$saveButton.Location = New-Object Drawing.Point(760, 570)
$saveButton.Size = New-Object Drawing.Size(100, 38)
$saveButton.BackColor = $colorPrimary
$saveButton.ForeColor = [Drawing.Color]::White
$form.Controls.Add($saveButton)

$defaultsButton.Add_Click({
    foreach ($key in $selectors.Keys) { $selectors[$key].SelectedItem = [string]$defaults[$key] }
    $speed.Value = [int]$defaults.speedMultiplier
})
$cancelButton.Add_Click({ $form.Close() })
$saveButton.Add_Click({
    try {
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
        Write-RegionalidadesConfig -Values $values
        [void][Windows.Forms.MessageBox]::Show($form, 'Configurações salvas.', 'Pokémon Regionalidades', 'OK', 'Information')
        $form.Close()
    } catch {
        [void][Windows.Forms.MessageBox]::Show($form, $_.Exception.Message, 'Pokémon Regionalidades', 'OK', 'Error')
    }
})

if ($ValidateOnly) {
    if ($selectors.Count -ne 18 -or $tabs.TabPages.Count -ne 2 -or $speed.Minimum -ne 2 -or $speed.Maximum -ne 10) {
        throw 'A validação estrutural da tela de configurações falhou.'
    }
    foreach ($selectorEntry in $selectors.GetEnumerator()) {
        $selector = $selectorEntry.Value
        if ($selector.Right -gt ($tabs.Width - 16) -or $selector.Bottom -gt ($tabs.Height - 40)) {
            throw "A tela de configurações possui um seletor fora da área visível: $($selectorEntry.Key)."
        }
    }
    $form.Dispose()
    Write-Output 'PC_SETTINGS_UI_TEST_OK'
    return
}

[void]$form.ShowDialog()
$form.Dispose()
