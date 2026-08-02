# Orçamento de memória e política de conteúdo

## Medição de referência

A ROM jogável de 1º de agosto de 2026 usa 26.650.720 bytes (25,42 MiB) dos
32 MiB tradicionais do GBA. Restam 6,58 MiB físicos, ou 5,08 MiB para conteúdo
se for preservada uma reserva de segurança de 1,5 MiB.

| Área | Uso | Livre | Observação |
| --- | ---: | ---: | --- |
| ROM | 25,42 MiB | 6,58 MiB | Mapas, áudio, gráficos, textos e código |
| EWRAM | 221,28 KiB | 34,72 KiB | Estado dinâmico do jogo |
| IWRAM | 27,71 KiB | 4,29 KiB | Margem crítica para pilha e rotinas rápidas |

O relatório deve ser executado depois de cada build completa:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\memory_report.ps1
```

Use `-Enforce` para falhar quando ROM, EWRAM ou IWRAM atravessarem as reservas
de segurança.

## Principais consumidores da ROM atual

| Grupo | Tamanho aproximado |
| --- | ---: |
| Áudio (`data/sound_data.o`) | 9,89 MiB |
| Pokémon: espécies, sprites e formas (`src/pokemon.o`) | 5,66 MiB |
| Scripts de eventos | 0,94 MiB |
| Gráficos gerais | 0,89 MiB |
| Tilesets | 0,71 MiB |
| Mapas | 0,66 MiB |

Portanto, eliminar pequenos mapas não resolve o orçamento. O projeto deve
controlar primeiro músicas, gritos, formas redundantes e gráficos globais.

## Primeiro pacote aplicado

- Megaevoluções continuam disponíveis, mas usam uma variação de tom do grito
  normal. Os 94 gritos exclusivos removidos da próxima build ocupavam cerca de
  1,18 MiB.
- Seguidores e encontros visíveis continuam desativados. Como nenhum mapa usa
  `OBJ_EVENT_GFX_SPECIES`, a próxima build não inclui sprites de mapa para todas
  as espécies e formas. Economia esperada: aproximadamente 1,1 MiB.
- Os fonemas do Bardo compartilham uma amostra, economizando cerca de 109 KiB.
- A apresentação promocional da base Expansion foi desativada.
- Union Room, Mystery Gift, e-Reader, recordes de link, Pokémon Jump e Record
  Mixing não reservam mais dados no save. Foram recuperados 3.654 bytes de save.
- Unidades métricas e vírgula decimal passam a ser o padrão da localização.

A economia estimada é de 2,4 MiB de ROM. O número definitivo só deve ser
registrado após uma compilação limpa.

## O que permanece útil

- história principal de Hoenn e os sistemas necessários aos eventos;
- Pokédex, PC, criação, frutas, encontros e evolução;
- golpes, habilidades, tipos, formas regionais e mecânicas modernas;
- quatro dificuldades, nível mundial, clima, horário e estações;
- Match Call/rematches, por enquanto, porque combinam com um mundo aberto;
- Fly, barcos e outros meios de viagem rápida;
- ferramentas de depuração durante o desenvolvimento.

## Candidatos para remoção completa

| Sistema | Código mínimo medido | Decisão recomendada |
| --- | ---: | --- |
| Link, wireless, Union Room, trocas e minijogos | 256 KiB | Remover do jogo final single-player |
| Battle Frontier, Tents e torres | 303 KiB | Remover ou redesenhar como conteúdo mundial |
| TV e Bardo | 208 KiB | Remover; não são essenciais à campanha mundial |
| Concursos e Pokéblocks | 149 KiB | Decisão de design antes de traduzir |
| PokéNav e Match Call | 74 KiB | Manter rematches; reavaliar o restante |
| Secret Bases | 11 KiB | Baixa prioridade, pois a economia é pequena |

Esses números não incluem todos os mapas, gráficos e scripts agregados desses
sistemas. A remoção definitiva deve ser feita em pacotes pequenos, sempre com
build limpa e teste de save. Não basta ocultar uma entrada de menu: referências,
scripts e recursos precisam ser retirados juntos.

## Política para regiões, tilesets e músicas

- Kanto e Johto compartilham tilesets primários de cidades e rotas temperadas.
- Johto e Sinnoh compartilham conjuntos de montanha, neve, caverna e floresta.
- Interiores comuns (Centro Pokémon, lojas, casas, portos e estações) são únicos
  e reutilizados por todas as regiões.
- Locais marcantes recebem somente um tileset secundário próprio.
- Estações usam trocas de paleta e metatiles alternativos; não quatro cópias
  completas de cada mapa.
- Cada região recebe temas de identidade, batalha e locais importantes, mas
  cidades menores e rotas compartilham famílias musicais.
- Rotas mundiais originais reutilizam instrumentos e músicas existentes antes
  de ganhar composições novas.

## Localização para português brasileiro

O inventário atual contém aproximadamente 202 mil palavras em strings de mapa,
interface e sistemas. Traduzir tudo de uma vez impediria revisão e testes. A
ordem de trabalho será:

1. interface e mensagens compartilhadas;
2. introdução e trecho jogável de Hoenn;
3. Kanto conforme seus mapas forem integrados;
4. Johto;
5. Sinnoh;
6. conteúdo opcional que tiver sobrevivido à otimização.

Controles como `\n`, `\l`, `\p`, variáveis entre chaves e limites das caixas de
texto devem ser preservados. O alfabeto atual ainda não possui glifos próprios
para `ã`, `õ`, `Ã` e `Õ`; essa fonte precisa ser ampliada antes da tradução em
português correto. A localização usará texto conciso para evitar crescimento
desnecessário da ROM.
