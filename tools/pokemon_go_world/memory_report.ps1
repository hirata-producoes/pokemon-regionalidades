param(
    [string]$MapPath = "",
    [switch]$Enforce
)

$ErrorActionPreference = "Stop"
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path

if ([string]::IsNullOrWhiteSpace($MapPath)) {
    $MapPath = Join-Path $projectRoot "pokemon_regionalidades.map"
}

if (-not (Test-Path -LiteralPath $MapPath)) {
    throw "Map file not found: $MapPath. Build the ROM first."
}

$romBase = [int64]0x08000000
$romLimit = 32MB
$ewramBase = [int64]0x02000000
$ewramLimit = 256KB
$iwramBase = [int64]0x03000000
$iwramLimit = 32KB
$recommendedRomReserve = 1.5MB
$recommendedEwramReserve = 24KB
$recommendedIwramReserve = 4KB

$romEnd = $romBase
$ewramEnd = $ewramBase
$iwramEnd = $iwramBase
$sectionPattern = '^\.(?<name>\S+)\s+0x(?<address>[0-9A-Fa-f]+)\s+0x(?<size>[0-9A-Fa-f]+)(?:\s+load address 0x(?<load>[0-9A-Fa-f]+))?'

foreach ($line in Get-Content -LiteralPath $MapPath) {
    if ($line -notmatch $sectionPattern) {
        continue
    }

    $address = [Convert]::ToInt64($Matches.address, 16)
    $size = [Convert]::ToInt64($Matches.size, 16)
    $loadAddress = if ($Matches.load) { [Convert]::ToInt64($Matches.load, 16) } else { 0 }

    if ($address -ge $romBase -and $address -lt ($romBase + $romLimit)) {
        $romEnd = [Math]::Max($romEnd, $address + $size)
    }
    # BSS/SBSS occupy RAM but have no payload in the ROM. GNU ld still prints a
    # load address for them, so exclude those sections from the ROM total.
    if ($Matches.name -notmatch '(?:bss|sbss)$' -and $loadAddress -ge $romBase -and $loadAddress -lt ($romBase + $romLimit)) {
        $romEnd = [Math]::Max($romEnd, $loadAddress + $size)
    }
    if ($address -ge $ewramBase -and $address -lt ($ewramBase + $ewramLimit)) {
        $ewramEnd = [Math]::Max($ewramEnd, $address + $size)
    }
    if ($address -ge $iwramBase -and $address -lt ($iwramBase + $iwramLimit)) {
        $iwramEnd = [Math]::Max($iwramEnd, $address + $size)
    }
}

$romUsed = $romEnd - $romBase
$ewramUsed = $ewramEnd - $ewramBase
$iwramUsed = $iwramEnd - $iwramBase

function New-BudgetRow([string]$Area, [int64]$Used, [int64]$Limit, [int64]$Reserve) {
    $free = $Limit - $Used
    [pscustomobject]@{
        Area = $Area
        Used = if ($Limit -le 256KB) { '{0:N2} KiB' -f ($Used / 1KB) } else { '{0:N2} MiB' -f ($Used / 1MB) }
        Free = if ($Limit -le 256KB) { '{0:N2} KiB' -f ($free / 1KB) } else { '{0:N2} MiB' -f ($free / 1MB) }
        Usage = '{0:N2}%' -f (($Used / $Limit) * 100)
        Status = if ($free -lt $Reserve) { 'ATTENTION' } else { 'OK' }
    }
}

$rows = @(
    New-BudgetRow 'ROM' $romUsed $romLimit $recommendedRomReserve
    New-BudgetRow 'EWRAM' $ewramUsed $ewramLimit $recommendedEwramReserve
    New-BudgetRow 'IWRAM' $iwramUsed $iwramLimit $recommendedIwramReserve
)

Write-Host "Pokemon Regionalidades memory budget"
$rows | Format-Table -AutoSize
Write-Host ('ROM available for planned content after the 1.5 MiB safety reserve: {0:N2} MiB' -f (($romLimit - $romUsed - $recommendedRomReserve) / 1MB))

if ($Enforce -and ($rows.Status -contains 'ATTENTION')) {
    throw "A memory safety reserve was crossed. Review the report before adding more content."
}
