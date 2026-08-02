# Pokémon Go World — visão e arquitetura

## Visão

Pokémon Go World será uma aventura de mundo aberto baseada em Pokémon Emerald,
com uma única jornada e um único arquivo de save atravessando várias regiões.
O jogador escolhe onde começar, recebe os iniciais daquela região e pode seguir
rotas diferentes conforme clima, estação, equipe e progressão.

O idioma principal será português brasileiro. Nomes oficiais de Pokémon,
golpes, habilidades e itens serão mantidos de forma consistente durante a
localização; a tradução integral dos diálogos será feita por região para que
cada entrega permaneça revisável e jogável.

## Escopo mundial

Regiões planejadas:

1. Hoenn e o primeiro corredor mundial (vertical slice).
2. Kanto, Johto e Ilhas Laranja.
3. Sinnoh e Hisui.
4. Unova e Kalos.
5. Alola, Galar e Paldea.
6. Rotas e arquipélagos originais que conectam os continentes.

As Ilhas Laranja são tratadas como uma região especial. Como não possuem um
trio oficial, o conjunto provisório é Pikachu, Eevee e Lapras.

## Sistemas principais

### Região inicial

O save guarda separadamente a região escolhida e a região atual. Cada região
tem um trio de iniciais próprio. A tabela para todas as regiões planejadas já
existe em `src/starter_choose.c`; a tela de seleção mundial será adicionada ao
fluxo de introdução.

### Dificuldade

Quatro modos serão oferecidos: fácil, normal, difícil e muito difícil. A escolha
fica no save e pode controlar equipes, IA, itens, limites de nível, ganho de
experiência e punição por derrota. Treinadores sem uma variante específica
usam temporariamente a equipe normal, evitando conteúdo quebrado durante o
desenvolvimento incremental.

### Horário

O relógio usa tempo de jogo acelerado, não a hora real do computador. O motor
já diferencia períodos do dia e permite encontros distintos. A interface
mostrará o horário em formato de 24 horas ao entrar em uma área.

Períodos de design:

- manhã;
- dia;
- noite.

O motor pode manter subdivisões internas (amanhecer e entardecer) para efeitos
visuais sem exigir tabelas extras de conteúdo.

### Estações

Primavera, verão, outono e inverno duram inicialmente 28 dias do mundo. A
estação será global, mas cada região terá um perfil climático próprio. Ela
poderá alterar:

- encontros e formas regionais/sazonais;
- crescimento de frutas e recursos;
- acesso a caminhos (gelo, neve, enchentes e marés);
- precisão, potência ou duração de efeitos em batalha;
- festivais, missões e eventos raros.

### Clima dinâmico

Cada área terá clima-base, probabilidades por estação, intensidade e duração.
O sistema aproveitará os climas já existentes: sol, chuva, temporal com raios,
neve, granizo, neblina, tempestade de areia, cinzas vulcânicas, seca e chuva
forte. Eventos lendários poderão substituir temporariamente a previsão.

O clima deverá ser determinístico por save e dia para impedir que recarregar o
jogo altere a previsão imediatamente.

### Mundo aberto e progressão

O balanceamento não usará apenas o número de insígnias. `VAR_PGW_WORLD_LEVEL`
representará faixas de ameaça globais. Áreas terão nível mínimo e máximo, e
líderes poderão escalar equipes dentro de limites definidos. Obstáculos serão
resolvidos por múltiplas opções sempre que possível: habilidade de campo,
montaria, item, estação ou rota alternativa.

## Pokémon e mecânicas

A base já habilita as gerações I–IX, evoluções entre gerações, formas regionais,
golpes e habilidades modernas. “Quase todos” significa manter todas as famílias
principais e decidir separadamente sobre formas apenas de batalha, fusões,
Gigantamax e conteúdo redundante quando o orçamento da ROM for medido.

## Limites técnicos

Uma ROM GBA tradicional possui 32 MiB. A base moderna, antes de adicionar
regiões e tradução, já ocupa grande parte desse espaço. Por isso, o projeto terá
um orçamento de ROM por fase e priorizará:

- reutilização e compressão de tilesets;
- músicas compartilhadas antes de arranjos exclusivos;
- remoção seletiva de formas não usadas;
- mapas compactos com conexões convincentes em vez de cópias 1:1 gigantes;
- testes em emulador e, enquanto possível, em hardware real.

Se o conteúdo final exceder o limite, a decisão entre uma ROM estendida para
emuladores e módulos regionais será tomada somente depois de medir uma build
real.

## Entregas

### Fase 0 — fundação

- branch e identidade do projeto;
- variáveis persistentes de região, estação, dificuldade e nível mundial;
- iniciais definidos por região;
- relógio acelerado, encontros por período e interface de horário;
- documento técnico e orçamento de conteúdo.

### Fase 1 — vertical slice

- introdução em português com escolha de dificuldade e região;
- Hoenn jogável do início até a primeira insígnia;
- uma rota original levando a um pequeno porto mundial;
- ciclo de um dia completo;
- uma mudança de estação acelerada para demonstração;
- chuva, neblina e tempestade afetando mapa, encontros e batalha;
- equipes fácil/normal/difícil/muito difícil para os treinadores do trecho.

### Fases seguintes

Cada região será incorporada como uma entrega jogável, com mapa, progressão,
Pokédex regional, tradução, clima, eventos e testes antes da próxima expansão.

## Estado da fundação

- Base: `rh-hideout/pokeemerald-expansion` 1.16.3, branch `master`.
- Branch do projeto: `feature/pokemon-go-world-foundation`.
- Região padrão temporária: Hoenn.
- Dificuldade padrão: normal.
- Estação padrão: primavera, dia 1.
- Arquivo de saída planejado: `pokemon_go_world.gba`.
- Compilação local: operacional em Ubuntu/WSL2 com GCC ARM 13.2.
- Primeira ROM gerada em 1º de agosto de 2026: `pokemon_go_world.gba`, 32 MiB.
- Cabeçalho verificado: título `PKMN GO WRLD`, código `BGWP`, fabricante `01`.
- Smoke test: inicialização confirmada no `mgba-rom-test` headless.

## Kanto — primeira integração

Pallet Town é o primeiro mapa de Kanto habilitado no build mundial. O mapa usa
o layout original de FireRed, mas possui eventos próprios e mínimos para não
carregar prematuramente a campanha inteira de FireRed. Somente os tilesets
`General_Frlg` e `PalletTown` foram liberados no alvo Emerald.

Durante o desenvolvimento, um marinheiro no laboratório do Prof. Birch leva o
jogador até Pallet Town. Outro marinheiro em Pallet Town faz a viagem de volta.
Esses atalhos serão substituídos pelo porto e pelas rotas mundiais definitivas.

Para preparar e compilar no Windows:

1. Execute `tools/pokemon_go_world/install_wsl2.ps1` em PowerShell como
   administrador e reinicie o computador caso o Windows solicite.
2. Abra Ubuntu uma vez e conclua a criação do usuário e da senha.
3. Na raiz do projeto, execute
   `powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_rom.ps1 -InstallDependencies`.
4. Nas compilações seguintes, execute o mesmo comando sem
   `-InstallDependencies`.
