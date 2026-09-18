# Roteiro do projeto

Este roteiro organiza a evolução técnica e de conteúdo. Ele não promete datas: cada marco só é considerado concluído quando pode ser compilado, testado e documentado.

## Princípios

- manter uma versão executável durante o desenvolvimento;
- implementar uma região por etapas jogáveis, não por quantidade de arquivos;
- compartilhar regras de jogo entre PC e GBA sempre que isso for viável;
- usar o GBA como referência de regressão do núcleo compartilhado, sem limitar recursos próprios do produto para PC;
- retirar do executável os recursos que podem viver no pacote externo;
- registrar decisões para que iniciantes entendam por que elas foram tomadas;
- preservar autoria, licenças e referências técnicas.
- preservar referências e saves narrativos autênticos antes de ampliar mapas ou ativar sistemas futuros.

## Marco 1 — Fundação do projeto

Estado: concluído como base inicial.

- base efetiva na linha `pokeemerald-expansion 1.16.3`, embora o planejamento inicial tenha citado a 1.13;
- histórico publicado no GitHub;
- alvo GBA preservado;
- documentação técnica inicial;
- identidade Pokémon Regionalidades definida.

## Marco 2 — Porte nativo para PC e baseline de Hoenn

Estado: em validação progressiva da campanha original.

- janela e renderização via SDL2;
- entrada por teclado e controle;
- áudio, RTC e save nativos;
- executável Windows;
- primeiros testes de mapas, menus e batalhas;
- migração de nomes antigos de save e configuração.
- [x] restaurar chão, paredes e porta do caminhão;
- [x] validar a saída para Littleroot e a entrada na casa inicial;
- [x] automatizar a introdução normal até o relógio, a transmissão e a saída da casa;
- [x] validar os indicadores de seleção dos menus no porte PC;
- [x] validar o carregamento isolado da Route 101 e suas conexões externas;
- [x] validar o resgate, a escolha do inicial, a batalha, o laboratório e o retorno a Littleroot pelo fluxo narrativo real;
- [x] validar uma sequência original desde `New Game` até Oldale;
- [x] criar um save isolado de exploração com Fly e HMs, sem sintetizar cenas narrativas;
- [x] isolar executável, pacote, configuração e save de exploração, com manifesto dos artefatos usados;
- [x] criar uma sessão narrativa persistente de Hoenn sem alteração artificial de progresso;
- [x] validar no save de exploração Fly, Cut, save, cura, captura, fuga, batalhas e o ciclo box–equipe do armazenamento;
- [x] separar o uso de movimentos de campo das insígnias e da progressão narrativa;
- [ ] validar individualmente Cut, Flash, Rock Smash, Strength, Surf, Fly,
  Dive, Waterfall, Defog e Rock Climb apenas com o movimento conhecido;
- [ ] validar a transferência de itens quando um marco real exigir essa função;
- [ ] percorrer marcos de Hoenn até os créditos.

Antes de iniciar testes pessoais prolongados da campanha:

- [x] validar o salvamento e o `Continue` da primeira vertical slice narrativa de Hoenn;
- [x] implementar escrita atômica e rotação técnica de três recuperações no PC;
- [x] separar no backend os caminhos dos perfis 1 e 2 e a configuração global;
- [x] validar importação por cópia, confirmação de hash e recusa de sobrescrita;
- [x] listar gerações internas e restaurar recuperações preservando o ativo anterior;
- [x] construir a primeira interface gráfica dos perfis sobre o backend validado;
- [x] validar visualmente a interface e oferecer criação de atalho sem terminal;
- [x] permitir selecionar e abrir os dois perfis pela interface;
- [x] permitir nomes personalizados para os dois perfis;
- [ ] mostrar nome do personagem, tempo jogado e última versão usada em cada perfil;
- [x] permitir reiniciar somente o perfil escolhido, com confirmação, removendo
  também as gerações da campanha anterior e preservando uma cópia recuperável;
- [x] manter tecnicamente os três últimos saves confirmados de cada perfil;
- [x] migrar os saves escolhidos sem sobrescrever suas origens;
- [x] manter campanha pessoal, exploração técnica e marcos narrativos em espaços separados;
- [x] exportar saves com verificação e backup ao substituir uma exportação anterior;
- [x] oferecer seleção e restauração das recuperações pela interface gráfica;
- [ ] validar manualmente uma restauração pela interface usando um perfil real.

## Marco 3 — Pontos de extensão e vertical slice ambiental

Estado: em andamento.

Fundação ambiental iniciada a partir das decisões D-023, D-029 e D-052:

