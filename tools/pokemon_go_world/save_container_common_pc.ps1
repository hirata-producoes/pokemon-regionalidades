if ($null -eq ('RegionalidadesSaveCrc32' -as [type])) {
    Add-Type -TypeDefinition @'
public static class RegionalidadesSaveCrc32
{
    public static uint Calculate(byte[] data, int offset, int count)
    {
        uint crc = 0xFFFFFFFFu;
        for (int index = 0; index < count; index++)
        {
            crc ^= data[offset + index];
            for (int bit = 0; bit < 8; bit++)
                crc = (crc >> 1) ^ (0xEDB88320u & (uint)-(int)(crc & 1u));
        }
        return ~crc;
    }
}
'@
}

function Get-RegionalidadesSaveDescriptor {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return $null
    }
    $bytes = [IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -eq 131072) {
        return [pscustomobject]@{
            Kind = 'Legacy'
            LegacyOffset = 0L
            LegacySize = 131072L
            ContainerGeneration = $null
            ChunkCount = 0
        }
    }
    if ($bytes.Length -lt 88 -or $bytes.Length -gt (64 * 1024 * 1024) -or
        [Text.Encoding]::ASCII.GetString($bytes, 0, 7) -ne 'PGRSAVE' -or
        $bytes[7] -ne 0) {
        return $null
    }

    $version = [BitConverter]::ToUInt32($bytes, 8)
    $headerSize = [BitConverter]::ToUInt32($bytes, 12)
    $containerGeneration = [BitConverter]::ToUInt64($bytes, 16)
    $chunkCount = [BitConverter]::ToUInt32($bytes, 24)
    $entrySize = [BitConverter]::ToUInt32($bytes, 28)
    if ($version -ne 1 -or $headerSize -ne 40 -or $entrySize -ne 48 -or
        $chunkCount -lt 1 -or $chunkCount -gt 128 -or
        ([uint64]$headerSize + [uint64]$chunkCount * [uint64]$entrySize) -gt [uint64]$bytes.Length) {
        return $null
    }
    $directorySize = [int]($chunkCount * $entrySize)
    if ([BitConverter]::ToUInt32($bytes, 36) -ne
            [RegionalidadesSaveCrc32]::Calculate($bytes, 0, 36) -or
        [BitConverter]::ToUInt32($bytes, 32) -ne
            [RegionalidadesSaveCrc32]::Calculate($bytes, [int]$headerSize, $directorySize)) {
        return $null
    }

    $legacyEntry = $null
    $ids = @{}
    $ranges = @()
    for ($index = 0; $index -lt $chunkCount; $index++) {
        $entryOffset = [int]($headerSize + $index * $entrySize)
        $id = [Text.Encoding]::ASCII.GetString($bytes, $entryOffset, 8).TrimEnd([char]0)
        $offset = [BitConverter]::ToUInt64($bytes, $entryOffset + 16)
        $storedSize = [BitConverter]::ToUInt64($bytes, $entryOffset + 24)
        $logicalSize = [BitConverter]::ToUInt64($bytes, $entryOffset + 32)
        $expectedCrc = [BitConverter]::ToUInt32($bytes, $entryOffset + 40)
        $flags = [BitConverter]::ToUInt32($bytes, $entryOffset + 12)
        $schemaVersion = [BitConverter]::ToUInt32($bytes, $entryOffset + 8)
        $directoryEnd = [uint64]$headerSize + [uint64]$directorySize
        if ($flags -gt 1 -or $ids.ContainsKey($id) -or $offset -lt $directoryEnd -or
            $offset -gt [uint64]$bytes.Length -or
            $storedSize -gt ([uint64]$bytes.Length - $offset) -or
            $logicalSize -ne $storedSize -or
            $storedSize -gt [int]::MaxValue -or
            $expectedCrc -ne [RegionalidadesSaveCrc32]::Calculate($bytes, [int]$offset, [int]$storedSize)) {
            return $null
        }
        $ids[$id] = $true
        foreach ($range in $ranges) {
            if ($storedSize -ne 0 -and $range.Size -ne 0 -and
                $offset -lt ($range.Offset + $range.Size) -and
                $range.Offset -lt ($offset + $storedSize)) {
                return $null
            }
        }
        $ranges += [pscustomobject]@{ Offset = $offset; Size = $storedSize }
        if ($id -eq 'LEGACY') {
            if ($null -ne $legacyEntry -or $schemaVersion -ne 1 -or $storedSize -ne 131072) {
                return $null
            }
            $legacyEntry = [pscustomobject]@{
                Offset = [long]$offset
                Size = [long]$storedSize
            }
        } elseif (($flags -band 1) -ne 0) {
            # Required chunks must be fully understood before a profile may
            # be launched, exported or restored by this version.
            if ($id -eq 'WORLD') {
                if ($schemaVersion -ne 1 -or $storedSize -lt 32 -or
                    [Text.Encoding]::ASCII.GetString($bytes, [int]$offset, 8) -ne 'PGRWORLD' -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 8) -ne 1 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 16) -ne 8 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 20) -ne 4 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 24) -ne 0 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 28) -ne 0) {
                    return $null
                }
                $worldEntryCount = [BitConverter]::ToUInt32($bytes, [int]$offset + 12)
                if ($worldEntryCount -gt 21504 -or
                    $storedSize -ne ([uint64]32 + [uint64]$worldEntryCount * [uint64]8)) {
                    return $null
                }
                $worldEntries = @{}
                for ($worldIndex = 0; $worldIndex -lt $worldEntryCount; $worldIndex++) {
                    $worldOffset = [int]$offset + 32 + $worldIndex * 8
                    $worldType = [uint32]$bytes[$worldOffset]
                    $worldRegion = [uint32]$bytes[$worldOffset + 1]
                    $worldId = [BitConverter]::ToUInt32($bytes, $worldOffset + 4)
                    if ($bytes[$worldOffset + 2] -ne 0 -or $bytes[$worldOffset + 3] -ne 0 -or
                        ($worldType -eq 1 -and ($worldRegion -ge 4 -or $worldId -ge 4096)) -or
                        ($worldType -eq 2 -and ($worldRegion -ne 255 -or $worldId -ge 1024)) -or
                        ($worldType -eq 3 -and ($worldRegion -ge 4 -or $worldId -ge 1024)) -or
                        ($worldType -lt 1 -or $worldType -gt 3)) {
                        return $null
                    }
                    $worldKey = "$worldType/$worldRegion/$worldId"
                    if ($worldEntries.ContainsKey($worldKey)) {
                        return $null
                    }
                    $worldEntries[$worldKey] = $true
                }
            } elseif ($id -eq 'ROTOMDEX') {
                if ($schemaVersion -ne 1 -or $storedSize -ne 24 -or
                    [Text.Encoding]::ASCII.GetString($bytes, [int]$offset, 6) -ne 'PGRDEX' -or
                    $bytes[[int]$offset + 6] -ne 0 -or $bytes[[int]$offset + 7] -ne 0 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 8) -ne 1 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 12) -ne 4 -or
                    ([BitConverter]::ToUInt32($bytes, [int]$offset + 16) -band 0xFFFFFFF0) -ne 0 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 20) -ne 0) {
                    return $null
                }
            } elseif ($id -eq 'INVENT') {
                if ($schemaVersion -ne 1 -or $storedSize -lt 32 -or
                    [Text.Encoding]::ASCII.GetString($bytes, [int]$offset, 6) -ne 'PGRINV' -or
                    $bytes[[int]$offset + 6] -ne 0 -or $bytes[[int]$offset + 7] -ne 0 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 8) -ne 1 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 16) -ne 16 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 20) -ne 6 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 24) -ne 4096 -or
                    [BitConverter]::ToUInt32($bytes, [int]$offset + 28) -gt 1) {
                    return $null
                }
                $inventoryEntryCount = [BitConverter]::ToUInt32($bytes, [int]$offset + 12)
                if ($inventoryEntryCount -gt 24576 -or
                    $storedSize -ne ([uint64]32 + [uint64]$inventoryEntryCount * [uint64]16)) {
                    return $null
                }
                $inventoryEntries = @{}
                for ($inventoryIndex = 0; $inventoryIndex -lt $inventoryEntryCount; $inventoryIndex++) {
                    $inventoryOffset = [int]$offset + 32 + $inventoryIndex * 16
                    $location = [uint32]$bytes[$inventoryOffset]
                    $slot = [BitConverter]::ToUInt32($bytes, $inventoryOffset + 4)
                    $itemId = [BitConverter]::ToUInt32($bytes, $inventoryOffset + 8)
                    $quantity = [BitConverter]::ToUInt32($bytes, $inventoryOffset + 12)
                    if ($bytes[$inventoryOffset + 1] -ne 0 -or
                        $bytes[$inventoryOffset + 2] -ne 0 -or
                        $bytes[$inventoryOffset + 3] -ne 0 -or
                        $location -ge 6 -or $slot -ge 4096 -or
                        $itemId -eq 0 -or $itemId -gt 4095 -or
                        $quantity -eq 0 -or $quantity -gt 99999) {
                        return $null
                    }
                    $inventoryKey = "$location/$slot"
                    if ($inventoryEntries.ContainsKey($inventoryKey)) {
                        return $null
                    }
                    $inventoryEntries[$inventoryKey] = $true
                }
            } else {
                return $null
            }
        }
    }
    if ($null -eq $legacyEntry) {
        return $null
    }

    return [pscustomobject]@{
        Kind = 'Native'
        LegacyOffset = $legacyEntry.Offset
        LegacySize = $legacyEntry.Size
        ContainerGeneration = $containerGeneration
        ChunkCount = $chunkCount
    }
}

