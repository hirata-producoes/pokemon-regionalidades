# Relatório técnico do porte nativo para PC

**Projeto:** Pokémon GO World

**Base:** pokeemerald-expansion + camada multiplataforma SDL2

**Ramo do porte:** `feature/pc-port`

**Revisão auditada:** `d5355f87d`

**Data da auditoria:** 30 de agosto de 2026

## 1. Objetivo e escopo

Este documento registra como o projeto foi convertido de uma aplicação destinada ao hardware Game Boy Advance para um executável nativo de Windows, quais componentes foram reutilizados, onde cada parte está, como elas se conectam, o histórico dos marcos, o que foi validado e o estado exato do trabalho atual.

O objetivo não é transformar uma ROM pronta em um executável por conversão binária. O porte recompila o **código-fonte descompilado** do jogo para a arquitetura x86 e substitui os serviços dependentes do GBA por implementações para PC. Assim, a lógica do jogo — mapas, scripts, Pokémon, batalhas, menus e regras do Expansion — continua sendo a mesma, mas vídeo, áudio, entrada, relógio, salvamento e acesso a recursos passam por uma camada de plataforma baseada em SDL2.

O segundo objetivo é remover progressivamente os grandes recursos do executável e armazená-los em `pokemon_go_world.pak`. Isso elimina a dependência prática do teto de ROM de 32 MiB: o conteúdo do PC passa a estar limitado pelo sistema de arquivos, memória e formatos internos, não pelo espaço de endereçamento do cartucho GBA.

Este relatório descreve o estado observado. Ele não declara como concluído o que ainda está em desenvolvimento.

## 2. Base real e correção de nomenclatura

O repositório atualmente auditado é identificado pelo Git como:

- `expansion/1.16.3-89-gd5355f87d-dirty` no porte de PC;
- `expansion/1.16.3-69-g84a694709-dirty` no projeto GBA original.

Portanto, embora o planejamento inicial tenha citado “pokeemerald-expansion 1.13”, a árvore de código efetivamente usada está baseada na linha **pokeemerald-expansion 1.16.3**, com alterações adicionais do Pokémon GO World. Esta distinção é importante para reproduzir builds, comparar APIs e buscar documentação compatível.

## 3. Onde estão o projeto, a ROM e os artefatos

### 3.1 Projeto GBA preservado

`C:/Users/Rafael/Documents/Codex/2026-07-31/vers-o-1-131-0-user/work/pokemon-go-world`

- ramo: `feature/pokemon-go-world-foundation`;
- ROM principal: `pokemon_go_world.gba`;
- tamanho da ROM: 33.554.432 bytes, exatamente **32 MiB**;
- estado: contém as alterações do Pokémon GO World e já era uma árvore de trabalho modificada.

Os arquivos `data/mb_berry_fix.gba`, `data/mb_colosseum.gba` e `data/mb_ereader.gba` são imagens auxiliares de multiboot. Eles não são a ROM principal do jogo.

### 3.2 Porte nativo para PC

`C:/Users/Rafael/Documents/Codex/2026-08-02/pokemon-go-world-pc`

- ramo: `feature/pc-port`;
- executável: `pokemon_go_world-pc.exe`;
- pacote externo: `pokemon_go_world.pak`;
- save: `pokemon_go_world.sav`;
- biblioteca de execução: `SDL2.dll`.

O porte foi desenvolvido em uma worktree e em um ramo separados. Isso preserva o alvo GBA e permite evoluir o alvo PC sem substituir a ROM original. “Preservado” aqui significa separação de histórico e diretório, não uma alegação de que a worktree original esteja limpa.

### 3.3 Documentação já existente

Os documentos complementares estão em `docs/pokemon_go_world`:

- `PROJECT.md` — visão geral do projeto;
- `MEMORY_BUDGET.md` — orçamento e análise de memória;
- `PC_PORT.md` — instruções resumidas do porte;
- `RESOURCE_PACK.md` — especificação do pacote externo;
- `THIRD_PARTY_NOTICES.md` — atribuições e aviso de terceiros;
- este arquivo — relatório consolidado, histórico e estado auditado.

## 4. Referências e fontes

### 4.1 Repositórios e documentação técnica

