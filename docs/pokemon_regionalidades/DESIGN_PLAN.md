# Planejamento de design e rastreabilidade

Este documento transforma a planilha de planejamento em um contrato versionado junto do código. A planilha continua sendo o espaço de elaboração; este arquivo registra o que o projeto assumiu como direção e qual parte já chegou ao jogo.

## Fonte desta revisão

- arquivo analisado: `Pokémon ideias planejamentos.xlsx`;
- revisão importada em: 30 de agosto de 2026;
- estrutura encontrada: 9 abas e 9 tabelas;
- decisões consolidadas: 50;
- estado das decisões: 35 decididas e 15 em discussão ou parcialmente decididas;
- regiões jogáveis previstas para a V1: Kanto, Johto, Hoenn e Sinnoh.

O arquivo binário não é necessário para compilar. As decisões que alteram o jogo devem aparecer neste documento, no código e nos testes correspondentes. Uma ideia antiga presente em material interno não se torna requisito atual automaticamente.

## Regra de implementação

Cada decisão passa por quatro estados de engenharia:

1. **Planejada**: registrada, mas sem contrato técnico estável.
2. **Contrato criado**: constantes, interfaces e casos de teste foram definidos.
3. **Integrada**: afeta uma sequência jogável real.
4. **Validada**: passou por build, testes automatizados aplicáveis e teste manual documentado.

O status de design (`DECIDIDO`, `PARCIALMENTE DECIDIDO` ou `EM DISCUSSÃO`) não é igual ao status de implementação. Uma regra pode estar decidida e ainda não ter sido programada.

## Primeiro sistema: tempo e estações

As decisões D-023, D-029 e D-052 formam a base ambiental:

- o World Clock avança 3 segundos internos para cada segundo real;
- cada estação possui 30 dias internos, equivalentes a 10 dias reais no ritmo 3×;
- a ordem global é Primavera, Verão, Outono e Inverno;
- a transição ocupa os dias 29–30 da estação anterior e 1–2 da nova;
- os dias 3–28 representam a estação estabelecida;
- estação, clima e bioma são conceitos diferentes;
- batalha mantém o relógio em andamento;
- menus marcados como pausáveis não acumulam tempo;
- saltos de sono ou viagem já são expressos em tempo interno e não recebem 3× novamente;
- o tempo fechado deve ser calculado a partir do tempo real transcorrido;
- mapas descarregados preservam o estado lógico do ambiente.

### Estado no código

| Parte | Estado | Referência |
|---|---|---|
| ritmo 3× durante o jogo | Integrada e compilada no PC | `TIME_REGIONALIDADES` e `PGW_WORLD_CLOCK_REALTIME_MULTIPLIER` |
| calendário de 30 dias | Integrada e compilada no PC | `PGW_DAYS_PER_SEASON` |
| fases 29–30 / 1–2 | Contrato criado e testado | `Pgw_GetSeasonPhaseForDay` |
| rotação Primavera→Verão→Outono→Inverno | Contrato criado e testado | `Pgw_CalculateSeasonAfterDays` |
| avanço enquanto o jogo está fechado | Integrada e compilada no PC | âncora real de 32 bits nas variáveis `VAR_PGW_REAL_TIME_ANCHOR_*` |
| pausa seletiva em menus | Integrada e compilada no PC; teste manual pendente | pausa transitória em `FakeRtc_SetMenuPaused`, acionada pelo start menu |
| estado ambiental de mapas descarregados | Planejada | exige registro lógico por área/grupo de mapas |
| apresentação ambiental | Protótipo integrado e compilado no PC | popup de área mostra estação/dia à esquerda e horário à direita; HUD/Rotom final ainda não foi congelado |

A âncora offline ocupa duas variáveis que estavam livres no save, sem aumentar `SaveBlock1`, `SaveBlock2` ou `SaveBlock3`. Saves anteriores começam com âncora zero: no primeiro carregamento eles apenas registram o horário atual, evitando aplicar retroativamente um intervalo desconhecido. Se o relógio do computador ou cartucho voltar no tempo, o jogo atualiza a âncora sem reduzir o World Clock.

A pausa de menu não usa `OW_FLAG_PAUSE_TIME`, porque esse flag pertence ao save e é destinado a scripts. O estado do menu é transitório e existe somente durante a execução. Ao voltar ao mundo, a âncora de tempo real é sincronizada para impedir que os minutos pausados sejam reaplicados posteriormente como avanço offline.

