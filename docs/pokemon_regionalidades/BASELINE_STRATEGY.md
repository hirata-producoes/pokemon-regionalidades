# Estratégia da versão-base

## Objetivo

Antes de ampliar mapas ou ativar sistemas de clima, ecologia e progressão mundial, o projeto deve possuir uma versão nativa para PC que reproduza de forma verificável o conteúdo-base das campanhas escolhidas.

A versão-base não é uma ROM gigante montada antes do porte. Ela é o mesmo motor compartilhado executado no PC, com cada região integrada como conteúdo e validada contra sua referência.

## Regra de prioridade

Enquanto a baseline de Hoenn não estiver jogável, novos sistemas ambientais permanecem em estado de contrato ou protótipo e não definem o comportamento final do jogo.

A ordem de trabalho é:

1. restaurar a renderização dos mapas originais no PC;
2. validar o começo de Emerald, do caminhão até Littleroot e Route 101;
3. validar uma sequência de história, batalha, captura, loja, Centro Pokémon e save;
4. percorrer Hoenn por marcos reproduzíveis até os créditos;
5. integrar e validar Kanto, Johto e Sinnoh como campanhas de conteúdo;
6. manter saves narrativos autênticos nos marcos da baseline;
7. ampliar mapas e ativar sistemas futuros por etapas comparáveis.

## Motor compartilhado e conteúdo regional

Movimentação, batalha, save, áudio, menus, Pokémon, golpes, habilidades e itens pertencem ao motor compartilhado. Mapas, scripts, diálogos, treinadores, encontros e progressão pertencem aos pacotes regionais.

Demakes baseados em `pokeemerald` ou `pokeemerald-expansion` podem reduzir o trabalho de adaptação de conteúdo de Nintendo DS para a estrutura GBA. Eles são fontes de implementação e referência, não substitutos automáticos do motor do projeto.

## Pontos de extensão futuros

Os mapas podem receber identificadores neutros de bioma, zona climática, grupo sazonal, região ecológica e grupo de chunks. Enquanto a respectiva funcionalidade estiver desativada, o motor deve preservar o comportamento original.

Essa regra evita que um protótipo incompleto se torne requisito para validar a campanha-base.

## Ampliação dos mapas

O valor inicial é aproximadamente 2× na largura e 2× na altura para mapas externos, produzindo aproximadamente 4× de área. Tiles, casas, árvores, personagens e objetos mantêm sua escala original.

A ampliação redistribui pontos de interesse e preenche as novas distâncias com conteúdo coerente. Não deve duplicar literalmente cada bloco original em uma matriz 2×2.

Interiores humanos permanecem próximos do tamanho original salvo necessidade. Routes e interiores naturais podem receber expansão maior quando isso melhorar exploração e conteúdo.

Antes dessa etapa, buffers fixos de mapas devem ser auditados. No PC, mapas grandes usarão alocação dinâmica ou chunks técnicos sem transição visual perceptível.

## Evidências mínimas

Cada marco regional deve registrar:

- build e versão dos recursos;
- ponto inicial e ponto final do percurso;
- transições verificadas;
- screenshots de referência;
- scripts e eventos essenciais;
- batalha, encontro, áudio e save;
- diferenças intencionais em relação à referência;
- regressões conhecidas.

Na rotina diária, o save de exploração permite visitar mapas e usar HMs sem alterar a história. Eventos narrativos usam saves gravados pelo próprio jogo antes da cena ou o percurso normal. A campanha completa é reservada para o fechamento de marcos e releases. O procedimento está documentado em [Estratégia de desenvolvimento e testes](TESTING_STRATEGY.md).

A baseline narrativa de Hoenn roda em `build/baseline-hoenn`, sem flags ou
perfis artificiais. Ela persiste o progresso normal entre sessões e permanece
separada tanto do save pessoal da raiz quanto do save técnico de exploração.

Um mapa presente no repositório não é considerado validado até aparecer corretamente e participar de uma sequência reproduzível.

Teletransportar para dentro de uma cena e preencher manualmente suas variáveis não conta como validação, pois pode deslocar NPCs, trocar encontros ou ocultar dependências. O marco da Route 101 só será fechado depois do percurso normal por Littleroot, do gatilho de resgate e da escolha do Pokémon inicial.