- [pret/pokeemerald](https://github.com/pret/pokeemerald) — descompilação comunitária usada como fundação histórica.
- [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion) — base expandida efetivamente registrada como `origin`.
- [Documentação do pokeemerald-expansion](https://rh-hideout.github.io/pokeemerald-expansion/) — documentação pública da base.
- [gradenGnostic/pokeemerald-multiplatform](https://github.com/gradenGnostic/pokeemerald-multiplatform) — fonte da camada multiplataforma reaproveitada e adaptada.
- [Commit multiplataforma fixado: 2f35335e](https://github.com/gradenGnostic/pokeemerald-multiplatform/commit/2f35335eff69ea1a08ebf19e64ea4d97ff6c0a05) — revisão usada como referência para tornar a importação reproduzível.
- [SDL2](https://github.com/libsdl-org/SDL) — janela, entrada, áudio e integração com o sistema operacional.
- [WinLibs](https://winlibs.com/) — distribuição MinGW-w64 usada para compilar o executável Windows de 32 bits.

### 4.2 Fontes internas que sustentam este relatório

O estado e as métricas deste documento foram obtidos de:

- histórico e metadados do Git;
- `Makefile`, `Makefile_pc` e `tools/pokemon_go_world/build_pc.ps1`;
- camada de plataforma em `src/platform` e `include/platform`;
- carregador do pacote em `src/platform/resource_pack.c`;
- geradores em `tools/pokemon_go_world`;
- manifesto `resources/pc/manifest.json`;
- artefatos e logs em `build`;
- capturas automatizadas em diretórios `build/pc-port-*`;
- tamanhos dos executáveis, objetos e pacotes efetivamente produzidos.

### 4.3 Licenciamento e limites de atribuição

`THIRD_PARTY_NOTICES.md` contém a licença MIT relativa às modificações do porte multiplataforma original. Esse aviso **não concede direitos** sobre Pokémon, assets da Nintendo/Game Freak, pokeemerald ou pokeemerald-expansion. Antes de qualquer distribuição pública ou comercial, é necessário revisar separadamente as licenças do código e os direitos sobre todos os recursos usados.

## 5. O que “converter para PC” significa tecnicamente

A conversão segue esta divisão:

| Parte | No GBA | No PC |
|---|---|---|
| CPU/código | ARM/Thumb | x86 de 32 bits |
| Janela e vídeo | registradores, VRAM, PPU | SDL2 + renderizador que interpreta o estado simulado |
| Entrada | registrador de teclas | teclado/controle lido pela SDL2 |
| Áudio | hardware PSG/MP2K | mixer em software + fila de áudio SDL2 |
| Save | flash do cartucho | arquivo `pokemon_go_world.sav` |
| RTC | hardware/rotinas SIO | relógio do sistema operacional |
| ROM/recursos | endereços fixos na ROM | executável + `pokemon_go_world.pak` |
| BIOS/DMA | serviços do GBA | implementações portáveis em C |

Não há emulação completa de um processador ARM executando a ROM. O executável contém a lógica do jogo recompilada. A camada de plataforma fornece comportamentos equivalentes aos serviços de hardware dos quais essa lógica dependia.

O alvo continua sendo **32 bits**, mas isso não é o limite de 32 MiB da ROM. São conceitos diferentes:

- 32 MiB é o tamanho máximo normal da janela de ROM do GBA;
- 32 bits é a arquitetura do processo compilado;
- o pacote externo pode crescer muito além de 32 MiB;
- o processo de 32 bits ainda possui um limite de espaço virtual de memória, por isso os dados externos são carregados sob demanda.

## 6. Separação entre o alvo GBA e o alvo PC

O `Makefile` principal mantém o fluxo tradicional do GBA e acrescenta:

```make
pc:
	$(MAKE) -f Makefile_pc
```

`Makefile_pc` é o arquivo específico do porte. Ele:

- habilita `PORTABLE=1`;
- seleciona `PLATFORM_SDL2`;
- seleciona `RENDERER_EASY_DRAW`;
- compila para i686 com `-m32`;
- usa `-mno-ms-bitfields` para manter layouts de estruturas esperados pelo código;
- usa `-fleading-underscore` para compatibilidade com símbolos gerados/importados;
- adiciona os fontes de plataforma;
- gera os manifestos e os blobs externos;
- constrói `pokemon_go_world.pak`;
- vincula SDL2, WinMM e XInput;
- copia `SDL2.dll` para junto do executável.

Essa estrutura permite usar `make pc` sem destruir o mecanismo de criação da ROM GBA.

## 7. Cadeia de compilação no Windows

### 7.1 Dependências auditadas

- MinGW-w64 i686: `C:/Users/Rafael/Documents/Codex/2026-08-02/toolchains/winlibs-i686-r4-tar/mingw32`
- SDL2 i686: `C:/Users/Rafael/Documents/Codex/2026-08-02/toolchains/SDL2-2.30.7/SDL2-2.30.7/i686-w64-mingw32`
- Python: `C:/Users/Rafael/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe`

### 7.2 Comando principal

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_pc.ps1 -Jobs 4
```

O script configura caminhos, invoca `Makefile_pc`, agrupa objetos em `build/pc-native/objects.rsp` para evitar ultrapassar o limite de tamanho da linha de comando e copia as dependências de execução.

### 7.3 Produtos do build

1. Objetos nativos são criados em `build/pc-native`.
2. Geradores Python produzem código, manifestos intermediários e blobs em `build/pc-generated`.
3. `pack_resources.py` monta `pokemon_go_world.pak`.
4. O linker cria `pokemon_go_world-pc.exe`.
5. `SDL2.dll` é colocada ao lado do executável.

`build/pc-generated` é derivado. Arquivos ali não devem ser editados manualmente; deve-se alterar a fonte ou o gerador e recompilar.

## 8. Arquitetura de execução

### 8.1 Inicialização

O ponto de entrada ativo está em `src/platform/sdl2.c`. Em ordem simplificada, `main`:

1. inicializa a SDL2;
2. abre o pacote indicado por `POKEMON_GO_WORLD_RESOURCE_PACK` ou, por padrão, `pokemon_go_world.pak`;
3. resolve os caminhos de `pokemon_go_world.sav` e `pokemon_go_world.cfg`;
4. lê o save existente;
5. cria janela, renderer e textura;
6. abre o dispositivo de áudio;
7. inicializa o RTC;
8. cria uma thread para `DoMain`, que chama `AgbMain`;
9. mantém o loop de eventos, desenho e áudio até o encerramento.

Este desenho preserva o ciclo principal esperado pelo código original, enquanto o thread principal do sistema operacional administra SDL2.

### 8.2 Renderização

`src/platform/gba_easy_draw.c` interpreta a representação de telas, fundos, sprites, paletas e registradores simulados pelo jogo. `VDraw`, em `sdl2.c`:

1. pede a composição do frame;
2. converte pixels BGR555, formato usado pelo GBA, para ARGB8888;
3. atualiza uma textura SDL;
4. escala e apresenta a textura na janela.

Isso não é uma reimplementação artística dos menus ou mapas. O renderizador consome o estado gráfico que o jogo original produz e o apresenta com APIs de PC.

### 8.3 Entrada

`Platform_GetKeyInput` traduz teclado e controles para o conjunto de botões esperado pelo GBA. O jogo continua lendo A, B, Start, Select, direcional, L e R por sua abstração original.

### 8.4 Áudio

O caminho principal envolve:

- `src/music_player.c` — execução de sequências musicais;
- `src/sound_mixer.c` — mistura em software;
- `src/m4a.c` e `src/sound.c` — interface e lógica MP2K;
- `src/platform/cgb_audio.c` — canais no estilo CGB/PSG;
- `Platform_QueueAudio` em `sdl2.c` — envio das amostras para a fila de áudio da SDL2.

Músicas, amostras e voicegroups já podem vir do pacote externo. O mixer recebe estruturas reconstruídas na memória, mantendo as referências que o motor musical espera.

### 8.5 Save

As rotinas de flash em `src/agb_flash.c`, `src/agb_flash_1m.c`, `src/agb_flash_dummy.c` e `src/save.c` foram conectadas a um arquivo no host. A imagem padrão possui 131.072 bytes, equivalente a um flash de 1 Mbit.

O arquivo de save é `pokemon_go_world.sav`. Ele não fica dentro do pacote e precisa permanecer gravável.

### 8.6 RTC

`src/siirtc.c`, `include/platform.h` e a plataforma SDL2 substituem a comunicação serial com o RTC por valores obtidos do relógio do sistema. Isso mantém eventos dependentes de horário sem exigir hardware de cartucho.

### 8.7 BIOS, DMA e serviços de baixo nível

- `src/platform/bios.c` implementa operações semelhantes às rotinas BIOS necessárias;
- `src/platform/dma.c` e `include/platform/dma.h` modelam transferências DMA;
- `src/platform/nostd.c` fornece auxiliares de baixo nível para o host;
- `src/platform/stubs.c` contém serviços do GBA ainda sem equivalente ou deliberadamente neutralizados;
- `src/platform/win32.c` é um backend alternativo/legado; o alvo atual usa SDL2.

## 9. Pacote externo de recursos

### 9.1 Por que ele existe

No GBA, gráficos, músicas, mapas e tabelas são vinculados ao espaço da ROM e acessados frequentemente como ponteiros constantes. Para crescer além de 32 MiB no PC, esses dados precisam deixar de ser parte obrigatória do binário e passar a ser identificados por nomes ou hashes.

O pacote atual é composto por:

- carregador: `src/platform/resource_pack.c`;
- API: `include/resource_pack.h`;
- construtor: `tools/pokemon_go_world/pack_resources.py`;
- manifesto raiz: `resources/pc/manifest.json`;
- arquivo final: `pokemon_go_world.pak`.

### 9.2 Formato binário versão 1

Cabeçalho de 48 bytes:

| Campo | Tamanho |
|---|---:|
| magic `PGWPACK\0` | 8 bytes |
| versão | 4 bytes |
| quantidade de entradas | 4 bytes |
| deslocamento do índice | 8 bytes |
| deslocamento dos nomes | 8 bytes |
| deslocamento dos dados | 8 bytes |
| tamanho total declarado | 8 bytes |

Cada entrada do índice possui 40 bytes:

| Campo | Tamanho |
|---|---:|
| hash do nome | 8 bytes |
| deslocamento do blob | 8 bytes |
| tamanho do blob | 8 bytes |
| deslocamento do nome | 8 bytes |
| comprimento do nome | 4 bytes |
| CRC32 | 4 bytes |

Os nomes usam hash FNV-1a de 64 bits. Entradas são ordenadas por hash e procuradas por busca binária. O CRC32 detecta conteúdo corrompido. O carregador rejeita colisões, duplicatas e estruturas incoerentes.

Existem limites defensivos de 1.000.000 de entradas e 4.096 bytes por nome. São proteções de parser, não metas recomendadas.

### 9.3 Carregamento sob demanda

Ao abrir o pacote, o programa mantém em memória apenas o cabeçalho, o índice e a tabela de nomes. O conteúdo pesado é buscado quando necessário:

- `ResourcePack_Get` e `ResourcePack_GetByHash` fornecem acesso persistente gerenciado;
- `ResourcePack_Load` e `ResourcePack_LoadByHash` carregam um blob transitório;
- `ResourcePack_Free` libera o blob transitório.

Essa decisão é essencial para que um pacote grande não precise caber inteiro na memória de um processo de 32 bits.

### 9.4 Estado auditado do pacote

O pacote atual, incluindo o trabalho não commitado de layouts, declara:

- magic: `PGWPACK\0`;
- versão: 1;
- entradas: **11.564**;
- índice em: 48;
- nomes em: 462.608;
- dados em: 958.576;
- tamanho declarado e real: **16.489.008 bytes**.

## 10. Método usado para externalizar uma família

Cada família de recursos é migrada com o mesmo padrão:

1. identificar os símbolos que hoje apontam para dados vinculados;
2. criar um gerador que lê as fontes e produz blobs determinísticos;
3. atribuir a cada blob um nome estável;
4. incluir esses nomes em um manifesto intermediário;
5. incluir o manifesto no `resources/pc/manifest.json`;
6. gerar no build tabelas de hashes ou descritores;
7. substituir a referência direta por uma consulta ao pacote;
8. reconstruir ponteiros/relocações quando o formato contém referências internas;
9. manter fallback seguro para pacote ausente, entrada ausente ou CRC inválido;
10. comparar funcionamento com pacote válido e com falhas intencionais.

O código GBA continua usando seus dados vinculados. As substituições são condicionadas ao alvo portátil/PC.

### 10.1 Por que placeholders continuam no executável

Alguns símbolos precisam existir para satisfazer código compartilhado, tabelas ou o linker. Nesses casos, o alvo PC pode vincular um placeholder pequeno e buscar os dados reais no pacote em tempo de execução. O objetivo é reduzir os objetos sem quebrar a interface de símbolos usada pelo Expansion.

### 10.2 Recursos que contêm ponteiros

Músicas e voicegroups não podem ser copiadas cegamente, porque endereços válidos no objeto/linker não são válidos após carregar um blob em outro endereço. Os geradores registram relocações. O loader então:

- resolve alvos internos relativos ao próprio blob;
- resolve símbolos externos conhecidos;
- aplica addends;
- valida limites antes de escrever os ponteiros reconstruídos.

## 11. Famílias já externalizadas

### 11.1 Gráficos de Pokémon

Gerador: `tools/pokemon_go_world/generate_pokemon_resources.py`

Inventário atual:

| Subfamília | Entradas |
|---|---:|
| imagens | 2.950 |
| paletas | 2.889 |
| ícones | 1.420 |
| pegadas | 1.031 |
| **Total** | **8.290** |

Tamanho-fonte total: **4.053.884 bytes**. O resultado preserva tabelas estáveis por espécie, forma e variante, mas transfere os blobs pesados ao pacote.

### 11.2 Cries

Gerador: `generate_cry_resources.py`

- 1.067 entradas;
- 8.111.114 bytes de áudio-fonte;
- caminho de fallback validado para entrada ausente.

### 11.3 Tilesets

Gerador: `generate_tileset_resources.py`

| Subfamília | Entradas |
|---|---:|
| tiles | 85 |
| paletas | 77 |
| metatiles | 72 |
| atributos | 72 |
| **Total** | **306** |

Tamanho-fonte: **745.700 bytes**.

### 11.4 Animações de tileset

Gerador: `generate_tileset_anim_resources.py`

- 174 entradas;
- 87.200 bytes.

### 11.5 Amostras musicais

Gerador: `generate_music_sample_resources.py`

- 105 amostras regulares;
- 1 grupo de fonemas;
- 106 entradas;
- 642.220 bytes.

### 11.6 Sequências de músicas

Gerador: `generate_song_resources.py`

- 530 músicas;
- 684.080 bytes de dados de pistas;
- 10.991 relocações;
- 872.656 bytes codificados.

Formato `PGWSONG\0`, versão 1:

- cabeçalho de 24 bytes: magic, versão, tamanho dos dados, quantidade de relocações e offset do cabeçalho da música;
- cada relocação possui offset, tipo e alvo;
- o formato de objeto-fonte é COFF i386, máquina `0x014C`, com relocações `DIR32 0x0006`.

### 11.7 Voicegroups

Gerador: `generate_voicegroup_resources.py`

- 195 voicegroups;
- 251.496 bytes de `ToneData` subjacente;
- 2.473 relocações;
- 182 símbolos externos;
- 315.528 bytes codificados.

Formato `PGWVOICE`, versão 1. O cabeçalho possui 24 bytes e cada relocação 24 bytes, incluindo hash do alvo e addend.

Durante o desenvolvimento, a inferência inicial do tamanho de drumsets pelo “próximo símbolo” produziu uma leitura inválida e falha em `MidiKeyToFreq`. A correção passou a analisar as definições-fonte e calcular o intervalo lógico `(primeira_nota + quantidade) * 12`. Há também um fallback de 128 instrumentos CGB square válidos para impedir ponteiros inválidos.

### 11.8 Assets diretos da tela inicial

Doze recursos da apresentação/título também estão no pacote e foram usados como primeiro ensaio de ponta a ponta antes da migração em massa.

### 11.9 Layouts de mapas — trabalho atual ainda não commitado

Gerador: `tools/pokemon_go_world/generate_map_layout_resources.py`

Header gerado/consumido: `include/map_layout_resources.h`

O gerador identificou:

- 442 layouts;
- 884 recursos;
- 653.694 bytes.

O build concluiu e `maps.o` caiu de 955.473 para 305.313 bytes. Entretanto, a validação em tempo de execução do primeiro mapa com o pacote atual foi interrompida e ainda não é conclusiva. O fallback sem layouts também não foi testado. Portanto, esta família é considerada **compilada e estruturalmente verificada, mas não validada em runtime nem commitada**.

## 12. Mapa de arquivos e responsabilidades

### 12.1 Build e configuração

| Arquivo/diretório | Responsabilidade |
|---|---|
| `Makefile` | mantém o alvo GBA e encaminha `make pc` |
| `Makefile_pc` | fontes, flags, geradores, pacote e link do PC |
| `tools/pokemon_go_world/build_pc.ps1` | entrada de build reproduzível no Windows |
| `build/pc-native` | objetos e arquivos intermediários nativos |
| `build/pc-generated` | saídas derivadas dos geradores |

### 12.2 Plataforma

| Arquivo | Responsabilidade |
|---|---|
| `src/platform/sdl2.c` | entrada, janela, eventos, áudio, RTC, save e loop |
| `src/platform/gba_easy_draw.c` | renderização do estado gráfico simulado |
| `src/platform/bios.c` | serviços equivalentes à BIOS |
| `src/platform/dma.c` | emulação funcional de DMA |
| `include/platform/dma.h` | interface e tipos de DMA |
| `src/platform/cgb_audio.c` | canais de áudio CGB |
| `src/platform/nostd.c` | auxiliares portáveis de baixo nível |
| `src/platform/stubs.c` | serviços não suportados/neutralizados |
| `src/platform/win32.c` | backend alternativo, não o alvo SDL2 ativo |
| `include/platform.h` | contrato geral da plataforma |
| `include/platform/framedraw.h` | contrato do desenho de frames |

### 12.3 Recursos externos

| Arquivo/diretório | Responsabilidade |
|---|---|
| `resources/pc/manifest.json` | raiz declarativa dos recursos empacotados |
| `tools/pokemon_go_world/pack_resources.py` | cria o pacote e seu índice |
| `src/platform/resource_pack.c` | valida e lê o pacote |
| `include/resource_pack.h` | API pública do carregador |
| `tools/pokemon_go_world/generate_*_resources.py` | extratores/geradores por família |
| `data/sound_data.s` | dados e símbolos de áudio originais |
| `sound/voice_groups.inc` | inclusão de definições de voicegroups |
| `tools/pokemon_go_world/voicegroups_source.s` | fonte auxiliar usada na extração |

### 12.4 Áudio, save e relógio

| Arquivo | Responsabilidade |
|---|---|
| `src/music_player.c` | sequenciamento musical |
| `src/sound_mixer.c` | mixer PCM |
| `src/m4a.c` | integração do motor M4A/MP2K |
| `src/sound.c` | interface geral de som |
| `src/agb_flash.c` | comportamento da flash |
| `src/agb_flash_1m.c` | flash de 1 Mbit |
| `src/agb_flash_dummy.c` | caminhos auxiliares/fallback |
| `src/save.c` | lógica superior de save |
| `include/gba/flash_internal.h` | contrato interno da flash |
| `src/siirtc.c` | lógica de RTC adaptada |

## 13. Histórico do porte

Todos os checkpoints abaixo são de 2 de agosto de 2026 e pertencem à linha do ramo `feature/pc-port`.

| Horário | Commit | Marco |
|---:|---|---|
| 00:37 | `0440c09b4` | snapshot antes do porte PC |
| 02:01 | `97bbf844e` | primeiro build nativo |
| 03:05 | `fd544a3c9` | boot até o primeiro mapa |
| 03:09 | `a18a0044e` | controles de campo e transição de mapa |
| 03:21 | `b28e73368` | save e RTC |
| 03:39 | `6a3e86771` | mixer MP2K |
| 03:53 | `6fca3da70` | fluxo de batalha |
| 04:09 | `83a9c773a` | pacote externo inicial |
| 04:21 | `1cca4e792` | assets de título externalizados |
| 04:38 | `2320ff951` | imagens de Pokémon |
| 04:50 | `7b06ddb5b` | paletas de Pokémon |
| 05:07 | `597b8c5bd` | ícones e pegadas |
| 05:21 | `8739f618e` | correção do summary |
| 05:35 | `4f5bce5ed` | cries |
| 05:53 | `a9e8271d5` | tilesets |
| 14:03 | `ff08885cc` | animações de tileset |
| 14:15 | `83125885f` | amostras musicais |
| 14:30 | `f193276d3` | pistas de música |
| 14:35 | `a64a9e182` | exclusão de payloads multiboot do PC |
| 14:57 | `d5355f87d` | voicegroups externalizados |

### 13.1 Leitura dos marcos

Os primeiros commits estabeleceram equivalência funcional: compilar, iniciar, desenhar, controlar, salvar, obter horário, tocar som e batalhar. A partir de `83a9c773a`, a prioridade mudou para escalabilidade: criar um pacote robusto e retirar famílias pesadas do executável em incrementos verificáveis.

Essa ordem reduz risco. Externalizar tudo de uma vez tornaria difícil distinguir uma falha de plataforma de uma falha de serialização, relocação ou fallback.

## 14. O que já foi validado

Os testes e capturas existentes demonstram funcionamento dos seguintes fluxos:

- copyright, introdução, título e novo jogo;
- entrada no primeiro mapa, caminhão e Littleroot;
- conversa, controles de campo e transição de mapa;
- salvar, continuar e RTC;
- áudio não silencioso;
- party menu, summary e renomeação;
- batalha selvagem Bulbasaur contra Pikachu;
- HUD, cries, turnos, HP, sleep, Thunder e Discharge;
- pacote válido, ausente e corrompido;
- fallbacks específicos para cries, tilesets, animações, amostras, músicas e voicegroups.

Evidências estão em diretórios como:

- `build/pc-port-cries-final`;
- `build/pc-port-tilesets-valid` e `build/pc-port-tilesets-missing`;
- `build/pc-port-tileset-anims-valid`;
- `build/pc-port-music-samples-valid`;
- `build/pc-port-song-resources-valid` e `build/pc-port-song-resources-missing`;
- `build/pc-port-voicegroups-valid2` e `build/pc-port-voicegroups-missing2`.

Variáveis de ambiente usadas para diagnóstico e automação:

| Variável | Finalidade |
|---|---|
| `POKEMON_GO_WORLD_CAPTURE_DIR` | diretório de capturas/logs |
| `POKEMON_GO_WORLD_RESOURCE_PACK` | caminho alternativo do pacote |
| `POKEMON_GO_WORLD_AUTOPLAY` | roteiro automatizado de entrada |
| `POKEMON_GO_WORLD_TEST_AUDIO` | diagnóstico de áudio |
| `POKEMON_GO_WORLD_TEST_RTC` | diagnóstico de relógio |

Estas evidências não substituem uma suíte automatizada completa, mas registram testes reais dos marcos. Layouts de mapas não estão incluídos nessa afirmação de validação.

## 15. Estado atual do Git

### 15.1 Último checkpoint commitado

- ramo: `feature/pc-port`;
- HEAD: `d5355f87d feat: externalize PC voicegroups`;
- descrição: `expansion/1.16.3-89-gd5355f87d-dirty`.

### 15.2 Alterações locais em andamento

Arquivos rastreados modificados:

- `Makefile_pc`;
- `data/maps.s`;
- `resources/pc/manifest.json`;
- `src/battle_pyramid.c`;
- `src/overworld.c`.

Arquivos novos ainda não rastreados:

- `include/map_layout_resources.h`;
- `tools/pokemon_go_world/generate_map_layout_resources.py`;
- este relatório, até que seja adicionado a um commit.

Essas alterações constituem o marco de layouts de mapas. Elas não devem ser confundidas com o checkpoint de voicegroups já commitado.

## 16. Métricas de tamanho

### 16.1 ROM GBA

| Artefato | Bytes | MiB |
|---|---:|---:|
| `pokemon_go_world.gba` | 33.554.432 | 32,00 |

MiB significa **mebibyte**, isto é, 1.048.576 bytes. Não significa megabit. O “B” maiúsculo indica bytes; “bit” é normalmente abreviado com “b” minúsculo.

### 16.2 Checkpoint commitado de voicegroups

| Artefato | Bytes | Aproximado |
|---|---:|---:|
| executável | 17.209.712 | 16,41 MiB |
| pacote | 15.752.096 | 15,02 MiB |
| entradas no pacote | 10.680 | — |
| `sound_data.o` | 96.025 | 0,09 MiB |

### 16.3 Árvore atual com layouts em desenvolvimento

| Artefato | Bytes | Aproximado |
|---|---:|---:|
| `pokemon_go_world-pc.exe` | 16.589.511 | 15,82 MiB |
| `pokemon_go_world.pak` | 16.489.008 | 15,73 MiB |
| `pokemon_go_world.sav` | 131.072 | 0,125 MiB |
| `SDL2.dll` | 2.043.392 | 1,95 MiB |
| entradas no pacote | 11.564 | — |
| `maps.o` | 305.313 | 0,29 MiB |

O tamanho atual do executável e do pacote não é um limite. Ele é somente o conteúdo que já foi migrado e gerado até este ponto.

### 16.4 Maiores objetos ainda vinculados

| Objeto | Bytes |
|---|---:|
| `event_scripts.o` | 2.674.975 |
| `pokemon.o` | 2.088.435 |
| `graphics.o` | 1.124.192 |
| `event_object_movement.o` | 964.768 |
| `battle_anim_scripts.o` | 656.754 |
| `map_events.o` | 557.639 |
| `fonts.o` | 433.975 |
| `battle_scripts_1.o` | 350.324 |
| `data.o` | 339.594 |
| `item.o` | 320.935 |
| `move.o` | 318.669 |

Essa lista orienta os próximos ganhos, mas tamanho não é o único critério. Scripts e tabelas com muitos ponteiros exigem um formato e uma validação mais cuidadosos do que blobs gráficos simples.

## 17. Limites, riscos e decisões técnicas

### 17.1 O porte ainda usa um processo de 32 bits

Isto facilita compatibilidade com layouts de estruturas, tamanhos de ponteiro e pressupostos da base original. O pacote pode ser maior que 32 MiB, mas não se deve carregar tudo de uma vez. Se no futuro o conteúdo ou caches se aproximarem do espaço virtual disponível, será necessário:

- fortalecer políticas de cache e descarte;
- usar streaming para áudio e assets grandes;
- reduzir blocos residentes;
- ou realizar uma etapa separada de migração limpa para 64 bits.

Migrar imediatamente para 64 bits não é apenas trocar `-m32` por `-m64`: estruturas serializadas, casts de ponteiro, assembly, tabelas e interfaces ABI precisam ser auditados.

### 17.2 Ponteiros e relocações

Uma grande parte da base foi escrita supondo que endereços de ROM são constantes. Dados externalizados não têm endereço fixo entre execuções. Toda família com ponteiros precisa de um formato explícito, validação de limites e resolução de símbolos. Falhas nesse ponto podem compilar normalmente e só aparecer em menus, músicas ou mapas específicos.

### 17.3 Fallbacks

Fallback não deve ocultar silenciosamente corrupção importante. Para conteúdo opcional, ele mantém o jogo utilizável. Para tabelas essenciais, o comportamento preferível pode ser falhar com uma mensagem clara. A política deve ser definida por família e testada tanto com pacote válido quanto com entrada ausente/corrompida.

### 17.4 Determinismo

Geradores devem produzir a mesma saída para a mesma revisão:

- ordenar entradas;
- usar nomes estáveis;
- rejeitar duplicatas;
- registrar versão do formato;
- verificar tamanho e CRC;
- não depender da ordem casual do sistema de arquivos.

### 17.5 Compatibilidade de saves

O save em arquivo reproduz a imagem de flash, mas mudanças futuras em estruturas do jogo ainda podem tornar saves incompatíveis. É recomendável versionar migrações de save antes de distribuir builds de longa duração.

### 17.6 Distribuição

Um pacote de distribuição mínimo precisará do executável, `pokemon_go_world.pak` e `SDL2.dll`. Save e configuração são dados do usuário e não devem ser sobrescritos por atualizações.

## 18. Próximos marcos recomendados

### Marco imediato: concluir layouts de mapas

1. executar o jogo com o pacote atual;
2. chegar ao primeiro mapa e realizar transições;
3. testar interiores e layouts usados por Battle Pyramid;
4. executar com layouts ausentes e confirmar o fallback definido;
5. verificar logs, captures e `git diff --check`;
6. somente então criar o checkpoint Git.

### Externalização seguinte

Após layouts, a ordem mais segura é escolher famílias com fronteiras claras e alto impacto:

1. eventos de mapas;
2. gráficos globais restantes;
3. fontes;
4. animações e scripts de batalha;
5. scripts globais e tabelas complexas.

`event_scripts.o` é o maior objeto, mas não deve ser migrado só por ser o maior. Scripts contêm comandos, offsets e referências cruzadas; precisam primeiro de uma especificação de serialização e relocação.

### Robustez e produto

- testes automatizados de boot, mapa, menus, batalha, save e áudio;
- mensagens claras para pacote ausente ou incompatível;
- cache com orçamento de memória;
- empacotamento de release;
- diretório de dados do usuário apropriado no Windows;
- versionamento de pacote e save;
- avaliação posterior de 64 bits, somente se houver necessidade concreta.

## 19. Procedimento para adicionar novos recursos

Ao acrescentar uma nova família:

1. **Inventariar:** listar símbolos, quantidade, formatos, ponteiros e consumidores.
2. **Medir:** registrar tamanho dos objetos antes da mudança.
3. **Especificar:** documentar nome lógico, versão do blob, endianness, alinhamento e relocações.
4. **Gerar:** criar um script determinístico em `tools/pokemon_go_world`.
5. **Manifestar:** conectar sua saída ao `resources/pc/manifest.json`.
6. **Adaptar:** acrescentar loader/cache condicionado ao alvo PC.
7. **Preservar o GBA:** manter o caminho original fora de `PORTABLE`.
8. **Definir fallback:** decidir conscientemente o comportamento de ausência/corrupção.
9. **Compilar:** construir do zero e observar warnings.
10. **Validar:** testar casos válidos, ausentes, corrompidos e todos os consumidores importantes.
11. **Comparar:** medir executável, pacote e objeto depois da mudança.
12. **Documentar e checkpoint:** atualizar os documentos e criar um commit isolado.

Convenções recomendadas:

- nomes de pacote minúsculos, estáveis e hierárquicos;
- nenhum ponteiro bruto persistido sem uma relocação definida;
- inteiros do formato com endianness explícita;
- overflow e limites verificados antes de alocar ou copiar;
- um número de versão sempre que o layout binário puder evoluir.

## 20. Como reproduzir e inspecionar

### 20.1 Abrir o diretório

```powershell
Set-Location 'C:\Users\Rafael\Documents\Codex\2026-08-02\pokemon-go-world-pc'
```

### 20.2 Conferir revisão e alterações locais

```powershell
git branch --show-current
git describe --tags --always --dirty
git status --short
```

### 20.3 Compilar

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_pc.ps1 -Jobs 4
```

### 20.4 Executar

```powershell
.\pokemon_go_world-pc.exe
```

O executável espera, por padrão, `pokemon_go_world.pak` e `SDL2.dll` no diretório de execução. O save é criado/atualizado separadamente.

### 20.5 Testar um pacote alternativo

```powershell
$env:POKEMON_GO_WORLD_RESOURCE_PACK = 'C:\caminho\teste\pokemon_go_world.pak'
.\pokemon_go_world-pc.exe
Remove-Item Env:POKEMON_GO_WORLD_RESOURCE_PACK
```

### 20.6 Verificar integridade textual antes de um commit

```powershell
git diff --check
git status --short
```

## 21. Critérios para considerar o porte concluído

O porte pode ser chamado de funcionalmente concluído quando:

- inicia e chega ao jogo sem depender da ROM GBA;
- mapas, menus, batalhas, áudio, animações, RTC e save passam por testes repetíveis;
- todas as famílias planejadas podem crescer no pacote sem relinkar grandes blobs no executável;
- pacote ausente, versão incompatível e corrupção geram comportamento controlado;
- atualização não destrói save/configuração;
- um build limpo pode ser reproduzido apenas com dependências e instruções documentadas;
- licenças e atribuições estão completas para a forma escolhida de distribuição.

Isso não exige necessariamente externalizar cada byte. Código e pequenas tabelas estáveis podem permanecer no executável. O critério é remover o limite estrutural de conteúdo e manter um sistema sustentável.

## 22. Resumo do estado

O Pokémon GO World já possui um executável Windows nativo baseado em SDL2. Ele não executa a ROM dentro de um emulador: recompila o motor e substitui dependências do hardware GBA. Vídeo, controles, áudio, RTC, save, mapas, menus e batalhas fundamentais já foram demonstrados.

O pacote externo versão 1 funciona, valida seu índice e carrega recursos sob demanda. Gráficos de Pokémon, cries, tilesets, animações, samples, músicas e voicegroups já foram migrados e testados, com fallbacks específicos. O checkpoint atual commitado é `d5355f87d`.

O trabalho local seguinte externaliza layouts de mapas. Ele compila, gera 884 recursos e reduz `maps.o` substancialmente, mas ainda precisa de validação completa em execução e teste de fallback antes de ser considerado concluído.

A estratégia, portanto, já provou ser viável. O projeto não está mais limitado a uma ROM de 32 MiB no alvo PC; o trabalho restante é ampliar a cobertura do pacote com formatos seguros, testes repetíveis e uma política de memória adequada.
