# Planejamento da geografia mundial

## Estado deste documento

Este documento registra hipóteses de composição para Kanto, Johto, Hoenn, Sinnoh e Ilhas Sevii. Ele detalha as decisões D-049, D-051 e D-055 da planilha, que continuam parcialmente decididas.

## Prévia de exploração de Kanto no PC

O porte PC inclui agora os layouts de FireRed e os mapas de Kanto/Ilhas Sevii já presentes no repositório para inspeção de terreno, gráficos, passagens e colisões. Nos 416 mapas de Kanto ainda não integrados à campanha, a prévia conserva warps e conexões, mas deixa personagens, gatilhos de coordenada, interações de cenário e scripts de entrada inativos. Isso não os declara jogáveis nem conclui suas histórias. Pallet e seus interiores já habilitados continuam com seus scripts próprios, ainda em revisão narrativa. Centros Pokémon e Poké Marts serão integrados junto da campanha, sem serviços provisórios a serem substituídos depois.

A prévia não cria ligações inéditas entre regiões nem garante que todos os mapas sejam alcançáveis por caminhada a partir de Pallet; transportes, obstáculos e eventos necessários à travessia normal ainda precisam ser adaptados. Warps com destino dinâmico e as entradas do elevador de Celadon ficam inativos na prévia, pois dependem de scripts que ainda não foram integrados. Os 132 conjuntos de encontros selvagens de FireRed foram incluídos no porte PC, sem substituir os de Hoenn; falta validar de forma mais ampla espécies, níveis e modalidades, embora uma batalha terrestre em Kanto já tenha ocorrido corretamente. No início de Pallet, Oak e as Poké Bolas usam identificadores persistentes sem colisão com Hoenn, e a primeira batalha usa três rivais próprios de nível 5; professor, seleção dos iniciais e batalha foram validados manualmente. A abertura de Kanto passa a usar a apresentação original de Oak, incluindo o nome do rival salvo em variáveis reservadas sem ampliar o bloco legado; ainda aguarda teste manual. Os demais treinadores e eventos de FireRed não podem ser ligados diretamente, pois seus identificadores se sobrepõem aos de Hoenn e precisam de uma integração multirregional própria. A correção visual do quarto inicial permanece pendente.

As ligações descritas aqui não autorizam alterar mapas imediatamente. Cada proposta precisa ser confrontada com as dimensões dos layouts, a geografia reconhecível, os eventos da campanha, as fontes catalogadas e um protótipo navegável.

## Regras preservadas

- Kanto e Johto mantêm a conexão oficial como parte da geografia mundial.
- Sinnoh também pode possuir vários pontos de conexão com Johto e Kanto.
- Hoenn e Sinnoh não possuem ligação direta na composição inicial: Kanto e Johto ocupam a faixa intermediária.
- Cada par de regiões adjacentes pode possuir mais de uma travessia coerente, em vez de depender de um único corredor obrigatório.
- As quatro regiões da V1 podem ser atravessadas fisicamente quando o jogador possuir as capacidades necessárias.
- Fast travel complementa a exploração e não substitui as ligações terrestres e marítimas.
- Mapas externos partem de uma referência aproximada de 2× na largura e 2× na altura, mas a escala final depende de testes.
- Casas, árvores, personagens e demais elementos mantêm a escala visual original.
- Novos espaços precisam receber relevo, encontros, eventos ou pontos de interesse coerentes. A expansão não deve apenas acrescentar chão vazio.
- Uma borda técnica de mapa não reinicia clima, estado ambiental ou passagem do tempo.

## Hipóteses de conexão

### Kanto e Sinnoh

A Route 24 pode substituir ou redesenhar sua barreira montanhosa ao norte para receber um rio, uma várzea ou uma faixa de vegetação úmida associada à Route 212 de Sinnoh.

A Route 25 pode perder parte das cercas e ganhar uma abertura para o litoral. A primeira hipótese conecta essa costa à Route 213, que continua conduzindo a Pastoria City. Uma ligação direta com Pastoria permanece como alternativa somente se as dimensões e a organização dos mapas demonstrarem maior coerência.