function Get-RegionalidadesSaveGeneration {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    $descriptor = Get-RegionalidadesSaveDescriptor -Path $Path
    if ($null -eq $descriptor) {
        return $null
    }
    $bytes = [IO.File]::ReadAllBytes($Path)
    $baseOffset = [long]$descriptor.LegacyOffset
    $sectors = for ($sector = 0; $sector -lt 28; $sector++) {
        $offset = [int]($baseOffset + $sector * 4096)
        $id = [BitConverter]::ToUInt16($bytes, $offset + 4084)
        $signature = [BitConverter]::ToUInt32($bytes, $offset + 4088)
        if ($signature -eq 0x08012025 -and $id -lt 14) {
            [pscustomobject]@{
                Id = $id
                Counter = [BitConverter]::ToUInt32($bytes, $offset + 4092)
            }
        }
    }

    [uint32]$latest = 0
    $hasLatest = $false
    foreach ($group in @($sectors | Group-Object Counter)) {
        if (@($group.Group.Id | Sort-Object -Unique).Count -eq 14) {
            $counter = [uint32]$group.Name
            if (-not $hasLatest -or $counter -gt $latest) {
                $latest = $counter
                $hasLatest = $true
            }
        }
    }
    if ($hasLatest) {
        return $latest
    }
    return $null
}

