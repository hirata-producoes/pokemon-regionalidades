# Pacote externo de recursos do PC

O alvo PC gera `pokemon_go_world.pak` ao lado de `pokemon_go_world-pc.exe`. O pacote não é uma ROM GBA e não possui o limite de 32 MiB. Ele foi projetado para receber gradualmente gráficos, áudio e outros dados grandes sem aumentar o executável.

## Como adicionar um recurso

1. Adicione uma entrada com nome único e `source` em `resources/pc/manifest.json`. Use `sources` para concatenar vários arquivos, na ordem informada, em uma única entrada.
   Famílias geradas podem ser referenciadas por `resource_lists`; a lista de imagens, paletas, ícones e pegadas de Pokémon é criada em `build/pc-generated/pokemon_resources.json`, a de cries ativos em `build/pc-generated/cry_resources.json`, a dos tilesets ativos em `build/pc-generated/tileset_resources.json`, a dos frames animados em `build/pc-generated/tileset_anim_resources.json`, a das amostras musicais em `build/pc-generated/music_sample_resources.json` e a das faixas MP2K em `build/pc-generated/song_resources.json`.
2. Garanta que o alvo `pc` produza o arquivo de origem se ele for gerado por uma ferramenta do projeto.
3. No código PC, obtenha o dado com `ResourcePack_Get` no momento em que ele for necessário. Tabelas geradas podem evitar nomes duplicados no executável usando `ResourcePack_GetByHash` ou `ResourcePack_LoadByHash`.
4. Mantenha o código GBA sob o caminho original com `#ifndef PORTABLE`.
5. Compile com `tools/pokemon_go_world/build_pc.ps1` e valide o recurso com o pacote presente e ausente.

Os 12 recursos usados pela tela de título Emerald, 2.950 imagens, 2.889 paletas, 1.420 ícones, 1.031 pegadas, 1.067 cries de Pokémon, 306 recursos de tileset, 174 frames de animação de tileset, 106 amostras musicais e 530 faixas MP2K foram removidos do executável PC. Os tilesets abrangem 85 gráficos de tiles, 77 conjuntos de paletas, 72 tabelas de metatiles e 72 tabelas de atributos; as amostras musicais atendem 156 identificadores, incluindo aliases de fonemas compartilhados. Os geradores inspecionam as configurações ativas com o pré-processador C, preservando formas habilitadas, diferenças de gênero e escolhas entre recursos modernos/GBA. Suas definições compiladas permanecem no alvo GBA.

As faixas usam o contêiner interno `PGWSONG` versão 1. O gerador lê os objetos COFF produzidos por `mid2agb`, guarda os dados de track e uma tabela de relocações, e deixa no executável somente um símbolo-placeholder de 8 bytes por música. Ao iniciar uma música, o PC carrega o recurso, reconstrói ponteiros internos de `GOTO`/`PATT` e liga a voicegroup compilada correspondente. Uma faixa ausente ou inválida vira uma música vazia segura; músicas dinâmicas usadas pelos cries continuam aceitas sem conversão.

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

O construtor rejeita nomes duplicados e colisões FNV-1a; o carregador também exige hashes únicos e valida versão, tamanho total, intervalos, ordenação, nomes, hashes e CRC32. Apenas o índice e os nomes entram na memória ao abrir o pacote. Recursos reutilizados podem permanecer em cache com `ResourcePack_Get`/`ResourcePack_GetByHash`; recursos grandes ou numerosos podem usar `ResourcePack_Load`/`ResourcePack_LoadByHash` e ser liberados após o uso. Paletas, ícones, pegadas, cries, componentes de tileset, frames animados, amostras e faixas musicais usam cache sob demanda; imagens comprimidas de Pokémon usam leitura transitória. O mixer mantém ponteiros de cries, instrumentos e tracks durante a reprodução, por isso esses dados permanecem em cache até o encerramento. Assim, um pacote de 100 ou 128 MiB não consome automaticamente a mesma quantidade de RAM.

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
