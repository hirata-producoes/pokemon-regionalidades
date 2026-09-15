# Política de itens e equipamentos entre regiões

Este documento define como uma campanha única trata objetos repetidos, ferramentas
globais e permissões narrativas de Kanto, Johto, Hoenn e Sinnoh. O objetivo é
preservar as cenas reconhecíveis de cada região sem duplicar itens, liberar o
transporte errado ou sobrescrever progresso no save mundial.

A pesquisa comparativa usada como referência descreve o funcionamento nos jogos
oficiais. As regras abaixo são decisões próprias de Pokémon Regionalidades e não
devem ser confundidas com o comportamento histórico de uma geração específica.

## Regra estrutural

Um encontro regional e o objeto visível ao jogador são estados diferentes:

- o marco regional registra que a cena aconteceu naquela campanha;
- a posse global registra ferramentas que só precisam existir uma vez;
- equipamentos colecionáveis registram cada variante e sua origem;
- permissões de viagem registram a rota ou o serviço liberado, mesmo quando dois
  bilhetes possuem o mesmo nome;
- uma recompensa repetida conclui a cena de forma idempotente, sem sobrescrever
  outra recompensa e sem fabricar uma quantidade sem função.

O save nativo deve armazenar esses estados em blocos versionados. Flags e itens
legados continuam como projeções de compatibilidade enquanto os mapas originais
forem migrados.

## TMs e HMs

TMs e HMs são aquisições permanentes e não consumíveis. A interface representa
cada disco como adquirido ou não adquirido, sem exibir uma quantidade. Ensinar o
movimento não remove o disco.

Se uma recompensa regional oferece um TM ou HM já possuído, o marco narrativo
regional ainda é concluído, mas o inventário não recebe outra cópia. A posse do
disco e o acontecimento da história permanecem independentes.

O uso de movimentos de campo continua seguindo D-067: não exige insígnia nem
progresso narrativo, apenas um Pokémon não ovo da equipe que conheça o movimento
e um alvo, terreno ou destino válido.

No código compartilhado, `I_REUSABLE_TMS` fica ativado. Os HMs já usam a mesma
semântica permanente.

## Exp Share

Exp. Share é uma capacidade global ativável, representada por um único item
importante. A primeira obtenção instala o recurso; entregas equivalentes de outras
regiões concluem seus marcos narrativos sem criar cópias equipáveis ou quantidades
ocultas.

Quando ligado, ele segue o modelo moderno já oferecido pelo motor:

- cada Pokémon que participou recebe 100% da experiência prevista pela fórmula;
- cada integrante elegível que não participou recebe 50% da mesma base;
- a parcela dos demais integrantes não reduz a experiência dos participantes;
- um participante nunca recebe também a parcela de 50%;
- ovos, Pokémon no nível máximo, bônus individuais, limites de nível e Double
  Battles continuam obedecendo às regras comuns do motor.

As divisões e o arredondamento permanecem os do cálculo oficial implementado no
núcleo: aritmética inteira e mínimo de um ponto quando existe uma recompensa de
experiência válida. Não existe uma segunda fórmula exclusiva do projeto.

Saves anteriores que guardavam o Exp. Share na mochila ou equipado devem migrar
uma única posse para a área de itens importantes, retirar cópias equipadas e
começar com a distribuição moderna ligada. Depois da migração, desligar o recurso
é uma escolha persistente do jogador.

## Bicicletas

Todas as bicicletas obtidas permanecem pertencendo ao perfil, mas somente uma
fica na mochila de cada vez. As demais são guardadas no PC pessoal como
equipamentos distintos, e não como quantidades empilhadas:

- Bicicleta de Kanto, recebida na campanha de Kanto;
- Bicicleta de Johto, recebida separadamente na campanha de Johto;
- Mach Bike e Acro Bike de Hoenn;
- bicicleta com marchas de Sinnoh.

Kanto e Johto podem compartilhar aparência e comportamento, mas conservam origens
e recompensas regionais distintas. A tela de detalhes poderá mostrar a origem para
diferenciá-las sem inventar uma função artificial.

O jogador troca a bicicleta ativa no PC pessoal. A escolhida vai para a mochila e
a anterior volta ao armazenamento; a operação deve ser atômica para nunca apagar
uma variante se não houver espaço. Usar o item registrado ou a ativação rápida
monta a bicicleta que está na mochila. Capacidades específicas, como manobras da
Acro Bike, velocidade da Mach Bike e marchas de Sinnoh, pertencem à variante, não
à região onde o jogador está.

## Go Goggles e proteção ambiental

Go Goggles continua sendo um equipamento associado a Hoenn. Ele não será
renomeado como se existisse originalmente nas outras regiões.

O sistema ambiental, porém, consulta uma capacidade de proteção, e não o nome de
um item específico. Assim, uma área futura pode:

