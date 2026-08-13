# Pacote externo de recursos do PC

O alvo PC gera `pokemon_go_world.pak` ao lado de `pokemon_go_world-pc.exe`. O pacote não é uma ROM GBA e não possui o limite de 32 MiB. Ele foi projetado para receber gradualmente gráficos, áudio e outros dados grandes sem aumentar o executável.

## Como adicionar um recurso

1. Adicione uma entrada com nome único e `source` em `resources/pc/manifest.json`. Use `sources` para concatenar vários arquivos, na ordem informada, em uma única entrada.
2. Garanta que o alvo `pc` produza o arquivo de origem se ele for gerado por uma ferramenta do projeto.
3. No código PC, obtenha o dado com `ResourcePack_Get` no momento em que ele for necessário.
4. Mantenha o código GBA sob o caminho original com `#ifndef PORTABLE`.
5. Compile com `tools/pokemon_go_world/build_pc.ps1` e valide o recurso com o pacote presente e ausente.

Os 12 recursos usados pela tela de título Emerald foram removidos do executável PC: gráficos comprimidos, tilemaps e paletas do logo, Rayquaza, nuvens, versão, brilho e banners. Suas definições compiladas permanecem no alvo GBA.

## Formato versão 1

Todos os inteiros são little-endian. O cabeçalho tem 48 bytes:

| Campo | Tipo |
| --- | --- |
| Magic `PGWPACK\0` | 8 bytes |
| Versão e quantidade | 2 × `u32` |
| Offset do índice | `u64` |
| Offset da tabela de nomes | `u64` |
| Offset inicial dos dados | `u64` |
| Tamanho total declarado | `u64` |

Cada entrada do índice tem 40 bytes: hash FNV-1a de 64 bits, offset e tamanho de dados de 64 bits, offset do nome de 64 bits, comprimento do nome de 32 bits e CRC32 de 32 bits.

O carregador valida versão, tamanho total, intervalos, ordenação, nomes, hashes e CRC32. Apenas o índice e os nomes entram na memória ao abrir o pacote; cada recurso é carregado e mantido em cache somente no primeiro uso. Assim, um pacote de 100 ou 128 MiB não consome automaticamente a mesma quantidade de RAM.

## Comandos úteis

Compilar executável e pacote:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_pc.ps1
```

Listar o conteúdo do pacote:

```powershell
python .\tools\pokemon_go_world\pack_resources.py inspect .\pokemon_go_world.pak
```

Para testar outro arquivo sem alterar a distribuição, defina `POKEMON_GO_WORLD_RESOURCE_PACK` com o caminho desejado antes de iniciar o executável.

## Limites práticos atuais

Os offsets do pacote são de 64 bits, mas o executável ainda é de 32 bits para preservar ponteiros existentes nos scripts e dados do Expansion. O pacote pode ser muito maior que 32 MiB porque não é carregado inteiro; um único recurso precisa caber no espaço de endereçamento do processo. Pacotes de 100 ou 128 MiB são adequados para esta arquitetura desde que sejam divididos em recursos razoáveis.
