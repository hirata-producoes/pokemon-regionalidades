# Progressão narrativa e recompensas multirregionais

## Objetivo

Pokémon Regionalidades não pode depender apenas da presença do jogador em um
mapa para decidir qual parte da história executar. Em um mundo aberto, chegar
cedo a uma cidade não significa ter concluído as cenas que preparam seu arco.

A fundação de progresso separa três conceitos:

1. flags e variáveis herdadas continuam executando a campanha original;
2. marcos narrativos próprios registram, por região, o que foi concluído;
3. um registro de recompensas impede que itens e aparelhos únicos sejam
   entregues repetidamente por campanhas diferentes.

Movimentos de campo não fazem parte dos pré-requisitos narrativos. Se um
Pokémon da equipe conhece o movimento, ele pode utilizá-lo sem insígnia e sem
licença de história. A liberdade criada por essa regra é o motivo para cada
cena relevante validar seus próprios marcos e controlar a presença de seus NPCs.

## Estado persistente

O `SaveBlock3` mantém um bloco versionado no final de sua área extensível. Ele
reserva:

- 128 marcos narrativos independentes para cada uma das quatro regiões da V1;
- 64 recompensas únicas globais;
- 64 recompensas regionais por região.

O bloco possui marcador e versão próprios. Saves anteriores são aceitos: na
primeira consulta, os marcos iniciais de Hoenn são reconstruídos a partir das
flags canônicas já gravadas. Os offsets anteriores do `SaveBlock3` não mudam.

## Pré-requisitos

Um evento registrado declara quais marcos precisam existir antes dele. A API
pode apenas consultar os requisitos ou tentar concluir o evento. Os resultados
diferenciam identificador inválido, requisito ausente, evento já concluído e
conclusão nova.

A repetição é segura: concluir novamente um marco retorna “já concluído” sem
duplicar estado. Os primeiros contratos de Hoenn cobrem:

- chegada a Littleroot;
- resgate do Professor Birch;
- vitória contra a rival na Route 103;
- recebimento da Pokédex;
- recebimento dos Running Shoes.

Durante esta etapa, as flags originais continuam sendo a autoridade do roteiro
de Hoenn. Ao serem definidas, elas alimentam os marcos novos somente quando os
pré-requisitos anteriores estão satisfeitos. A migração de saves antigos pode
inferir os passos anteriores quando uma flag posterior canônica já existe.

## Recompensas únicas

Uma recompensa possui identificador estável e escopo:

- **global:** existe uma vez na campanha mundial, como o RotomDex;
- **regional:** pode existir uma vez em cada região, como uma licença ou
  melhoria regional.

Para itens, o registro só é marcado depois que o objeto já existe na mochila ou
foi adicionado com sucesso. Se não houver espaço, a entrega continua pendente.
Isso evita o estado em que o roteiro afirma ter entregado um item que o jogador
não recebeu.

RotomDex, Exp. Share e acesso a bicicleta já possuem identificadores de
recompensa, mas a política de cada encontro regional ainda será catalogada antes
da campanha de Kanto usar esses contratos.

Consumíveis comuns, incluindo as cinco Poké Balls do começo de Hoenn, continuam
usando a entrega normal do jogo. Eles não fazem parte do registro de recompensas
únicas. A compatibilidade multirregional deve se concentrar em dispositivos,
licenças e equipamentos cujo recebimento repetido altera a progressão.

Os Running Shoes atuais também não são um item da mochila: são uma permissão de
movimento. No sistema futuro de aparência, modelos e cores de calçado poderão
ser equipamentos colecionáveis. Uma cópia já possuída poderá ir ao armazenamento
do jogador, enquanto a permissão de correr continuará única e independente do
visual escolhido.

## Integração com scripts

Scripts de mapas podem consultar, concluir e entregar recompensas através das
funções especiais registradas. A convenção é:

| Variável | Conteúdo |
|---|---|
| `0x8004` | região |
| `0x8005` | marco ou recompensa |
| `0x8006` | escopo da recompensa |
| `0x8007` | item |
| `0x8008` | quantidade |
| `VAR_RESULT` | resultado da operação |

Não se deve substituir toda a história de Hoenn de uma vez. Cada arco será
migrado quando houver um teste narrativo reproduzível para o caminho normal,
repetição e exploração fora de ordem.

## Próxima validação

A primeira integração está nos scripts do laboratório de Birch e da entrega
dos sapatos em Littleroot. O laboratório verifica a vitória na Route 103 e o
marco correspondente antes de iniciar a cena. Uma cena rejeitada remove seu
gatilho por quadro para não prender o jogador em tentativas sucessivas.