O popup de área da geração 5 foi escolhido como primeiro protótipo de apresentação porque já mostrava o horário e existe nos dois alvos. A faixa secundária agora apresenta `ESTAÇÃO DD` à esquerda e o horário em 24 horas à direita. Essa solução serve para validar legibilidade e utilidade durante a vertical slice; ela não define o HUD permanente nem substitui o futuro protótipo do Rotom Phone. Clima e fase de transição ainda não aparecem nessa faixa.

## Registro resumido das decisões

### Mundo, mapas, viagem e ecologia

- D-010: exploração física e serviços de viagem, com acesso inicial controlado a hubs;
- D-023: World Clock 3× e calendário sazonal de 30 dias;
- D-027: top-down em grid 16×16, áreas maiores e chunks invisíveis;
- D-028: travessia clássica ampliada, Field Abilities, bicicleta, Surf e Dive;
- D-029: ambiente continua evoluindo fora da tela;
- D-036: recursos regeneráveis, crafting em instalações e camp contextual;
- D-037: equipamentos ambientais funcionais, sem durabilidade nem bônus de batalha;
- D-039: população, atividade e migração como camadas distintas;
- D-040: encontros influenciados por microhabitat, condições e método;
- D-041: outbreaks, ninhos, roamers e sistemas especiais preservados ou modernizados;
- D-049: expansão espacial inicialmente orientada por cerca de 2× por eixo;
- D-051: geografia contínua entre as quatro regiões da V1;
- D-052: transição sazonal por grupos coerentes de mapas;
- D-053: migração sazonal controlada por calendário;
- D-054: cálculo contextual de encounters com base oficial e modificadores ambientais;
- D-055: travessia contínua e fast travel apenas por pontos descobertos.

### Pokémon, pesquisa e treinamento

- D-009: Nursery e Training Center são instalações diferentes;
- D-011: Pokédex regional por região e National Dex global;
- D-012: Pokédex e Pokécology separadas, com conhecimento por evidência;
- D-030: Field Abilities ficam fora dos quatro golpes de batalha;
- D-038: dados observados e histórico de indivíduos na Pokédex;
- D-042: IV, EV, Nature, Ability, moves e evolução modernizados sem grind irreversível;
- D-043: amizade, follower e traços individuais sem punição por inatividade;
- D-044: breeding e incubação por tempo, inclusive offline;
- D-045: origem, histórico, títulos e memórias relevantes de cada Pokémon;
- D-050: Ditto ajudante separado da equipe de seis;
- D-059: obediência por insígnias, vínculo e origem do Pokémon;
- D-060: captura clássica contextualizada e encontros em grupo quando coerentes.

### Progressão, história, batalha e conclusão

- D-024: especializações paralelas, sem classes exclusivas;
- D-025: Poké Balls artesanais e industriais coexistem;
- D-026: relacionamento restrito a NPCs e serviços importantes;
- D-031: oito estágios de cada líder conforme insígnias da própria região;
- D-032: League regional e World Championship como desafios distintos;
- D-033 e D-056: histórias regionais reconhecíveis em qualquer ordem e arco global paralelo;
- D-034 e D-057: sem reset ou scaling global de nível ao mudar de região;
- D-035 e D-065: economia global acessível, mochila ampla e proteção contra grind dominante;
- D-046: mesmo protagonista oficial nas quatro regiões, com customização inicial simples;
- D-047: Rotom Phone como interface de mapa, pesquisa, missões e avisos;
- D-048 e D-064: 100% regional e 100% global/Platina finitos e verificáveis;
- D-061: turnos tradicionais, mais Double Battles e IA por perfis sem leitura do turno;
- D-062: Terastal entra; outras transformações seguem decisões específicas;
- D-063: facilities oficiais preservadas com regras regionais;
- D-070: Mega Evolution global com introdução narrativa regional;
- D-071: Team Rocket inter-regional e Project M ligado a Mega e Mewtwo;
- D-072: Forma Reavivada ligada a Ho-Oh, Celebi e Rainbow Wing;
- D-073: uma transformação especial principal por lado em cada batalha.

## Fontes e política de pesquisa

O planejamento prioriza fontes primárias auditáveis para comportamento factual: `pret/pokeemerald`, `pret/pokefirered`, `pret/pokecrystal`, `pret/pokeheartgold`, `pret/pokeplatinum` e `pret/pokediamond`. Sites oficiais Pokémon são a referência pública oficial. PokéAPI, Bulbapedia, Serebii, guias editoriais e referências cartográficas servem para descoberta e confirmação cruzada.

Quando uma fonte secundária divergir materialmente de código, dado de jogo ou material oficial, a fonte primária prevalece. Opiniões editoriais devem permanecer separadas de fatos observáveis.

Consulte também [Créditos e referências](CREDITS_AND_REFERENCES.md) e [Roteiro](ROADMAP.md).
