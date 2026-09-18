# Auditoria de cenas e eventos de Hoenn

Este inventário impede que “o script existe” seja confundido com “o evento está
protegido e validado para mundo aberto”. A referência estrutural é Emerald.

## Campanha principal

| Sequência | Estado estrutural | Validação manual |
|---|---|---|
| Littleroot, Birch, rival da Route 103, RotomDex/Pokédex e sapatos | integrada | parcial |
| encontro com Norman e tutorial de captura do Wally | integrada na versão 19 | pendente |
| Roxanne, roubo, recuperação e devolução das Peças Devon | integrada | pendente no fluxo completo |
| encomendas Devon, Steven, Dock, Stern e museu | integrada | parcial |
| rival da Route 110, Wally em Mauville e Wattson | integrada | pendente no fluxo completo |
| Meteor Falls, Mt. Chimney, Flannery e Go-Goggles | integrada | pendente |
| Norman, Steven na Route 118, Weather Institute e rival da Route 119 | integrada | pendente |
| Devon Scope, acesso ao ginásio e Winona | integrada | pendente |
| Mt. Pyre, Magma Hideout, submarino e Aqua Hideout | integrada | pendente |
| Tate e Liza, Centro Espacial e entrega narrativa de Dive | integrada | pendente |
| Seafloor Cavern, crise de Sootopolis, Wallace, Sky Pillar e Rayquaza | integrada | pendente |
| Waterfall, Juan, Wally na Victory Road, Liga e Campeão | integrada | pendente |
| Brawly como ramo flexível exigido antes da Liga | integrada | pendente |

“Integrada” indica que existe marco e proteção de pré-requisito. Não significa
que todas as animações, posições de NPC, derrotas, recargas e rotas alternativas
já foram exercitadas manualmente.

## Cenas secundárias que precisam de auditoria própria

Estes eventos existem na baseline herdada, mas ainda não receberam uma revisão
completa para chegada antecipada, repetição e persistência:

- recompensas e estados internos de Scott nas instalações da Battle Frontier;
- Exp. Share, bicicletas, Coin Case, Itemfinder e outras recompensas utilitárias;
- Rusturf Tunnel depois da reunião de Wanda e o namorado;
- New Mauville e a missão de Wattson;
- Abandoned Ship, Scanner e troca da recompensa;
- Mirage Tower, escolha de fóssil, Desert Underpass e ressurreição;
- concursos, Pokéblocks, fã-clube, entrevistas, TV e Match Call;
- Trick House, Safari Zone, Day Care, Secret Bases e Shoal Cave;
- Regirock, Regice, Registeel e o enigma da Sealed Chamber;
- Southern Island e eventos originalmente dependentes de tickets;
- Trainer Hill e demais instalações opcionais.

Os encontros opcionais da rival em Rustboro e na Route 104 já foram protegidos.
As duas alternativas originais continuam disponíveis depois que o presidente
da Devon entrega as encomendas, mas expiram quando uma das entregas é concluída
ou quando a viagem de barco começa. O encontro de Lilycove só aparece depois
da cena da Route 119 e expira após a conversa ou quando o roubo dos orbes no
Mt. Pyre avança a campanha.

Scott em Ever Grande também deixou de consultar apenas a antiga insígnia de
Fortree; ele aparece depois de Juan e é ocultado de forma persistente depois da
conversa, ou expira quando o jogador entra na Liga. Esses encontros continuam
secundários e não se tornam requisitos para avançar.

O início da trajetória de Scott também possui janela definida. Petalburg abre
depois do tutorial de captura do Wally e expira com a vitória sobre Roxanne. A
Escola de Rustboro só mostra Scott se o encontro de Petalburg realmente
aconteceu, aceita seus diálogos anterior e posterior à Stone Badge e expira
quando as encomendas Devon e o PokéNav são recebidos.

Em Slateport, a conversa depois do museu expira caso a batalha da rival na
Route 110 já tenha acontecido. A segunda cena, na saída do Battle Tent, continua
disponível durante o caminho até Mauville e expira com a batalha do Wally. A
aparição de Scott em Mauville faz parte da conclusão dessa própria batalha e,
portanto, herda o pré-requisito já validado em vez de duplicar outro bloqueio.