O encontro com Norman e o tutorial de captura do Wally formam um marco próprio
depois dos sapatos. A primeira batalha contra Roxanne e o ramo flexível de
Brawly dependem dessa apresentação; chegar aos ginásios antecipadamente não
inicia uma batalha sem contexto.

O primeiro arco real migrado liga a vitória contra Roxanne ao roubo das Peças
Devon, à recuperação no Rusturf Tunnel e à devolução ao funcionário em
Rustboro. As três cenas consultam o contrato narrativo antes de executar, de
modo que chegar fora de ordem, repetir uma cena ou expor um NPC por um save
técnico não concede itens nem avança a história indevidamente.

O arco seguinte liga a encomenda recebida do Sr. Stone à entrega da carta ao
Steven, à orientação recebida de Dock e à entrega das Peças Devon ao Capitão
Stern. Steven e Stern também conferem a presença do item esperado antes de
iniciar suas cenas, protegendo saves importados ou técnicos que tenham flags e
inventário incompatíveis.

Depois da entrega ao Capitão Stern, o encontro da rival na Route 110 só pode
começar quando esse marco anterior estiver concluído. A cena registra sua
própria conclusão depois da batalha e não pode ser repetida. Isso elimina a
dependência exclusiva do bloqueio físico original do Team Aqua: em um mapa
aberto, alcançar a Route 110 cedo não deve antecipar essa parte da história.

Em Mauville, a batalha contra Wally depende do encontro concluído na Route 110,
e a primeira batalha contra Wattson depende da vitória contra Wally. O Perfil 2
valida o caminho original sem modificações; as tentativas de alcançar esses
eventos cedo serão feitas mais tarde no Perfil 1, que possui mobilidade técnica.

O arco seguinte liga Wattson ao roubo do meteorito em Meteor Falls, ao confronto
com Maxie no Mt. Chimney e à batalha contra Flannery. Nos dois mapas do conflito,
os integrantes das equipes também ficam ocultos antes do pré-requisito e voltam
a aparecer quando a etapa correspondente se torna válida. Isso começa a trocar
o bloqueio físico do mapa por presença narrativa derivada do progresso.

Depois de Flannery, a cena dos Go-Goggles em Lavaridge é habilitada pelo marco
narrativo, e não apenas pelo valor deixado na variável do mapa. Estados técnicos
fora de ordem são desativados sem criar um ciclo por quadro e voltam a ser
habilitados quando os requisitos forem cumpridos. A primeira batalha contra
Norman depende dessa entrega; seus diálogos anteriores e posteriores permanecem
sob o fluxo original do ginásio.

Depois de Norman, Steven passa a aparecer na Route 118. Concluir esse encontro
libera a presença da Equipe Aqua na Route 119 e dentro do Weather Institute;
antes disso, esses personagens permanecem ocultos mesmo que Surf ou outra rota
permitam chegar cedo. A derrota de Shelly registra a libertação do instituto
antes da entrega de Castform, permitindo que a recompensa continue pendente se
não houver espaço sem reabrir a invasão. O HM Surf não é requisito dessa cadeia.

Nesta etapa, a proteção cobre os integrantes da Equipe Aqua, suas batalhas e o
avanço narrativo. A apresentação civil do instituto antes da invasão ainda será
definida: os cientistas e a criança conservam por enquanto os diálogos originais,
escritos para um mapa que não podia ser alcançado antecipadamente. Isso não abre
a história fora de ordem, mas continua sendo uma pendência de ambientação para o
mundo aberto.

A versão 2 do progresso deixa de deduzir a história a partir da capacidade de
correr. A migração também verifica o estado narrativo de Littleroot para os
sapatos, pois antigos saves técnicos marcavam sua flag de recebimento.

Depois do Instituto do Clima, o encontro da rival na Route 119 passa a depender
da libertação do prédio. A posse antecipada do HM Fly não conta como progresso:
somente o encerramento da cena registra o marco. Esse encontro libera Steven e
o Kecleon da ponte na Route 120; receber o Devon Scope libera o Kecleon que
obstrui o ginásio de Fortree, e afastá-lo libera a primeira batalha com Winona.

Depois de Winona, Archie e a Equipe Aqua passam a ocupar o cume do Mt. Pyre. Se
o jogador chegar antes, os atores hostis e o gatilho do roubo dos orbes ficam
inativos. A conclusão da cena registra um marco próprio antes de abrir o caminho
original para o esconderijo da Equipe Magma. O retorno posterior de Archie e
Maxie ao cume continua sendo controlado pelo estado original do mapa.

