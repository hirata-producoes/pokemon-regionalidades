# Histórico do Pokémon Regionalidades

Este arquivo acompanha apenas mudanças específicas do projeto. O `CHANGELOG.md` original continua reservado ao histórico herdado do `pokeemerald-expansion`.

## Em desenvolvimento

- encontros opcionais da rival em Rustboro e na Route 104 protegidos pelo
  marco das encomendas Devon, preservando as duas alternativas originais e
  impedindo ativação antes da hora; eles expiram após uma entrega ou depois do
  início da viagem de Briney;
- rival de Lilycove e Scott no Centro Pokémon de Ever Grande condicionados a
  janelas narrativas explícitas, expirando respectivamente no avanço de Mt.
  Pyre e na entrada da Liga sem se tornarem requisitos da campanha principal;
- início da trajetória de Scott protegido: Petalburg abre após o tutorial do
  Wally e expira com Roxanne; a Escola de Rustboro exige que esse encontro
  tenha acontecido e expira ao receber as encomendas Devon;
- cenas de Scott em Slateport separadas em duas janelas: a conversa do museu
  expira na Route 110 e a saída do Battle Tent expira com Wally; a cena
  acoplada à batalha de Mauville reutiliza o pré-requisito já existente;
- passagem pelo Fiery Path separada do avanço narrativo de Scott: Verdanturf e
  Fallarbor agora respeitam a janela entre Wattson e o roubo do meteorito, sem
  perder o encontro caso a caverna seja explorada antecipadamente;
- encontros de Scott em Lilycove e Mossdeep protegidos por janelas narrativas;
  Mossdeep recebeu um marco opcional persistente na versão 20 para impedir a
  repetição da conversa e do incremento do contador ao recarregar o mapa;
- convite de Scott na SS Tidal e recepção da Battle Frontier condicionados ao
  pós-jogo, ao S.S. Ticket e à ordem correta; entradas técnicas antecipadas não
  entregam o Frontier Pass e não consomem permanentemente a cena pendente;
- contêiner de save nativo `PGRSAVE` criado para o PC, com blocos versionados,
  CRC32, gravação atômica, três recuperações e importação não destrutiva do
  `.sav` Emerald;
- progresso mundial e módulos regionais do RotomDex separados nos blocos
  `WORLD` e `ROTOMDEX`;
- inventário do PC migrado para o bloco `INVENT`, com quantidades de 32 bits e
  limite de gameplay de 99.999 por posição;
- mochila, PC de itens, lojas e seletores revisados para cinco dígitos, incluindo
  correções encontradas nos testes manuais de captura e retirada integral;
- TMs e HMs convertidos em aquisições permanentes sem pilhas duplicadas, e Exp.
  Share migrado para a forma global ativável com distribuição moderna;
- menu de substituição em batalha protegido contra uma ordem de equipe inválida
  que podia repetir o primeiro Pokémon nos seis espaços após um desmaio;
- escolha de região inicial integrada ao fluxo de `New Game`, mantendo somente
  Hoenn disponível enquanto as demais campanhas não forem validadas;
- pré-requisitos narrativos e recompensas idempotentes preparados para a
  progressão multirregional;
- continuidade narrativa de Hoenn protegida até a conquista da Liga, incluindo
  os oito líderes e a ramificação flexível de Brawly, sem transformar HMs ou
  barreiras físicas herdadas em requisitos;
- diálogos dos líderes e de Wallace alinhados à regra de mundo aberto: HMs de
  campo dependem do movimento aprendido, não da posse de uma insígnia;
- sequência de Mossdeep corrigida para exigir Tate e Liza, Centro Espacial e a
  entrega narrativa de Dive antes dos acontecimentos da Seafloor Cavern;
- abertura de Petalburg corrigida com marco para o tutorial de captura do Wally
  e pré-requisito explícito nas primeiras batalhas de Roxanne e Brawly;
- política de versões e inventário de eventos de Hoenn documentados para separar
  baseline de Emerald, variantes, conteúdo secundário e pós-jogo;
- rival de Lilycove e Scott em Ever Grande sincronizados com marcos narrativos,
  sem transformar seus encontros secundários em requisitos da campanha;
- sessões técnicas isoladas criadas para baseline de Hoenn, mobilidade e testes
  de inventário sem alterar os dois perfis pessoais;
- menu externo ampliado com dois perfis, importação, exportação, recuperações,
  controles, vídeo, áudio e aceleração;
- nomes dos perfis tornados personalizáveis e independentes do uso escolhido
  para cada campanha;
- reinício individual de campanha acrescentado com confirmação e preservação
  recuperável da pasta anterior;
- planejamento atualizado convertido em registro Markdown rastreável;
- World Clock configurado em 3× conforme D-023;
- estações alteradas para 30 dias internos, com transição nos dias 29–30 e 1–2;
- avanço offline calculado a partir de uma âncora de RTC persistida sem ampliar os SaveBlocks;
- cálculo sazonal separado em API determinística e testes de fronteira adicionados;
- pausa transitória do World Clock integrada ao menu inicial e aos seus submenus, sem persistir um flag de pausa no save;
- protótipo de interface ambiental adicionado ao popup de área, mostrando estação, dia sazonal e horário;
- previsão ambiental determinística criada em blocos de seis horas, estável pelas próximas 24 horas;
- clima lógico separado de estação e bioma, sem consumir o RNG de gameplay;
- mapa externo de Littleroot conectado ao primeiro perfil de clima dinâmico;
- popup ambiental ampliado para mostrar a condição climática e corrigido para separar o texto do horário;
- identidade pública alterada de Pokémon GO World para Pokémon Regionalidades;
- executáveis, ROM, pacote de recursos, save e configuração receberam nomes públicos próprios;
- leitura compatível dos nomes antigos de recursos;
- cópia não destrutiva de save e configuração legados;
- documentação reorganizada para estudo, referência e portfólio;
- regiões e estados de implementação descritos separadamente;
- guias de compilação para Windows e GBA adicionados;
- roteiro, contribuição, créditos e avisos legais documentados.

## 30 de agosto de 2026 — Primeira publicação

- histórico completo enviado para `hirata-producoes/pokemon-regionalidades`;
- branch principal configurada como `main`;
- commit de referência da publicação inicial: `27251a5d0e`.

## Fase anterior — Porte PC experimental

- integração inicial de SDL2;
- execução nativa no Windows;
- testes de renderização, áudio, RTC, controles, save, mapas e batalhas;
- protótipo de pacote externo de recursos;
- relatório técnico preservado em `docs/pokemon_go_world/RELATORIO_TECNICO_PORTE_PC.md`.