Composição provisória:

```text
Route 212
    |
rio ou área úmida
    |
Route 24 -- Route 25 -- costa intermediária -- Route 213 -- Pastoria City
    |
Cerulean City
```

Um ou mais mapas intermediários podem representar estuário, manguezal, praia ou mar raso. Eles evitam uma mudança abrupta entre duas Routes originalmente criadas para mundos separados.

### Sinnoh e Johto

Sinnoh também pode se ligar a Johto em mais de um ponto. As extremidades exatas permanecem abertas até que os mapas das duas regiões sejam inventariados na mesma escala. As alternativas devem comparar passagens montanhosas, rios, florestas, cavernas e serviços de transporte já sugeridos pelos jogos de referência.

Essas conexões não substituem a ligação oficial entre Kanto e Johto. Elas criam rotas alternativas de exploração e ajudam a impedir que uma região inteira dependa de uma única entrada.

### Kanto, Ilhas Sevii e Hoenn

As Ilhas Sevii podem ser aproximadas até formar um arquipélago compacto, mantendo faixas navegáveis de mar entre as ilhas. Praias, portos, cavernas e orientação costeira devem continuar reconhecíveis.

Referência cartográfica para o primeiro encaixe Hoenn–Kanto, com base nas imagens
fornecidas em 20 de setembro de 2026: Cinnabar Island fica imediatamente ao
norte da Route 124. A borda sul de Cinnabar toca a borda norte da Route 124;
as duas bordas oeste ficam alinhadas. As linhas coloridas das imagens são guias
de alinhamento, não elementos gráficos a adicionar. Os layouts importados têm
respectivamente 24×20 e 80×80 blocos: o encaixe direto cobre somente os 24
blocos mais à esquerda do limite norte da Route 124. O restante dessa borda
precisa conservar um limite ou receber outra solução geográfica coerente.

Para preparar a travessia, revisar somente as pequenas rochas marcadas que
funcionavam como barreira técnica, preservando os obstáculos e eventos que têm
função própria. Em 20 de setembro de 2026, as
10 rochas marcadas no print de Cinnabar e as 11 marcadas no print da Route 124
foram trocadas por mar nos layouts, sempre substituindo seus quatro blocos e
mantendo todas as demais rochas. O protótipo PC agora conecta a borda sul de
Cinnabar aos primeiros 24 blocos da borda norte da Route 124, nos dois sentidos,
com a mesma posição horizontal. A faixa exibida antes da troca de mapa usa os
blocos do mapa atual para não interpretar o mar com o conjunto gráfico da outra
região. Em 21 de setembro de 2026, a travessia foi validada manualmente nos dois
sentidos, incluindo uma batalha selvagem em Kanto e o retorno a Hoenn sem erro
gráfico. Essa faixa ainda é temporária. Como ela prolonga o mar do mapa atual,
pode ocultar pedras próximas da borda e tornar mais evidente a diferença entre
as águas das duas regiões. Não uniformizar toda a água apenas para mascarar o
problema: substituir a repetição quando Route 124 e Route 125 forem ligadas de
forma completa a Cinnabar Island, Route 20 e Route 19, usando mapas reais ou
trechos intermediários coerentes em toda a largura necessária.

Essa validação não libera a
história de Kanto. As conexões devem funcionar
nos dois sentidos, manter colisão, encontros, clima, música e posição estáveis
e não liberar por engano conteúdo narrativo de Kanto. A posição no mapa não
ativa automaticamente scripts, personagens ou recompensas: cada cena de Kanto
deverá depender de pré-requisitos explícitos de história, tempo e local,
aplicados de forma idempotente e testados ao entrar antecipadamente ou voltar
depois. A Route 124 continua a ligar Lilycove a Mossdeep; Route 21 ao norte
e Route 20 a leste de Cinnabar
também precisam permanecer coerentes.