O marco do Mt. Pyre também controla a abertura narrativa do Magma Hideout. Os
integrantes da equipe e o Groudon adormecido ficam ocultos se o mapa for
alcançado fora de ordem. O confronto final com Maxie registra o despertar de
Groudon e somente então habilita a entrevista de Stern e o roubo do submarino
em Slateport. Itens técnicos e valores isolados de mapas não substituem esses
marcos.

Depois do roubo, os três andares do Aqua Hideout sincronizam os integrantes da
equipe e o submarino com um único marco narrativo. Se o interior for alcançado
antes da hora, ele não apresenta os integrantes da equipe nem a cena final. O
gatilho de visão de Matt também é desativado nesse estado, evitando tentar
mover um personagem oculto. Sua derrota registra a fuga para a Seafloor Cavern
e mantém a equipe removida quando o mapa é recarregado.

Depois da fuga do submarino, Tate e Liza só iniciam a primeira batalha quando
o Aqua Hideout foi concluído. Rematches continuam usando o fluxo original. A
vitória libera narrativamente a invasão da Equipe Magma: os personagens do
lado de fora e dos dois andares do Centro Espacial são sincronizados com esse
marco, e o gatilho das três batalhas não executa sobre personagens ocultos.
Derrotar Maxie com Steven registra um marco separado.

A entrega de Dive na casa de Steven também possui seu próprio marco. Possuir o
HM antecipadamente dá apenas capacidade técnica e não equivale a ter concluído
essa conversa. Só depois da recompensa narrativa a entrada e os andares da
Seafloor Cavern mostram a Equipe Aqua. Explorar a caverna antecipadamente
continua permitido, mas os personagens e a cena de Archie não aparecem fora de
ordem. No último recinto, o Kyogre adormecido só aparece quando a sequência
pode começar; o despertar registra um marco próprio antes de iniciar a crise
climática em Sootopolis.

A crise de Sootopolis, a consulta a Wallace, a chegada dele a Sky Pillar, o
despertar de Rayquaza e o encerramento da anomalia climática possuem marcos
separados. Wallace fica oculto na Cave of Origin e em Sky Pillar quando esses
locais são explorados fora de ordem. Rayquaza também não aparece nem dispara
sua cena antecipadamente. O acesso à Cave of Origin e a porta externa de Sky
Pillar permanecem livres para que a exploração do mundo não dependa de uma
barreira narrativa; somente os personagens e cenas dependem da sequência.

Depois da crise, a recompensa de Waterfall por Wallace é registrada
separadamente da posse técnica do HM. Quem já possuir o item ainda conclui a
conversa sem receber uma cópia desnecessária. A primeira batalha contra Juan,
o encontro com Wally na Victory Road e a autorização para entrar na Liga usam
marcos sucessivos; a antiga suposição de que verificar apenas uma insígnia era
suficiente foi removida.

Brawly é tratado como uma ramificação flexível de Hoenn: sua primeira batalha
exige apenas o início regional já consolidado, por isso pode acontecer antes ou
depois de partes posteriores da campanha. A vitória, porém, precisa estar
registrada junto à sequência de Wally antes da entrada na Liga. Assim o mundo
aberto preserva liberdade sem permitir concluir Hoenn ignorando uma das oito
insígnias.

Sidney, Phoebe, Glacia, Drake e o Campeão também possuem marcos próprios. Cada
batalha exige a conclusão narrativa da anterior, mas o acesso volta a ser
permitido nas novas tentativas da Liga: concluir um marco não transforma a
campanha pós-jogo em uma sequência de uso único. A entrada no Hall da Fama
registra o título regional sem confundi-lo com futuros títulos de Kanto, Johto,
Sinnoh ou com o Campeonato Mundial.

Os testes nativos em `tools/pokemon_go_world/tests/story_progress_native.c`
executam a implementação real de progresso com adaptadores em memória. Foram
verificados requisitos, permissão técnica de corrida, os dois arcos Devon,
itens obrigatórios, os encontros da Route 110 e de Mauville, repetição e
migração. A versão 19
do registro importa os novos marcos de saves a partir da versão 2 preservando
os bits já existentes das demais regiões. Movimento de NPCs, diálogos e
salvamento em jogo ainda exigem validação manual em cópias dos saves.

