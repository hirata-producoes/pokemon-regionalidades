# Arquitetura

## Ideia central

O código de gameplay foi escrito para o GBA. Em vez de reescrever o jogo inteiro, o porte mantém esse código e fornece equivalentes para os serviços do console.

O PC é o alvo principal do produto ampliado. O GBA permanece como referência de
regressão para o núcleo compartilhado e como build compatível da base de Hoenn;
ele não obriga menus, armazenamento, recursos ou mapas próprios do PC a caberem
nas limitações físicas do console.

```text
mapas, scripts, batalhas e menus
                 |
          código compartilhado
           /             \
      alvo GBA          alvo PC
   hardware real     camada SDL2
```

## Componentes

| Componente | Responsabilidade |
|---|---|
| `src/platform/sdl2.c` | janela, eventos, áudio, RTC, save e ciclo de execução |
| `src/platform/gba_easy_draw.c` | interpreta o estado gráfico e compõe frames |
| `src/platform/bios.c` | substitui rotinas de BIOS necessárias |
| `src/platform/dma.c` | modela transferências DMA |
| `src/platform/cgb_audio.c` | reproduz canais de áudio no estilo CGB |
| `src/platform/resource_pack.c` | abre, valida e lê o pacote externo |
| `src/pokemon_regionalidades_clock.c` | cálculos puros de relógio, calendário, seed e previsão climática |
| `src/pokemon_regionalidades_progress.c` | marcos narrativos regionais, pré-requisitos e recompensas idempotentes |
| `src/pokemon_go_world.c` | estado mundial persistido e consultas ambientais de gameplay |
| `Makefile_pc` | gera recursos e vincula o executável Windows |

## Por que ainda é 32 bits

O processo PC usa x86 de 32 bits porque partes da base armazenam ponteiros em estruturas e scripts com layout de 32 bits. Isso não é o limite de 32 MiB do GBA. O pacote possui offsets de 64 bits e é lido sob demanda.

Uma futura migração para 64 bits exigirá auditar tamanhos de estruturas, casts, formatos serializados e interfaces de assembly. Não é apenas trocar uma opção do compilador.

## Pacote externo

`pokemon_regionalidades.pak` guarda recursos grandes fora do executável. O índice usa nomes, FNV-1a de 64 bits e CRC32. Apenas índice e nomes entram na memória ao abrir; os blobs são carregados quando necessários.

Músicas e voicegroups contêm referências internas. Seus geradores registram relocações para que os ponteiros sejam reconstruídos com segurança no endereço onde cada blob foi carregado.

## Conversão sem refazer o jogo

Emerald e o Expansion já fornecem grande parte da lógica e do conteúdo de Hoenn. O trabalho do porte não é reconstruir cada diálogo ou mapa para PC. Ele remove, por famílias, as dependências que ainda presumem hardware ou memória de ROM do GBA.

A arquitetura de evolução fica dividida em cinco camadas:

| Camada | Conteúdo | Regra |
|---|---|---|
| núcleo compartilhado | movimento, scripts, batalha, menus e regras de Pokémon | deve continuar atendendo PC e GBA |
| conteúdo regional | mapas, eventos, treinadores, encontros e progresso | cada região usa o mesmo núcleo |
| adaptação de plataforma | vídeo, entrada, áudio, RTC, arquivos e janela | implementações específicas para PC ou GBA |
| pacote externo | gráficos, mapas, músicas e outros dados volumosos | cresce sem depender do limite de ROM |
| extensões mundiais | região, bioma, clima, ecologia e progressão global | entram gradualmente sobre contratos estáveis |

Uma correção feita no carregador compartilhado deve resolver toda uma família de conteúdo, não apenas a tela usada para descobri-la. A validação cotidiana usa cenas representativas e um save de exploração; eventos de história são verificados somente a partir de um save narrativo autêntico ou do fluxo real da campanha.