Este encaixe é só o primeiro segmento para testar a travessia. A composição
pretendida deverá aproximar toda a faixa marítima das Routes 124 e 125 de Hoenn
de Cinnabar Island e das Routes 20 e 19 de Kanto, sem supor que as dimensões
atuais permitam unir todas as bordas diretamente. Depois, Johto deverá manter
sua ligação reconhecível com Kanto; sua ligação com Hoenn será escolhida após
comparar as posições e decidir se são necessários mapas novos ou ajustes locais.
Nenhuma dessas ligações adicionais foi ativada nesta etapa.

A primeira travessia expôs outro limite do porte: mapas de Hoenn e de FireRed
usam conjuntos gráficos primários diferentes. A transição de câmera agora
recarrega gráficos e paletas primários quando eles mudam, redesenha a vista
inteira e não reaproveita blocos salvos da outra região. A primeira validação
manual nos dois sentidos passou. Um teste posterior com várias travessias
seguidas encontrou corrupção gráfica e perda de controle sem encerramento por
exceção. A causa estrutural encontrada foi o uso, em uma transição sem tela de
carregamento, de buffers temporários que só seriam liberados por um carregamento
completo. O carregamento do tileset principal passou a usar buffers que se
liberam após a cópia. Falta repetir várias travessias seguidas para validar a
correção.

### Serviços marítimos entre regiões

Como alternativa de viagem entre pontos já descobertos, estudar paradas a
partir de Slateport City e Lilycove City para Cinnabar Island e Fuchsia City
(acesso pela Route 19), em Kanto, e Olivine City, Cianwood City, Goldenrod City
e Cherrygrove City, em Johto. São destinos candidatos, não linhas existentes
nem autorização para acesso antecipado. Cada parada exige conferir costa,
porto, identidade do navio/bilhete, marcos da campanha e sentido de ida e
volta. Serviços marítimos não substituem a travessia física planejada.

### Hoenn e Johto

A região de Fortree City pode se aproximar da região de Azalea Town por uma transição florestal. A hipótese preferencial não conecta diretamente as duas cidades: Route 119 ou Route 120 conduz a uma nova faixa tropical, que passa por relevo ou floresta temperada antes de alcançar Route 33 ou Ilex Forest.

```text
Fortree City
    |
Route 119 ou Route 120
    |
floresta e passagem intermediárias
    |
Route 33 ou Ilex Forest
    |
Azalea Town
```

## Dimensões atuais de referência

As medidas abaixo são as dimensões dos layouts atuais em blocos. A coluna ampliada apenas calcula 2× por eixo; ela não congela a escala final.

| Mapa | Atual | Referência 2× por eixo |
|---|---:|---:|
| Route 24 | 24 × 40 | 48 × 80 |
| Route 25 | 72 × 20 | 144 × 40 |
| Route 19 | 24 × 60 | 48 × 120 |
| Route 20 | 120 × 20 | 240 × 40 |
| Cinnabar Island | 24 × 20 | 48 × 40 |
| Lilycove City | 80 × 40 | 160 × 80 |
| Route 124 | 80 × 80 | 160 × 160 |
| Route 125 | 80 × 40 | 160 × 80 |
| Mossdeep City | 80 × 40 | 160 × 80 |
| Route 127 | 80 × 80 | 160 × 160 |
| Fortree City | 40 × 20 | 80 × 40 |
| Route 119 | 40 × 140 | 80 × 280 |
| Route 120 | 40 × 100 | 80 × 200 |

As dimensões de Johto e Sinnoh serão acrescentadas depois que suas fontes de implementação forem escolhidas e importadas. Até lá, não é possível afirmar se as bordas propostas cabem sem mapas intermediários.

## Continuidade técnica no PC

Um mundo contínuo não exige manter todas as regiões ativas na memória nem reuni-las em um único layout. O porte PC usará áreas técnicas ao redor do jogador: a área atual e as vizinhas são preparadas conforme ele se aproxima, enquanto as distantes são descarregadas.