As aparições opcionais também podem consultar marcos sem se tornarem parte da
cadeia obrigatória. A rival em Rustboro e na Route 104 exige que as encomendas
Devon já tenham sido recebidas; a alternativa de Rustboro desaparece quando a
cena migra para a rota, e ambas expiram quando uma entrega é concluída ou a
viagem de Briney começa. A rival de Lilycove expira no avanço de Mt. Pyre, e
Scott em Ever Grande expira na entrada da Liga. Assim, uma cena opcional pode
ser ignorada sem permanecer anacronicamente disponível no fim da campanha.

Quando uma sequência opcional contém diálogos que pressupõem um encontro
anterior, a etapa seguinte exige também a evidência canônica desse encontro.
Por isso Scott só aparece na Escola de Rustboro se a cena de Petalburg foi
concluída. Petalburg, por sua vez, abre depois do tutorial do Wally e expira
com Roxanne; a escola expira quando o jogador recebe as encomendas Devon.

Uma cena acoplada à conclusão de outro evento não recebe uma condição paralela
sem necessidade. Scott entra em Mauville dentro da conclusão da batalha do
Wally, que já possui pré-requisito explícito. Em contraste, as duas cenas de
Slateport podem permanecer pendentes no mapa: a primeira expira na Route 110 e
a segunda expira com Wally, evitando encontros atrasados ao retornar à cidade.

Marcos de exploração física não devem consumir etapas narrativas. O Fiery Path
continua registrando que a caverna foi visitada, mas a transferência de Scott
entre os Battle Tents de Verdanturf e Fallarbor só acontece depois de Wattson.
Antes disso, uma chegada antecipada não altera a trajetória do personagem;
depois do roubo do meteorito, as duas aparições antigas são encerradas. Essa
separação é o modelo para locais que poderão ser alcançados fora da ordem no
mundo aberto.

A versão 20 aplica a mesma regra às etapas posteriores de Scott. Lilycove exige
o encontro da Route 119 e expira no roubo do submarino. Mossdeep exige Tate e
Liza porque seu diálogo já pressupõe o aviso ao Centro Espacial, e expira ao
fim da invasão. Como a baseline não possuía uma evidência persistente para a
conversa de Mossdeep, foi acrescentado um marco opcional; ele impede repetição
ao recarregar o mapa sem transformar Scott em requisito da campanha.

O primeiro arco pós-jogo distingue presença física, convite e autorização. A
SS Tidal só executa o convite de Scott depois dos créditos, com o marco de
Campeão e o S.S. Ticket realmente possuído. A recepção da Battle Frontier só
entrega o Frontier Pass depois desse convite. Entradas técnicas antecipadas
podem carregar os mapas, mas não concedem esses avanços; ao cumprir os
requisitos, as cenas pendentes tornam-se disponíveis automaticamente.

1. abrir os dois perfis existentes e confirmar carregamento normal;
2. usar o Perfil 2 para validar naturalmente o caminho original, atualmente a
   partir de Petalburg depois da entrega da carta ao Steven;
3. quando esta sequência de Hoenn estiver completa, usar o Perfil 1 e seus HMs
   antecipados para tentar alcançar cenas fora de ordem;
4. confirmar no Perfil 1 que Stern, a rival da Route 110, Wally e Wattson não
   iniciam antes dos respectivos marcos anteriores;
5. confirmar nos dois perfis que salvar e recarregar não repete cenas;
6. visitar Mossdeep antecipadamente e confirmar que Tate e Liza não iniciam a
   primeira batalha antes do Aqua Hideout;
7. depois do Aqua Hideout, derrotar Tate e Liza, concluir o Centro Espacial e
   receber Dive de Steven na ordem normal;
8. confirmar que a Seafloor Cavern pode ser explorada antes dessa entrega sem
   mostrar a Equipe Aqua, Archie ou Kyogre;
9. depois de receber Dive, concluir a cena de Archie e confirmar o início
   normal da crise climática;
10. visitar Cave of Origin e Sky Pillar antecipadamente pelo Perfil 1: a torre
   deve ser explorável, mas Wallace e Rayquaza não devem aparecer;
11. seguir a sequência normal por Sootopolis, Wallace, Sky Pillar e Rayquaza e
   confirmar que o clima volta ao normal;
12. receber Waterfall de Wallace, derrotar Juan e confirmar que Wally não
   aparece na Victory Road antes dessa vitória;
13. confirmar que Brawly pode ser enfrentado em momento flexível, mas que a
   Liga exige tanto sua vitória quanto a conclusão do encontro com Wally;
14. concluir a Elite Four, o Campeão e o Hall da Fama, depois confirmar
   que uma nova tentativa da Liga continua disponível;
15. só então aplicar o padrão à primeira sequência de Kanto.