- apenas ficar mais fácil com proteção adequada;
- exigir proteção por segurança ou visibilidade;
- aceitar Go Goggles ou outro equipamento coerente com o ambiente.

Se uma campanha começar longe de Hoenn e uma área realmente exigir essa proteção,
deverá existir uma obtenção coerente antes ou perto da área, por compra, conquista
ou recompensa. Quem já possuir proteção compatível não recebe uma cópia inútil.
Equipamentos ambientais não têm durabilidade e não concedem bônus de batalha.

Nem todo clima extremo cria uma barreira. A nevasca de Sinnoh, por exemplo, pode
reduzir mobilidade e visibilidade sem exigir um item. A decisão é feita por área e
deve sempre evitar aprisionar o jogador do lado errado de um bloqueio.

## Coin Case

Coin Case será uma capacidade global de carteira. Recebê-lo pela primeira vez
habilita o armazenamento de fichas; encontros equivalentes em outras regiões
concluem seus próprios marcos sem criar carteiras concorrentes.

As fichas formam um saldo global para todos os Game Corners que usem a mesma
moeda. A obtenção de outro Coin Case não zera, copia nem aumenta esse saldo. O
armazenamento nativo reserva um inteiro de 32 bits; o limite visível e os preços
serão definidos pelo balanceamento, sem exigir mudança do formato do save. Se uma
atividade futura usar outra moeda de fato, ela terá nome e contador próprios em
vez de reutilizar silenciosamente Coin Case.

## Itemfinder e RotomDex

Itemfinder e Dowsing Machine deixam de ser objetos concorrentes e passam a ser
módulos do RotomDex. Cada entrega regional:

1. conclui o marco daquela região;
2. instala o módulo se ainda não existir;
3. melhora a capacidade global quando o módulo já existe.

O número de módulos regionais distintos poderá controlar alcance, precisão e
apresentação contínua dos itens escondidos. Os valores exatos serão definidos
quando o protótipo do RotomDex estiver pronto. Repetir a mesma cena ou importar
um save não aumenta o nível duas vezes.

## Bilhetes e permissões de viagem

Cada bilhete informa claramente o navio, a cidade de partida e o destino. O mesmo
nome histórico, inclusive `S.S. Ticket`, só pode ser preservado quando esses
detalhes visíveis eliminarem qualquer ambiguidade. Internamente, a permissão é
registrada por serviço ou rota:

- S.S. Anne em Kanto;
- S.S. Aqua entre Johto e Kanto;
- S.S. Tidal em Hoenn;
- outras rotas e destinos de evento quando forem integrados.

Essa separação é obrigatória: possuir o bilhete de um navio não pode liberar
automaticamente outro navio só porque o texto mostrado é igual. A interface pode
apresentar um único porta-bilhetes com as rotas habilitadas e os três dados de
identificação de cada serviço.

Bilhetes de evento com o mesmo destino podem apontar para a mesma permissão
global. Bilhetes com nomes iguais, mas destinos ou condições diferentes, mantêm
permissões separadas. Nenhum deles usa quantidade.

## Itens já cobertos por políticas anteriores

- o primeiro professor entrega o RotomDex global; professores seguintes instalam
  módulos regionais;
- Running Shoes libera correr uma única vez; sapatos futuros são cosméticos;
- consumíveis comuns continuam usando quantidades e o inventário nativo;
- objetos puramente narrativos permanecem regionais quando sua posse não deve
  produzir uma capacidade global.

## Categorias ainda a catalogar

Esta política resolve os itens pesquisados, mas não encerra todo o inventário das
quatro regiões. Antes de portar os respectivos eventos, ainda será necessário
classificar:

- varas de pesca e outras ferramentas com níveis equivalentes;
- Pokégear, PokéNav, Pokétch, Vs. Seeker e aplicativos que podem virar módulos do
  Rotom Phone;
- passes de transporte diferentes do S.S. Ticket;
- caixas, estojos e moedas de atividades regionais;
- fósseis, chaves e objetos de eventos lendários;
- roupas, acessórios e armazenamento do futuro bloco `GEAR`.

Essas categorias seguem a mesma pergunta: a recompensa representa uma posse
global, uma variante colecionável, uma permissão regional ou apenas a conclusão
de uma cena. Nenhuma delas deve ser consolidada somente porque o nome é parecido.

## Ordem de implementação

1. manter TMs e HMs reutilizáveis no núcleo atual;
2. ativar o Exp. Share moderno do motor e migrar com segurança as posses antigas;
3. criar os registros nativos de equipamento, módulos e permissões de viagem;
4. migrar as recompensas de Hoenn para esses contratos sem mudar sua cronologia;
5. integrar os eventos equivalentes de Kanto somente depois dos testes de
   repetição, importação e reabertura do save.