A travessia não deve reposicionar ou teleportar o personagem, mesmo de maneira disfarçada. O jogador continua andando nas mesmas coordenadas do mundo, e o limite serve apenas para decidir quais dados permanecem ativos. Mapas ou chunks continuam úteis para edição, scripts, eventos, carregamento e testes, mas deixam de representar uma interrupção da exploração no PC.

Clima e ambiente pertencem a zonas geográficas maiores. Ao atravessar um limite técnico, o estado atual continua. Uma mudança real de zona pode ocorrer por distância e tempo, com uma faixa de transição, em vez de trocar instantaneamente por causa do nome do mapa.

O alvo GBA pode conservar o carregamento compatível com o aparelho original. O mundo contínuo e a janela dinâmica de áreas são uma evolução principal do porte PC, sem obrigar o GBA a manter dados que excedam sua memória.

Cada mapa exterior deverá poder registrar:

- coordenada mundial e região proprietária;
- bioma, zona climática e região ecológica;
- vizinhos, lado, deslocamento e extensão da borda compartilhada;
- faixa de transição ambiental;
- continuidade terrestre, aquática, subterrânea ou por transporte;
- condições de travessia e avanço correspondente do World Clock.

Os mapas importados de Kanto já conservam região proprietária, clima básico,
dimensões individuais e tabelas de encontros de FireRed no pacote do PC. Isso
os deixa prontos para receber metadados próprios sem depender da ROM, mas ainda
não significa ecologia dinâmica ou clima regional concluídos. Bioma, zona
climática, transições sazonais, população ecológica e coordenadas mundiais
continuam como uma camada futura. Os layouts podem ser redimensionados e
reempacotados, porém cada mapa ainda tem dimensões fixas; a janela dinâmica de
áreas e os mapas ampliados ainda precisam ser implementados e medidos.

## Critérios para escolher uma ligação

Antes de congelar uma conexão, o protótipo deve responder:

1. As bordas e dimensões permitem o encaixe sem deformar locais importantes?
2. A direção preserva a geografia reconhecível e a conexão oficial Kanto–Johto?
3. A travessia possui distância e conteúdo suficientes para parecer uma viagem entre regiões?
4. Eventos, NPCs, encontros, portos e progressão continuam coerentes?
5. O clima pode continuar ou mudar gradualmente sem depender da borda técnica?
6. A ligação funciona no mundo contínuo, no mapa regional, no Fly e nos transportes públicos?
7. As fontes e adaptações utilizadas estão registradas conforme a política do projeto?

## Critérios de desempenho

A ampliação e o carregamento contínuo serão medidos antes de ampliar todas as regiões. Os protótipos devem registrar:

- tempo médio e pior tempo de quadro;
- memória usada pela área atual, vizinhas, NPCs e efeitos ambientais;
- tempo de leitura e descompressão do pacote externo;
- ocorrência de pausas ao atravessar uma borda;
- quantidade segura de áreas vizinhas mantidas em espera;
- comportamento com mapas 2×, clima, encontros e vários personagens ativos;
- liberação correta de uma área depois que ela fica distante.

Se o custo ultrapassar a meta do porte PC, a solução deve reduzir dados ativos, antecipar carregamentos, reutilizar recursos e melhorar a composição dos chunks antes de reduzir a visão de mundo aprovada.

## Registro na planilha cartográfica

A futura planilha de mapas deve manter separadas a situação original, a hipótese e a decisão aprovada. Para cada conexão proposta, deve registrar pelo menos:

- identificador estável;
- origem e destino;
- lados e medidas das bordas;
- escala original e escala planejada;
- tipo de terreno ou mar;
- mapas intermediários;
- zona ambiental dos dois lados;
- condição narrativa e capacidade necessária;
- fonte cartográfica;
- estado `HIPÓTESE`, `EM PROTÓTIPO`, `APROVADA` ou `REJEITADA`;
- motivo da decisão e evidência do teste.