Os pontos reservados para sistemas futuros devem ter significado conhecido. Não serão criadas grandes estruturas vazias apenas para antecipar possibilidades: um campo novo entra quando existe uma regra que o produz, uma parte do jogo que o consome e uma forma de testar sua compatibilidade.

## Movimentos de campo no mundo aberto

Insígnias e eventos narrativos não autorizam movimentos de campo. Cut, Flash,
Rock Smash, Strength, Surf, Fly, Dive, Waterfall, Defog e Rock Climb ficam
disponíveis desde o começo quando um Pokémon não ovo da equipe conhece o golpe.
As verificações de terreno, alvo, mapa e destino continuam obrigatórias.

Receber um HM, TM ou outra forma de ensinar o golpe pode continuar fazendo parte
de uma campanha regional, mas isso não cria uma licença separada para usar o
movimento. Assim, exploração e história permanecem independentes: o jogador
pode atravessar o mundo livremente, enquanto os personagens e acontecimentos
são controlados pelos marcos narrativos próprios.

## Áreas contínuas no porte PC

O mapa dividido do jogo original continua sendo uma fonte de conteúdo, mas não define a experiência final do porte PC. A arquitetura planejada mantém uma janela de áreas ativas ao redor do jogador:

1. a área atual permanece ativa;
2. áreas vizinhas são preparadas antes de entrarem no campo de visão;
3. o personagem atravessa a borda andando, sem mudar artificialmente de posição;
4. áreas distantes são descarregadas depois de uma margem de segurança;
5. relógio, clima, ecologia e eventos persistentes continuam existindo como estado mundial, independentemente do que está carregado.

Essa camada será implementada e medida primeiro em um conjunto pequeno de mapas externos. O alvo GBA preserva o modelo compatível com o console; o PC pode usar memória dinâmica, leitura antecipada do pacote e chunks maiores. Consulte [Planejamento da geografia mundial](WORLD_GEOGRAPHY_PLAN.md).

## Nomes legados

`PGW_*`, `tools/pokemon_go_world` e algumas variáveis de ambiente continuam presentes. Eles formam um namespace interno estável. A interface pública já usa Pokémon Regionalidades; a migração interna será incremental para facilitar revisão e regressão.

Para detalhes de formatos, métricas e histórico, consulte o [relatório técnico](../pokemon_go_world/RELATORIO_TECNICO_PORTE_PC.md).

O fluxo ambiental e seus limites de protótipo estão documentados em [Sistema ambiental](ENVIRONMENT_SYSTEM.md).

O fluxo do save de exploração e a divisão entre testes técnicos, visuais e narrativos estão em [Estratégia de desenvolvimento e testes](TESTING_STRATEGY.md).

Perfis pessoais, rotação de recuperações, save rápido e Soft Reset pertencem à
camada de produto do PC e estão especificados em [Perfis, saves e recuperação no PC](SAVE_PROFILES.md).

O save principal do PC já usa um contêiner extensível. A imagem de flash Emerald
permanece inteira no bloco `LEGACY`, enquanto `WORLD`, `ROTOMDEX` e `INVENT`
guardam estado nativo versionado sem depender do limite de 128 KiB. O formato,
os tipos usados e a migração segura estão definidos em
[Arquitetura de save nativo para PC](PC_SAVE_ARCHITECTURE.md).

A separação entre observação mundial, catálogos regionais e pesquisa liberada está em [Arquitetura do RotomDex mundial](WORLD_DEX_ARCHITECTURE.md).

Os contratos que separam eventos regionais e impedem recompensas duplicadas
estão em [Progressão narrativa e recompensas multirregionais](STORY_PROGRESS_ARCHITECTURE.md).

TM e HM permanentes, Exp. Share, bicicletas, proteção ambiental, Coin Case,
Itemfinder e bilhetes seguem a [Política de itens e equipamentos entre regiões](CROSS_REGION_ITEM_POLICY.md).
