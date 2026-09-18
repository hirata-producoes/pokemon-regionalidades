# Planejamento da geografia mundial

## Estado deste documento

Este documento registra hipóteses de composição para Kanto, Johto, Hoenn, Sinnoh e Ilhas Sevii. Ele detalha as decisões D-049, D-051 e D-055 da planilha, que continuam parcialmente decididas.

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

Cinnabar Island e as Routes 19 e 20 podem ser ajustadas para ocupar a faixa marítima próxima de Lilycove City. Mossdeep City, Route 125 e Route 127 podem delimitar outro lado do arquipélago de Sevii.

Composição provisória:

```text
Route 19
    |
Route 20 -- Cinnabar Island -- mar inter-regional -- Lilycove / Route 124
                                                       |
                                                  Route 125
                                                       |
                         Ilhas Sevii -- Mossdeep City -- Route 127
```

Essa representação indica relações, não coordenadas finais. A Route 124 continua sendo a ligação marítima de Lilycove com Mossdeep, e as ilhas não devem apagar essa função.

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