- [x] ritmo 3× durante a execução;
- [x] calendário de 30 dias e fronteiras sazonais;
- [x] contrato das fases de transição;
- [x] avanço offline com âncora persistida no save;
- [x] pausa seletiva no menu inicial e seus submenus;
- [x] protótipo de estação, dia e horário no popup de área;
- [x] previsão climática determinística em blocos de seis horas;
- [x] mapa externo de Littleroot ligado ao clima dinâmico como protótipo;
- [ ] corrigir o recorte do efeito de nuvens no renderizador nativo;
- [ ] separar nome da área e informações ambientais no popup;
- [ ] apresentar estação e clima primeiro em português do Brasil;
- [ ] ampliar as fontes para os caracteres necessários ao português do Brasil, incluindo `Ã` e `ã`;
- [ ] validar visualmente Littleroot em diferentes horários, estações e climas.

Uma *vertical slice* é uma pequena parte do jogo funcionando de ponta a ponta. O objetivo é validar:

- início de uma nova partida;
- exploração de uma sequência curta de mapas;
- diálogos, eventos e transições;
- captura e batalha;
- centro Pokémon, loja e save;
- retorno ao jogo depois de fechar o programa.

Esses sistemas permanecem como contratos ou protótipos até a baseline original estar estável. O detalhamento das decisões importadas da planilha está em [DESIGN_PLAN.md](DESIGN_PLAN.md).

## Marco 4 — Estrutura multirregional

Estado: em andamento; contrato central das quatro regiões criado, com ativação gradual.

- [x] criar um registro único para Kanto, Johto, Hoenn e Sinnoh, distinguindo região planejada, dados importados e campanha jogável;
- definir como regiões são desbloqueadas;
- [x] criar a primeira tela de seleção depois de `New Game`, antes da introdução regional, bloqueando campanhas ainda não validadas;
- permitir que builds de desenvolvimento iniciem diretamente em regiões experimentais;
- [x] encaminhar cada campanha jogável para uma introdução e um ponto inicial registrados, com retorno seguro para Hoenn se uma entrada incompleta for acionada em desenvolvimento;
- padronizar identificadores de mapas e pontos de viagem;
- [x] separar progresso global e progresso regional;
- [x] criar armazenamento versionado para marcos narrativos por região e recompensas globais ou regionais;
- [x] importar com segurança os primeiros marcos de Hoenn a partir de saves anteriores;
- [x] espelhar o progresso multirregional real no bloco nativo `WORLD`, unindo-o
  ao legado durante a migração sem apagar avanços;
- [x] tornar `WORLD` a fonte oficial no PC antes de ampliar a quantidade de eventos
  além da capacidade temporária do legado;
- [x] disponibilizar pré-requisitos e entregas idempotentes para scripts de mapas;
- migrar gradualmente os arcos de Hoenn para pré-requisitos explícitos, com testes de repetição e exploração fora de ordem;
- [x] definir um RotomDex global, entregue pelo primeiro professor, com módulos regionais cumulativos;
- [x] persistir os módulos regionais no bloco nativo versionado `ROTOMDEX`, com
  migração e espelho de compatibilidade;
- [x] criar o esquema inicial de inventário nativo esparso e validar sua
  gravação sem alterar ainda as quantidades usadas pelo gameplay;
- [x] sincronizar em tempo real as mudanças do inventário legado com o bloco
  nativo antes de inverter sua autoridade;
- [x] promover `INVENT` a autoridade persistente no PC com uma projeção legada
  estrita e sem truncamento;
- [x] desvincular a largura das quantidades de gameplay do layout fixo do save
  Emerald, sem alterar seus offsets;
- [x] catalogar a política de TM e HM permanentes, Exp. Share, bicicletas,
  proteção ambiental, Coin Case, Itemfinder e bilhetes;
- [ ] implementar os contratos nativos e o cálculo validado definidos na
  política de itens antes de ativar Kanto;
- testar Pokédex, equipes, flags e variáveis entre regiões;
- integrar Kanto como segunda região jogável validada.

A auditoria detalhada dos desafios, símbolos e Frontier Brains foi pausada
intencionalmente em 13 de setembro de 2026. A implementação herdada de Emerald
permanece disponível depois da recepção, e esse acabamento não é pré-requisito
para o save mundial nem para preparar Kanto. O trabalho deverá ser retomado
antes de considerar o pós-jogo de Hoenn integralmente validado.

Antes de ativar uma segunda região jogável, o PC concluirá a primeira fase do
[save nativo extensível](PC_SAVE_ARCHITECTURE.md). Isso impede que progresso,
RotomDex, inventário e atualizações futuras sejam construídos sobre o limite de
128 KiB da flash do GBA.