function Test-RegionalidadesSave {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )
    return $null -ne (Get-RegionalidadesSaveDescriptor -Path $Path)
}

function Convert-RegionalidadesLegacyToNativeBytes {
    param(
        [Parameter(Mandatory = $true)]
        [byte[]]$LegacyBytes,
        [uint64]$Generation = 1
    )

    if ($LegacyBytes.Length -ne 131072) {
        throw 'A imagem legada precisa ter exatamente 128 KiB.'
    }

    [byte[]]$output = New-Object byte[] (40 + 48 + $LegacyBytes.Length)
    [Array]::Copy([Text.Encoding]::ASCII.GetBytes('PGRSAVE'), 0, $output, 0, 7)
    [Array]::Copy([BitConverter]::GetBytes([uint32]1), 0, $output, 8, 4)
    [Array]::Copy([BitConverter]::GetBytes([uint32]40), 0, $output, 12, 4)
    [Array]::Copy([BitConverter]::GetBytes($Generation), 0, $output, 16, 8)
    [Array]::Copy([BitConverter]::GetBytes([uint32]1), 0, $output, 24, 4)
    [Array]::Copy([BitConverter]::GetBytes([uint32]48), 0, $output, 28, 4)

    [Array]::Copy([Text.Encoding]::ASCII.GetBytes('LEGACY'), 0, $output, 40, 6)
    [Array]::Copy([BitConverter]::GetBytes([uint32]1), 0, $output, 48, 4)
    [Array]::Copy([BitConverter]::GetBytes([uint32]1), 0, $output, 52, 4)
    [Array]::Copy([BitConverter]::GetBytes([uint64]88), 0, $output, 56, 8)
    [Array]::Copy([BitConverter]::GetBytes([uint64]$LegacyBytes.Length), 0, $output, 64, 8)
    [Array]::Copy([BitConverter]::GetBytes([uint64]$LegacyBytes.Length), 0, $output, 72, 8)
    $payloadCrc = [RegionalidadesSaveCrc32]::Calculate($LegacyBytes, 0, $LegacyBytes.Length)
    [Array]::Copy([BitConverter]::GetBytes($payloadCrc), 0, $output, 80, 4)
    [Array]::Copy($LegacyBytes, 0, $output, 88, $LegacyBytes.Length)

    $directoryCrc = [RegionalidadesSaveCrc32]::Calculate($output, 40, 48)
    [Array]::Copy([BitConverter]::GetBytes($directoryCrc), 0, $output, 32, 4)
    $headerCrc = [RegionalidadesSaveCrc32]::Calculate($output, 0, 36)
    [Array]::Copy([BitConverter]::GetBytes($headerCrc), 0, $output, 36, 4)
    return ,$output
}
