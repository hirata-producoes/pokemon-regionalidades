# Política para versões, remakes e conteúdo exclusivo

## Regra central

Pokémon Regionalidades possui uma linha narrativa própria por região. Jogos
diferentes da mesma região são fontes de referência, não campanhas paralelas
ativadas ao mesmo tempo. Cada região escolhe uma versão-base estrutural; eventos
de outras versões só entram depois de classificados e adaptados sem duplicar
personagens, recompensas únicas ou estados incompatíveis.

## Hoenn

O código atual é compilado como `GAME_VERSION = EMERALD`. Portanto, Emerald é a
baseline estrutural de Hoenn: ordem principal, mapas, estados, Equipe Aqua e
Equipe Magma, Juan, Wallace, Rayquaza e Battle Frontier seguem inicialmente
essa referência.

Ruby e Sapphire servem para recuperar variações que acrescentem valor, como
diálogos, encontros ou pequenas missões exclusivas. Elas não substituem
silenciosamente a sequência de Emerald e não criam duas versões do mesmo
acontecimento.

Omega Ruby e Alpha Sapphire são referências de redesign. Personagens, locais,
Mega Evolução, Delta Episode e outras ideias desses remakes exigem adaptação
explícita ao mundo e à cronologia próprios do projeto. A existência de dados ou
de uma mecânica ORAS no `pokeemerald-expansion` não significa que sua história
já esteja integrada.

## Critério de fusão

Cada diferença entre versões recebe uma destas decisões:

1. **baseline**: permanece como na versão-base regional;
2. **variante compatível**: entra como diálogo, encontro, treinador ou missão
   secundária que não contradiz a baseline;
3. **adaptação própria**: duas ideias conflitantes são reescritas como uma única
   sequência coerente;
4. **alternativa contextual**: a ocorrência depende de estação, horário, escolha
   narrativa ou estado mundial, sem depender de comprar uma “versão” diferente;
5. **não integrada**: permanece apenas como referência documentada.

Conteúdo exclusivo de uma versão não será distribuído por edição do executável.
Espécies e encontros serão decididos pela ecologia mundial. Itens únicos usam
recompensas idempotentes; aparelhos como Pokédex serão módulos do RotomDex, não
cópias concorrentes. Layouts alternativos não podem ocupar simultaneamente o
mesmo lugar sem uma justificativa narrativa e técnica.

## Outras regiões

- Kanto possui dados de FireRed/LeafGreen no repositório, mas sua política final
  ainda deve comparar esses jogos com Red/Green/Blue/Yellow e Let's Go antes de
  congelar a campanha.
- Johto deve ter sua versão-base escolhida antes da importação extensa; Gold,
  Silver, Crystal e HGSS continuam como referências distintas.
- Sinnoh segue a mesma regra para Diamond, Pearl, Platinum e BDSP.

A escolha de uma versão-base não impede aproveitar conteúdo das demais. Ela
evita que a implementação comece com quatro estados contraditórios para cada
cidade e permite testar uma sequência reproduzível antes de ampliá-la.

## Registro obrigatório

Ao incorporar uma diferença de versão, documentar:

- jogo e cena de origem;
- classificação usada;
- adaptação realizada;
- marco narrativo e recompensa afetados;
- comportamento em saves anteriores;
- teste do caminho normal, repetição e chegada fora de ordem.