Somente Hoenn está marcada como campanha jogável. Kanto possui dados de mapas importados,
mas permanece indisponível na seleção até ter entrada, scripts e continuidade validados.
Johto e Sinnoh permanecem planejadas. Essa barreira impede que um protótipo ou conjunto
parcial de mapas seja confundido com a base real do jogo.

O encaixe espacial das regiões e ilhas será prototipado conforme o [Planejamento da geografia mundial](WORLD_GEOGRAPHY_PLAN.md), sem confundir uma hipótese cartográfica com uma conexão implementada.

## Marco 5 — Pacote externo de recursos

Estado: protótipo existente, integração progressiva.

- versionamento do manifesto;
- validação de integridade;
- [x] verificação integral do pacote contra o manifesto, incluindo checksum de cada recurso;
- [x] auditoria estrutural de grupos, layouts, conexões e warps dos mapas atuais;
- leitura de imagens, áudio e dados externos;
- mensagens claras quando um recurso estiver ausente;
- política de atualização e compatibilidade do pacote.

## Marco 6 — Campanhas regionais e fontes de demake

Estado: planejado em longo prazo.

Johto e Sinnoh terão como referências de implementação os demakes abertos catalogados em [Créditos e referências](CREDITS_AND_REFERENCES.md), sempre comparados às reconstruções oficiais de código e dados. As regiões entram uma por vez; “dados presentes no repositório” não será tratado como sinônimo de “região jogável”. Consulte [REGIONS.md](REGIONS.md).

## Marco 7 — Distribuição para PC

Estado: planejado.

- build reproduzível;
- pacote portátil para Windows;
- testes em instalação limpa;
- [x] primeira interface externa integrada à seleção de perfis;
- [x] controles de teclado configuráveis com restauração dos padrões;
- [x] ações principais do controle XInput configuráveis;
- [x] multiplicador de aceleração momentânea configurável entre 2× e 10×;
- [x] primeira barra externa com pausa, reinício de sessão, retorno aos perfis,
  saída, configurações e ajuda;
- [x] recarregar controles e aceleração durante a execução;
- [x] validar manualmente a permanência na tela de título, a alteração de teclas e
  velocidade durante a sessão e o retorno ao seletor de perfis;
- [x] reter os cinco registros de execução anteriores por perfil;
- [x] menu externo do produto para tela cheia, modo redimensionável ou fixo,
  escala predefinida, escala inteira, VSync e moldura;
- [x] volumes separados para música, efeitos e volume geral;
- [x] remapeamento inicial de teclado e controle, com restauração dos padrões;
- [x] modo de janela com tamanhos fixos e modo redimensionável;
- [x] configuração de escala, tela cheia e preservação da proporção lógica;
- [x] prévia temporária das configurações, com aplicação sem fechar e cancelamento reversível;
- [ ] acrescentar modo de aceleração alternável e verificar sua integração com o World Clock e o áudio;
- idioma, acessibilidade e gestão das configurações locais;
- arquitetura de localização e primeiro segundo idioma completo;
- exportação de save para arquivo portátil;
- importação com validação de versão, confirmação e backup automático do save anterior;
- testes de transporte do save entre duas instalações limpas;
- logs de diagnóstico;
- atualização de save entre versões;
- [x] contêiner de save nativo, versionado e dividido em blocos independentes;
- [x] importação do `.sav` Emerald como bloco legado sem sobrescrever a origem;
- [x] primeiro esquema `WORLD` validado pelo jogo e pelas ferramentas de perfis;
- [x] inventário nativo com quantidades de 32 bits e limite de gameplay de 99.999;
- [x] menu de dois perfis com save ativo e três recuperações por perfil;
- save e carregamento rápidos somente em estados seguros do motor;
- Soft Reset separado de gravação e carregamento;
- licenças e créditos incluídos no pacote.

A infraestrutura de perfis e recuperação será antecipada para depois da primeira
vertical slice estável de Hoenn, pois ela protege os testes pessoais durante o
restante do desenvolvimento. O acabamento visual dessa interface continua neste
marco. Consulte [Perfis, saves e recuperação no PC](SAVE_PROFILES.md).

## Definição de pronto

Uma funcionalidade é considerada pronta quando:

1. compila no alvo afetado;
2. tem um teste manual ou automatizado descrito;
3. não quebra saves suportados sem aviso e migração;
4. possui documentação proporcional à complexidade;
5. registra a origem de código ou recurso reutilizado.