Os Battle Tents de Verdanturf e Fallarbor também possuem uma janela narrativa
explícita. Depois de Wattson, Scott permanece em Verdanturf até o jogador passar
pelo Fiery Path; essa passagem então o transfere para Fallarbor. Visitar a
caverna antes de Wattson apenas registra o local conhecido e não consome a
mudança narrativa. As duas aparições expiram quando o roubo do meteorito é
presenciado, evitando que Scott reapareça atrasado em qualquer um dos prédios.

No motel de Lilycove, Scott aparece depois da cena da Route 119 e deixa de estar
disponível após a conversa ou o roubo do submarino. Em Mossdeep, sua fala sobre
o aviso ao Centro Espacial abre depois da vitória sobre Tate e Liza e expira
quando a invasão termina. A versão 20 registra a conversa de Mossdeep em um
marco opcional próprio, impedindo que recarregar o mapa repita a cena e aumente
novamente o contador histórico de encontros.

Na SS Tidal, o convite de Scott agora exige a conclusão dos créditos, o marco
de Campeão e o S.S. Ticket realmente presente na mochila. Uma chegada técnica
antecipada ao corredor apenas oculta a cena; ela volta a ficar disponível quando
os requisitos forem atendidos. A cerimônia inicial da Battle Frontier e a
entrega do Frontier Pass exigem que o convite no navio tenha acontecido. Isso
protege a cronologia sem transformar a exploração antecipada do mapa em um
avanço narrativo.

Na versão 22, essa sequência deixou de depender da flag genérica de jogo
terminado. O recebimento do S.S. Ticket, o encontro com Scott no navio e a
primeira recepção da Battle Frontier possuem marcos regionais próprios. Os dois
portos só mostram e operam a S.S. Tidal quando o bilhete foi recebido na
campanha e continua presente na mochila. A viagem Slateport–Lilycove fica
disponível com o bilhete; o destino Battle Frontier só aparece depois do
encontro válido com Scott. Flags ou itens isolados de um save técnico não
fabricam essa cronologia.

Na versão 23, Scott também deixa de aparecer em sua casa se a recepção ainda
não foi concluída. A primeira conversa e a entrega inicial de Battle Points
possuem um marco próprio; visitar novamente a casa não repete a recompensa. As
Lansat e Starf Berries e os escudos de sequências permanecem independentes,
pois já conferem todos os símbolos ou recordes necessários e só registram suas
flags depois de uma entrega bem-sucedida.

## Pós-jogo

Ainda precisam ser tratados como um arco separado:

- o retorno a Littleroot e a atualização de pesquisa após o Hall da Fama já
  possuem pré-requisito e marco próprios na versão 21; a cena atualiza o
  arquivo mundial do RotomDex sem ativar a antiga National Dex plana e sem
  entregar antecipadamente um inicial de Johto;
- SS Tidal, convite de Scott e primeira recepção da Battle Frontier já possuem
  uma cadeia persistente; os destinos de eventos especiais continuam separados;
- desafios, símbolos e Frontier Brains das instalações da Battle Frontier —
  pausados intencionalmente; a baseline herdada continua utilizável e a
  auditoria de exploração fora de ordem será retomada antes do fechamento do
  pós-jogo de Hoenn;
- batalha contra Steven em Meteor Falls integrada na versão 24; Steven só
  aparece depois do título de Hoenn, a vitória possui marco próprio e derrotas
  não consomem o encontro;
- encontros lendários e eventos especiais compatíveis com o mundo persistente;
- rematches, chamadas e estados que o Emerald reinicializa depois dos créditos.

## Método de fechamento

Para cada linha, verificar:

1. caminho normal em save narrativo;
2. chegada antecipada com o Perfil 1;
3. recusa ou adiamento sem perder a missão;
4. derrota em batalha e retorno ao último Centro Pokémon;
5. salvamento antes e depois da cena;
6. repetição, rematch ou pós-jogo quando aplicável;
7. recompensa já possuída e mochila cheia;
8. migração de um save anterior ao marco novo.
