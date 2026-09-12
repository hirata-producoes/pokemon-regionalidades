function Sync-RegionalidadesDevSessionArtifacts {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RepoRoot,
        [Parameter(Mandatory = $true)]
        [string]$RunDir,
        [Parameter(Mandatory = $true)]
        [string]$SessionKind
    )

    $repoPath = [IO.Path]::GetFullPath($RepoRoot).TrimEnd('\')
    $buildPath = [IO.Path]::GetFullPath((Join-Path $repoPath 'build')).TrimEnd('\') + '\'
    $runPath = [IO.Path]::GetFullPath($RunDir).TrimEnd('\') + '\'

    if (-not $runPath.StartsWith($buildPath, [StringComparison]::OrdinalIgnoreCase)) {
        throw "A sessao de desenvolvimento deve ficar dentro de $buildPath"
    }

    New-Item -ItemType Directory -Path $runPath -Force | Out-Null
    $artifactNames = @(
        'pokemon_regionalidades-pc.exe',
        'pokemon_regionalidades.pak',
        'SDL2.dll'
    )
    $manifestLines = @(
        'Pokemon Regionalidades - sessao isolada de desenvolvimento',
        "tipo=$SessionKind",
        "preparada_utc=$([DateTime]::UtcNow.ToString('yyyy-MM-ddTHH:mm:ssZ'))"
    )

    foreach ($artifactName in $artifactNames) {
        $source = Join-Path $repoPath $artifactName
        $destination = Join-Path $runPath $artifactName
        if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
            throw "Arquivo necessario nao encontrado: $source"
        }
        Copy-Item -LiteralPath $source -Destination $destination -Force
        $hash = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant()
        $length = (Get-Item -LiteralPath $destination).Length
        $manifestLines += "artefato=$artifactName tamanho=$length sha256=$hash"
    }

    $commit = (& git -C $repoPath rev-parse HEAD 2>$null)
    if ($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace($commit)) {
        $manifestLines += "commit=$commit"
    }
    $manifestLines += 'codigo_fonte=arvore_oficial; esta pasta nao contem uma copia do codigo'

    $manifestPath = Join-Path $runPath 'dev-session-manifest.txt'
    Set-Content -LiteralPath $manifestPath -Value $manifestLines -Encoding utf8
    return $manifestPath
}
