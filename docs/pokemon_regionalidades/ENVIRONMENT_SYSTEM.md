# Sistema ambiental: relógio, estações e clima

Este documento descreve o primeiro sistema de gameplay criado diretamente a partir da planilha de planejamento. A implementação é compartilhada pelos alvos GBA e PC; apenas a forma de obter o relógio real muda entre as plataformas.

## Origem das regras

As regras atuais foram extraídas de `Pokémon ideias planejamentos.xlsx`:

- D-023, decidido: World Clock 3×, quatro estações globais e 30 dias internos por estação;
- D-029, decidido: o estado ambiental continua evoluindo com mapas descarregados e não reinicia por Fly ou streaming;
- D-052, parcialmente decidido: transição sazonal nos dias 29–30 e 1–2, com grupos exatos de mapas ainda abertos;
- I-031, adotado: as próximas 24 horas internas devem permanecer definidas e ser apresentadas de maneira natural.

Estação, clima e bioma são domínios diferentes. A estação influencia tendências; o clima representa uma condição lógica temporária; o bioma decide como essa condição pode ser traduzida visualmente. Por isso o modelo não transforma automaticamente todo inverno em neve.

## Modelo implementado

O estado lógico possui seis condições iniciais:

| Condição | Significado lógico | Efeito visual inicial em Littleroot |
|---|---|---|
| `CLEAR` | céu aberto | ensolarado |
| `CLOUDY` | cobertura de nuvens | nuvens |
| `RAIN` | precipitação comum | chuva |
| `STORM` | precipitação intensa e instável | tempestade |
| `FOG` | visibilidade reduzida | neblina horizontal |
| `WIND` | vento perceptível | nuvens, enquanto não existe efeito próprio de vento |

As condições são calculadas em blocos de seis horas internas. Quatro blocos formam a previsão-base de 24 horas. O cálculo usa:

- seed ambiental persistido no save;
- estação e dia sazonal;
- região atual;
- identificador da área;
- bloco horário.

O cálculo é determinístico e não consome o gerador aleatório usado por batalhas, encontros ou scripts. Consultar a previsão, sair de uma área e retornar dentro do mesmo bloco produz o mesmo resultado. Quando o dia muda, o seed avança por uma sequência determinística; assim, a previsão feita antes da meia-noite coincide com a condição observada depois da virada.

## Evolução fora da tela

Não é necessário manter uma estrutura pesada para cada mapa. O estado de uma área descarregada pode ser reconstruído a partir dos mesmos dados persistidos. O avanço offline já move o World Clock; ao carregar uma área, o jogo calcula a condição correspondente ao novo horário. Isso atende ao núcleo de D-029 sem aumentar os `SaveBlock` nem simular cada mapa continuamente.

Essa abordagem também impede um reroll artificial por viagem: Fly, carregamento e streaming consultam o mesmo estado lógico em vez de sortear outra condição.

## Vertical slice de Littleroot

Somente o mapa externo de Littleroot usa o perfil ambiental neste checkpoint. Casas, laboratório, mapas submersos e climas roteirizados continuam usando suas regras originais.

Ao entrar em Littleroot, o estado lógico é traduzido para `WEATHER_DYNAMIC`. Enquanto o jogador permanece no mapa, a verificação temporal atualiza o efeito quando começa um novo bloco climático. O popup de área mostra estação, dia, condição e horário em campos separados.

O perfil é opt-in no código para não forçar a regeneração dos 884 mapas durante este protótipo e para proteger eventos climáticos da campanha de Hoenn. Depois que os perfis de bioma e os grupos sazonais forem validados, a associação deve migrar para dados gerados por mapa ou Location.

## Valores de protótipo

Os pesos de `CLEAR`, `CLOUDY`, `RAIN`, `STORM`, `FOG` e `WIND` variam por estação, mas ainda são dados de ajuste. Eles servem para tornar o teste jogável e não congelam a fórmula ecológica de D-054, que permanece parcialmente decidida.

Também continuam abertos:

- perfis por bioma, altitude, costa e microhabitat;
- neve, areia, cinzas e outros efeitos condicionados ao ambiente;
- nome e quantidade final dos grupos de transição sazonal;
- interface completa de previsão no Rotom Phone;
- influência sobre encontros, atividade e exploração;
- transições visuais de tilesets entre estações.

## Compatibilidade de save

Nenhum campo novo foi acrescentado aos `SaveBlock`. O sistema reutiliza `VAR_PGW_WEATHER_SEED`, criada anteriormente para o estado mundial. Saves da versão anterior continuam válidos. Se o seed for zero, o cálculo usa o valor seguro `1` até a próxima inicialização ou evolução do estado.

## Evidência de verificação

O checkpoint inclui testes para:

- estabilidade dentro de um bloco de seis horas;
- equivalência da previsão antes e depois da meia-noite;
- limites válidos das condições;
- nomes estáveis para a interface.

Em 1º de setembro de 2026, o teste foi compilado no alvo nativo de testes e o porte PC foi recompilado integralmente. Um smoke test isolado manteve o executável ativo por 12 segundos e confirmou inicialização sem encerramento inesperado. A validação manual do efeito visual em Littleroot, da mudança ao vivo entre blocos e da legibilidade do popup ainda é obrigatória antes de considerar o sistema validado.
