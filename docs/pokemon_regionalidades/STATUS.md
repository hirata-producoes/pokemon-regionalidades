# Estado atual

## Câmera livre oculta para inspeção — 22 de setembro de 2026

O porte PC possui agora uma câmera de desenvolvedor separada da câmera do
personagem. `Ctrl + Shift + F` congela temporariamente a simulação e abre um
atlas de todos os mapas externos cadastrados como cidade, vila, rota terrestre
ou rota marítima. Arrastar
com o botão esquerdo move a visão, a roda do mouse controla o zoom de 0,2% a
800%, e `Esc` ou o mesmo atalho retorna ao jogo. A abertura enquadra o atlas
inteiro; o limite inferior reduzido permite comportar o mundo
durante a inspeção. A pausa que existia antes da
abertura é restaurada ao sair.

A visão é reconstruída diretamente dos layouts, metabloco, tilesets e paletas
empacotados. Portanto, reduzir o zoom revela terreno adicional de verdade, em
vez de apenas diminuir os 240×160 pixels já desenhados pelo GBA. As conexões
definem a posição relativa dos mapas ligados. Conjuntos ainda desconectados são
organizados em setores separados do atlas, sem fingir que existe uma passagem
entre eles. Cada mapa usa seu próprio conjunto gráfico.
O marcador vermelho indica a posição preservada do jogador. A ferramenta não
move o personagem, não executa colisões, warps ou eventos e não grava estado no
save. Sprites móveis e efeitos animados não fazem parte desta primeira visão
cartográfica estática.

O executável Windows compilou com sucesso e o pacote externo foi verificado:
14.938 recursos, 936 mapas e 785 layouts. A interação visual da nova câmera
ainda precisa de uma conferência manual curta no computador.

## Validação manual da campanha de Kanto — 22 de setembro de 2026

O responsável repetiu os testes manuais dos problemas relatados anteriormente,
confirmou as correções e conseguiu atravessar a Viridian Forest. Essa validação
encerra o bloqueio manual do início atual de Kanto; a continuação da campanha
pode avançar sem tratar novamente essas falhas como pendentes.

Em 11 de setembro de 2026, as leituras e alterações de itens durante o jogo no
PC consultam diretamente o bloco nativo `INVENT` depois que o save é carregado. O
espelho Emerald é reconstruído e comparado de forma independente no momento da
gravação, preservando a detecção de divergências e a compatibilidade com o alvo
GBA. O limite do PC foi elevado para 99.999 por posição; mochila, PC de itens,
lojas e seletores de quantidade transportam e exibem cinco dígitos. O espelho
Emerald permanece limitado a 999, sem reduzir o valor nativo. Os inventários
temporários do tutorial do Wally e dos modos de link preservam uma cópia nativa
completa antes de usar as matrizes antigas.

A compilação PC, os testes do contêiner e a compilação GBA passaram depois da
migração. O teste automático cobre 0, 999, 1.000, 99.998 e 99.999 e rejeita
100.000 na codificação. Outro teste inseriu 99.999 em uma cópia isolada, abriu e
gravou essa cópia pelo executável real e confirmou novamente 99.999 no bloco
`INVENT`. Captura, descarte, depósito, retirada parcial e retirada integral já
foram exercitados manualmente com cinco dígitos; a compra e a venda acima de 999
continuam como validação manual específica da Fase 3.

Em 11 de setembro, o primeiro teste manual com 99.999 Poké Bolas revelou um
encerramento ao selecionar uma delas durante uma batalha selvagem. O relatório
nativo localizou a falha em `CannotUseItemsInBattle`: itens usados diretamente
pela mochila passam `NULL` de propósito, mas a função lia os pontos de vida do
alvo antes de verificar o tipo do item. Essa leitura foi condicionada à
existência do alvo, foi acrescentado um teste de regressão para Poké Bolas e as
compilações PC e GBA voltaram a passar. O save isolado com 99.999 unidades foi
recriado. A confirmação manual concluiu normalmente o lançamento e a captura,
mostrou 99.998 na mochila e gravou o contêiner nativo na geração 24. A leitura
estrutural posterior confirmou `ItemId 1`, quantidade 99.998, no bloco `INVENT`.
Com a causa comprovada e corrigida, a instrumentação temporária de callbacks foi
retirada.

A revisão seguinte dos demais fluxos de inventário eliminou uma cópia obsoleta
de quantidade da mochila para um campo de tarefa de 16 bits. Também corrigiu a
compactação do PC de itens ao retirar a pilha inteira: o espaço que chega a zero
agora é removido antes de os itens restantes serem deslocados. O histórico
auxiliar da loja, usado por eventos de televisão e não pelo inventário, continua
deliberadamente limitado a 255 e agora aplica esse limite também na primeira
compra registrada. O build PC, os testes nativos de save, perfis, configurações
  e inventário, e o build GBA passaram depois dessas correções. Permanecia
  pendente a validação manual dos demais fluxos com uma quantidade nativa acima
  de 999.

O caso de regressão da compactação foi compilado. A suíte herdada completa não
chegou a executá-lo porque sua montagem parou posteriormente em
`test/pokemon.c`, por deslocamentos ARM grandes demais. Essa pendência do
executor de testes não ocorreu nos dois produtos compilados.

O primeiro teste manual das demais operações confirmou descarte, retirada
parcial, depósito, retirada integral e seletores com 99.999. A aparente perda do
save ao reabrir não veio do formato nativo: o lançador de teste recriava a cópia
a cada execução. Ele agora preserva a sessão existente por padrão e só a
substitui quando recebe `-Recreate`. Falta repetir apenas a gravação e reabertura
com o lançador corrigido para concluir essa evidência de persistência.

## Nomes e reinício seguro dos perfis — 11 de setembro de 2026

Os títulos fixos “Exploração e testes” e “Campanha de Hoenn” foram retirados da
interface. Cada um dos dois espaços agora possui um `profile.json` versionado e
pode receber um nome de 1 a 32 caracteres sem alterar o identificador interno ou
o caminho do save.

A ação `Reiniciar campanha` exige confirmação e só fica disponível quando existe
um save ativo. Em vez de apagar arquivos individualmente, o backend move a pasta
inteira do perfil para um nome datado ao lado dos perfis e recria somente o
espaço ativo, preservando o nome escolhido. Se a recriação falhar, a pasta
original é recolocada no lugar. Um teste isolado no Windows PowerShell 5.1
confirmou renomeação, rejeição de nome longo, importação, reconhecimento visual,
reinício, perfil ativo vazio e preservação exata do save anterior. Os dois
perfis reais não foram usados nessa validação. Nome do personagem, tempo jogado
e versão da última gravação continuam como próxima camada de identificação.

## Primeira implementação do save nativo — 9 de setembro de 2026

O executável PC agora lê e grava o contêiner `PGRSAVE` versão 1. Seu diretório
usa posições e tamanhos de 64 bits; cabeçalho, diretório e conteúdo dos blocos
possuem CRC32. O bloco obrigatório `LEGACY` conserva integralmente a imagem
Emerald de 128 KiB, permitindo que o gameplay continue inalterado enquanto os
novos sistemas ganham blocos próprios.

O carregador aceita um `.sav` antigo e cria `pokemon_regionalidades.pgrsave` ao
lado sem apagar ou modificar a origem. Arquivos corrompidos, truncados, grandes
demais, com blocos sobrepostos ou com versões obrigatórias desconhecidas são
recusados antes de chegar ao motor. Blocos opcionais desconhecidos são
preservados byte por byte quando o jogo grava novamente.

A rotação atômica deixou de exigir que todas as gerações tenham o mesmo tamanho,
pois blocos futuros farão o contêiner crescer. A interface de perfis reconhece
o formato antigo durante a transição, importa `.sav` diretamente para um
contêiner válido, exporta `.pgrsave`, lê a geração Emerald dentro do bloco e
continua oferecendo as três recuperações.

O teste reproduzível para Windows confirmou save novo, importação legada, duas
gerações, preservação de um bloco opcional desconhecido, detecção de corrupção e
rotação atômica. Cópias dos dois perfis reais foram convertidas em uma pasta
temporária: as gerações 13 e 9 permaneceram respectivamente 13 e 9, e ambas as
origens ficaram intactas.

A validação manual foi concluída em seguida: ambos os perfis abriram, gravaram e
reabriram normalmente. A inspeção somente leitura confirmou o perfil 1 na
geração Emerald 14 e geração 2 do contêiner, com recuperação nativa contendo as
gerações 13/1; o perfil 2 ficou nas gerações 10/2, com recuperação 9/1. Os dois
`.sav` legados continuaram válidos e inalterados nas gerações 13 e 9. Com isso,
a Fase 1 do save nativo está concluída.

Na Fase 2, o contêiner ganhou uma API para consultar, acrescentar e substituir
blocos por identificador. O esquema 1 de `WORLD` agora espelha os marcos
narrativos e as recompensas reais das quatro regiões em cada save confirmado no
PC. Ao carregar, o motor une os bits monotônicos de `WORLD` e `SaveBlock3`, de
modo que uma execução antiga que tenha atualizado apenas o legado não apague um
avanço e uma atualização nativa também não se perca.

O payload usa um cabeçalho de 32 bytes e registros variáveis de oito bytes, com
tipo, região e identificador de 32 bits em little-endian; não é uma cópia crua
das matrizes limitadas de uma estrutura C. O executável e as ferramentas de
perfis recusam um `WORLD` obrigatório malformado, duplicado ou de esquema
desconhecido, mesmo quando seus CRCs foram recalculados. Identificadores
reservados só podem ser marcados como obrigatórios quando seus respectivos
esquemas existirem; `ROTOMDEX` já cumpre esse contrato, enquanto `INVENT`,
`GEAR` e `TIME` continuam reservados.

Os testes isolados confirmaram codificação, leitura, preservação entre gerações,
rejeição estrutural e reconhecimento do novo bloco pela interface de perfis. O
porte PC e a ROM GBA foram compilados com sucesso. `WORLD` já é a fonte oficial
no PC e conserva no `SaveBlock3` somente um espelho de compatibilidade dos
identificadores que cabem no formato antigo.

Uma cópia da campanha-base de Hoenn também percorreu o fluxo real do motor duas
vezes em uma sessão oculta: carregou, aplicou o perfil técnico, salvou, reabriu o
contêiner com `WORLD` e salvou novamente. O resultado chegou à geração 3 do
contêiner e geração 5 de Emerald, com `LEGACY` de 131.072 bytes, seis registros
em um `WORLD` de 80 bytes e duas recuperações preservadas. Nenhum dos dois
perfis pessoais foi alterado por essa validação.

## Decisão estrutural do save nativo — 8 de setembro de 2026

A auditoria confirmou que o porte PC ainda persiste uma imagem de flash Emerald
de exatamente 131.072 bytes. O motor divide `SaveBlock1`, `SaveBlock2`,
`SaveBlock3` e o armazenamento Pokémon em setores fixos de 4 KiB; os lançadores
de perfis também validam esse tamanho. Esse é o limite herdado relevante. O uso
de `u16` em campos cuja faixa é pequena continua válido e não será substituído
indiscriminadamente.

Foi decidido que, antes de tornar outra região jogável, o PC adotará um
contêiner de save próprio, versionado e formado por blocos independentes. A
imagem atual continuará inteira em um bloco legado, enquanto progresso mundial,
RotomDex, inventário e sistemas futuros poderão migrar para blocos próprios. O
alvo GBA preservará o save antigo como referência e deixará de limitar a
persistência do produto PC.

Quantidades do inventário nativo usarão `u32` e terão limite lógico inicial de
99.999. `u64` ficará reservado a contadores realmente duradouros. Inteiros de
256 bits foram descartados para dados comuns: não ampliariam a liberdade prática
além de `u32`/`u64` e aumentariam complexidade, espaço e risco de erro. Hashes de
integridade podem continuar usando 256 bits como sequência de bytes.

A especificação completa, incluindo migração sem sobrescrever os `.sav` atuais,
preservação de blocos futuros, recuperação atômica e ordem de implementação,
está em [Arquitetura de save nativo para PC](PC_SAVE_ARCHITECTURE.md). Nesta
etapa nenhuma estrutura persistente nem perfil real foi alterado.

## Primeira versão do menu externo — 8 de setembro de 2026

A janela nativa agora possui uma barra simples no estilo de emulador, com os
menus `Jogo`, `Configurações` e `Ajuda`. Ela permite pausar, reiniciar a sessão,
voltar à escolha dos dois perfis, sair e abrir controles/aceleração durante a
execução. Alterações salvas nessa tela são percebidas pelo motor em até um
segundo, sem tocar no save. O reinício desta etapa relança o processo usando o
mesmo perfil; o Soft Reset totalmente interno ao motor continua planejado.

Cada início de sessão também preserva o registro anterior em
`runtime-history`, com retenção das cinco execuções mais recentes. O log informa
se a saída veio do fechamento da janela, do menu, do retorno aos perfis ou de um
reinício, evitando que uma execução posterior apague a única evidência de uma
falha intermitente.

A tela de perfis agora abre uma tela compartilhada de configurações. Nela é
possível remapear todas as teclas do GBA, a tecla de aceleração, as seis ações
principais do controle XInput e o botão usado para acelerar. O direcional e o
analógico esquerdo do controle continuam fixos para movimento nesta etapa.

O multiplicador momentâneo pode ser escolhido entre 2× e 10×. Os padrões atuais
permanecem iguais aos já usados pelo porte, e uma ação restaura todos eles. A
interface impede teclas ou botões duplicados. O arquivo compartilhado é substituído de maneira atômica e sua
versão anterior é preservada com o sufixo `before-settings`.

A tela agora separa prévia e confirmação: `Aplicar` atualiza o jogo sem fechar,
`Cancelar` restaura o arquivo que existia ao abrir a janela e `Salvar` torna a
seleção permanente. As prévias não geram uma sequência de backups artificiais.

O executável foi recompilado e um teste isolado confirmou que valores diferentes
dos padrões chegaram ao motor: C/V no lugar de Z/X, Shift esquerdo para acelerar,
B/Y no controle, gatilho esquerdo para acelerar e multiplicador 3×. As duas telas
também passaram pela validação estrutural no Windows PowerShell 5.1. A composição
visual e o uso com um controle físico ainda dependem de uma verificação manual
curta.

O primeiro teste manual encontrou dois defeitos nessa interface: o cálculo das
colunas arredondava alguns índices e colocava direção direita e aceleração fora
da janela; além disso, o limitador de pausas do laço principal ainda tomava 5×
como máximo e reduzia multiplicadores acima disso a aproximadamente 1×. As
colunas agora usam divisão inteira, o teste rejeita seletores fora da área visível
e o limitador mede pausas pelo passo normal de 60 Hz, independentemente do
multiplicador escolhido. O porte PC foi recompilado após as correções.

O fechamento reproduzido ao aguardar na tela de título foi localizado no áudio,
não em um temporizador: `mus_title` seleciona o instrumento 127, mas sua fonte
declara somente 90 instrumentos. No GBA essa leitura alcançava dados ROM
adjacentes; depois da divisão em recursos externos, ela ultrapassava a alocação
isolada. Os bancos do PC agora mantêm os 128 índices endereçáveis e identificadores
desconhecidos usam uma amostra silenciosa segura. Amostras já resolvidas e gritos
compactados carregados pelo pacote também são reconhecidos sem perder o áudio. A
versão recompilada permaneceu 55 segundos na tela de título — além dos cerca de
44 segundos da falha reproduzida — com o registro limpo, sem exceção nem amostra
desconhecida.

A validação manual posterior manteve a tela de título parada por dois minutos sem
fechamento. Durante a mesma sessão, alterações de velocidade e de teclas foram
aplicadas, a tela de configurações pôde ser reaberta e a ação `Voltar aos perfis`
retornou corretamente ao seletor. Essa primeira versão da integração externa fica
validada para teclado; a conferência com um controle físico continua independente.

A tela externa ganhou uma terceira aba para vídeo. Ela permite escolher janela
redimensionável ou fixa, tamanhos de 2× a 5×, tela cheia, escala inteira, VSync e
moldura. O motor aplica as mudanças durante a sessão e só redefine as dimensões
quando escala, tela cheia ou modo fixo realmente mudam. Um teste nativo confirmou
a troca ao vivo entre uma janela fixa de `960 × 540` e uma redimensionável de
`640 × 360`; a composição visual das opções ainda aguarda verificação manual.

O enquadramento não inteiro passou a ocupar a maior área possível da janela sem
alterar a proporção 3:2. A interface identifica `1280 × 720` como tamanho
recomendado, explica que a escala inteira pode criar margens e informa `Alt +
Enter` para entrar ou sair da tela cheia. O atalho também foi integrado ao motor
e persiste a escolha no arquivo compartilhado.

A quarta aba oferece volume geral, música e efeitos/gritos em níveis de 0 a 10.
O mixer classifica os canais pela faixa que os originou e aplica música ou
efeitos antes do volume geral, incluindo os quatro canais sonoros clássicos. A
estrutura da interface, a leitura e a preservação dos três valores passaram nos
testes automatizados; a diferença auditiva ainda aguarda confirmação manual.

O fechamento relatado após capturar Aron na Granite Cave foi localizado na saída
da tela de apelido: no PC, os sprites ainda podiam terminar o quadro depois que
o estado da tela já havia sido liberado. A saída agora desativa esses callbacks
antes da liberação e também protege a atualização de vídeo correspondente.

O bloqueio narrativo observado em Slateport no save de exploração tem outra
causa: as oito insígnias artificiais fazem Dock escolher o diálogo posterior à
sétima insígnia, antes do trecho que remove a fila do Team Aqua. Isso confirma
que o perfil atual é adequado para mobilidade, mas não para validar a ordem da
história; sua forma de liberar HMs precisa ser separada das insígnias narrativas.

## Base de dois perfis pessoais — 7 de setembro de 2026

O porte PC agora aceita caminhos independentes para save e configuração sem
alterar a execução direta já existente. Um lançador provisório abre os perfis 1
e 2 em `%LOCALAPPDATA%\Pokemon Regionalidades`: cada perfil possui seu próprio
save e suas recuperações, enquanto as configurações do programa permanecem
compartilhadas.

A importação é explícita, copia o arquivo em vez de movê-lo, confirma seu SHA-256
e é recusada quando o perfil já possui progresso. O teste técnico confirmou a
cópia idêntica, a recusa de uma segunda importação e o isolamento entre os dois
destinos. Uma execução oculta do jogo também confirmou pelo registro que os
caminhos isolados foram realmente usados. A seleção e a restauração já estão na
interface gráfica; nomes personalizados permanecem pendentes.

O backend de recuperação também passou a listar a geração interna encontrada em
um conjunto completo de 14 setores, evitando depender apenas do horário do
arquivo. Nas gravações reais da campanha-base, as três versões foram
identificadas como gerações 3, 2 e 1. O teste de restauração promoveu a geração
escolhida a save ativo por substituição atômica e confirmou, por SHA-256, que o
ativo anterior permaneceu intacto em `before-restore`. A operação é recusada
enquanto houver uma instância do jogo aberta.

Uma primeira interface gráfica para Windows foi construída sobre esse mesmo
backend. Ela apresenta os dois perfis em cartões separados, informa se o perfil
é novo ou possui save, permite importar um arquivo apenas em destino vazio e
lista as recuperações por sua geração interna. A estrutura foi exercitada
automaticamente com um perfil contendo duas recuperações e outro vazio. A
composição visual já recebeu uma validação manual curta. A restauração de uma
recuperação ainda deve ser validada manualmente em um perfil real; essa interface
é uma etapa funcional anterior ao menu definitivo do produto.

O primeiro teste no Windows PowerShell 5.1 revelou que o script UTF-8 sem marca
de codificação era interpretado como ANSI; o travessão transformado interrompia
a análise antes de abrir a janela. A interface passou a usar UTF-8 com BOM e foi
executada novamente pelo próprio `powershell.exe` 5.1 com validação concluída.

A abertura manual confirmou a composição esperada: título e instrução legíveis,
dois cartões alinhados, ações visíveis e ambos os perfis vazios, sem migração
silenciosa de saves. A interface também ganhou uma ação para criar seu próprio
atalho na área de trabalho; o acabamento do ícone permanece planejado junto da
identidade visual definitiva.

Por decisão do responsável, o perfil 1 recebeu uma cópia verificada do save de
exploração com os cinco Pokémon adicionais e o perfil 2 recebeu uma cópia da
campanha normal salva na Route 102 com Mudkip. Os hashes dos destinos
correspondem às respectivas origens, que permaneceram intactas. A interface
identifica esses papéis como `Exploração e testes` e `Campanha de Hoenn`.

A exportação também foi implementada no backend e na interface. O teste confirmou
uma cópia idêntica ao perfil e verificou que substituir uma exportação preserva a
versão anterior em `before-export`. Importar, exportar e restaurar permanecem
operações distintas para reduzir o risco de trocar a direção de uma cópia.

## Gravação segura no PC — 7 de setembro de 2026

A primeira camada de proteção de saves foi integrada ao porte PC. O motor agora
termina e verifica sua gravação interna antes de confirmar o arquivo físico. O
novo conteúdo passa por um arquivo temporário, o save ativo anterior é mantido
em uma rotação de três recuperações e a troca final é atômica. Uma falha de
escrita também passa a ser informada ao fluxo de salvamento do jogo, em vez de
ser registrada apenas no log.

O teste técnico realizou cinco gravações diferentes e confirmou o conteúdo do
save atual e das recuperações 1, 2 e 3. Repetir uma gravação idêntica não avançou
a rotação. Depois disso, o executável completo para Windows foi recompilado com
sucesso. A seleção dos dois perfis e a escolha de uma recuperação já estão na
interface; nomes personalizados, metadados amigáveis e o menu externo definitivo
ainda não fazem parte desta etapa.

A integração também foi confirmada manualmente na campanha-base de Hoenn. Depois
de carregar o progresso real, gravar em dois novos estados e reabrir a sessão, o
diretório continha o save ativo e `recovery-1` e `recovery-2`, todos com os
131.072 bytes esperados. Não foi necessário reiniciar a história nem modificar o
save de exploração.

## Estado mundial nativo no PC — 9 de setembro de 2026

Os dois perfis reais foram abertos, gravados e reabertos normalmente depois da
introdução do bloco `WORLD`. Na segunda fase, esse bloco passou de espelho para
fonte oficial do progresso multirregional no PC. Sua capacidade nativa é de
4.096 marcos narrativos por região e 1.024 recompensas por categoria; a
serialização esparsa mantém o arquivo proporcional somente ao progresso usado.

O `SaveBlock3` preserva um espelho dos identificadores antigos para migração e
para o alvo GBA, mas não define mais o teto do PC. Os testes cobrem os maiores
identificadores permitidos, rejeição de conteúdo inválido, recuperações e leitura
pelas ferramentas de perfis. O porte PC recompilou com sucesso. A interface de
perfis ainda receberá nomes escolhidos pela pessoa, nome do personagem, tempo de
jogo, última versão usada e reinício explícito da campanha sem herdar gerações
anteriores.

A máscara cumulativa dos módulos regionais do RotomDex também passou a ter seu
próprio bloco obrigatório, `ROTOMDEX`, com esquema e validação independentes. O
valor antigo é unido na migração e mantido como espelho de compatibilidade, sem
permitir que o módulo de uma região sobrescreva outro. Uma sessão técnica isolada
gravou e reabriu duas vezes o contêiner com `LEGACY`, `WORLD` e `ROTOMDEX`; as
ferramentas de perfis reconheceram normalmente os três blocos.

## Primeiro retrato do inventário nativo — 9 de setembro de 2026

A fase 3 começou com o esquema 1 de `INVENT`, sem modificar ainda os menus ou o
limite efetivo da mochila. Cada registro preserva compartimento, posição, item e
quantidade; isso mantém a ordem atual e aceita itens divididos em mais de um
espaço. O formato reserva 4.096 posições em cada um dos cinco compartimentos da
mochila e no PC de itens, usando quantidade de 32 bits com teto lógico inicial
de 99.999. Como a codificação é esparsa, posições vazias não ocupam o save.

Os testes cobriram codificação, reabertura, posições extremas e quantidade
99.999. A sessão técnica isolada gravou e reabriu o contêiner com quatro blocos:
`LEGACY`, `WORLD`, `ROTOMDEX` e `INVENT`; as ferramentas de perfis validaram o
resultado. Nesta etapa o inventário legado continua sendo a fonte do gameplay e
o bloco nativo é regenerado ao salvar. Essa barreira permite testar a migração
antes de mudar assinaturas, telas e operações de itens.

O teste manual dos dois perfis confirmou a mochila, a Potion guardada no PC
pessoal, a gravação e a reabertura; entregar um item para um Pokémon segurar
também continuou funcionando. Itens segurados pertencem aos dados do Pokémon e
não são duplicados em `INVENT`.

Depois dessa confirmação, o esquema recebeu um marcador de autoridade. Enquanto
estiver em modo de espelho, o carregamento compara integralmente `INVENT` com a
imagem legada e recusa divergências em vez de escolher um lado e perder itens. O
modo nativo fica reservado, mas só será aceito quando as APIs e telas suportarem
as quantidades ampliadas. A comparação passou em um novo ciclo real da sessão
técnica isolada.

As funções centrais de verificar espaço e posse, adicionar, remover e somar itens
da mochila e do PC passaram a usar quantidades de 32 bits. A alteração foi
compilada contra todos os consumidores no PC e no GBA e percorreu uma gravação
técnica real. O teto continua em 999 nesta etapa; portanto, esse trabalho remove
truncamentos nas interfaces internas sem antecipar a mudança visual ou promover
prematuramente `INVENT` como fonte oficial.

Após os testes manuais de depósito, retirada, descarte, troca de posição e item
segurado, o espelho nativo deixou de ser apenas uma fotografia criada ao salvar:
cada alteração feita pelas operações centrais passa a atualizar também o estado
`INVENT` em memória. Os poucos fluxos que substituem matrizes inteiras — novo
jogo, inventário inicial do PC, tutorial do Wally e restauração temporária de
link — fazem uma sincronização integral explícita. A autoridade continua legada
nesta etapa, permitindo validar o espelho vivo antes da inversão definitiva. A
gravação também reconstrói uma cópia independente apenas para conferência e é
recusada se algum caminho não tiver atualizado o espelho; ela não encobre a
divergência regenerando `INVENT` silenciosamente.

Depois dessa barreira, `INVENT` passou a ser a autoridade persistente no PC para
o subconjunto que o gameplay atual representa. Um contêiner antigo em modo de
espelho é promovido na gravação seguinte; ao reabrir um contêiner nativo, o bloco
alimenta a mochila e o PC legados usados pelas telas. A projeção está limitada a
999 unidades e às posições atuais. Se uma versão futura trouxer quantidades ou
posições maiores, esta versão recusa o carregamento em vez de truncar itens.

A representação usada durante o gameplay também foi separada fisicamente da
estrutura persistida pelo Emerald. `LegacyItemSlot` conserva exatamente quatro
bytes e a mochila legada conserva `0x2E8` bytes, protegidos por verificações de
compilação; `ItemSlot` passa a transportar quantidades de 32 bits sem deslocar
nenhum campo dos saves existentes. A ampliação revelou uma dependência implícita
do truncamento de 16 bits na criptografia da mochila. A máscara foi tornada
explícita, e duas gravações e reaberturas isoladas voltaram a concluir com
sucesso. O teste técnico também passou a exigir a linha completa de sucesso,
evitando confundir códigos de erro que apenas comecem pelo mesmo dígito.

## Compilação cruzada PC e GBA — 7 de setembro de 2026

O alvo GBA voltou a gerar `pokemon_regionalidades.gba` depois da separação
explícita entre as implementações físicas do console e a camada de plataforma do
PC. O script de build agora reconhece corretamente o histórico quando o projeto
está em um `git worktree` criado no Windows, sem recorrer a `.histignore`.

As implementações de BIOS, DMA, áudio, registradores e recursos externos em
`src/platform` ficaram restritas ao porte nativo. As regras compartilhadas de
áudio usam operações neutras no GBA e preservam as conversões exigidas pelo
montador do Windows. Depois dessas correções, o alvo PC também foi recompilado
com sucesso.

O link da ROM registrou 22,88 MiB de ROM, 217,74 KiB de EWRAM e 28,39 KiB de
IWRAM. Permanecem 7,62 MiB de ROM para conteúdo após a reserva de segurança de
1,5 MiB. A IWRAM, com somente 3,61 KiB livres, é o limite mais próximo e deve ser
acompanhada antes de integrar sistemas grandes. Essa medição veio de um build
incremental bem-sucedido e foi reproduzida por uma compilação com limpeza
completa, fechando o marco técnico dos dois alvos.

## Isolamento da base e das sessões de desenvolvimento — 7 de setembro de 2026

O save de exploração continua usando o código e os recursos da árvore oficial, mas agora executa uma cópia coerente dos três artefatos necessários dentro de `build/dev-save-exploracao`. O lançador não mistura mais o executável da raiz com pacote, configuração e save da sessão.

Cada preparação cria `dev-session-manifest.txt` com tamanho e SHA-256 do executável, pacote e SDL2. Uma verificação sem abrir o jogo confirmou que as três cópias correspondem exatamente aos artefatos oficiais atuais e que o hash do save não mudou.

O perfil artificial de mobilidade passou a exigir duas chaves de processo simultâneas. O gerador define ambas somente para a execução oculta que grava a cópia. Abrir o jogo normalmente ou abrir o save de exploração depois de preparado não reaplica o perfil. O alvo PC foi recompilado com sucesso depois dessa proteção.

Protótipos futuros permanecem desativados na base normal até funcionarem com os mesmos carregadores, recursos e saves, em pelo menos dois cenários representativos. Um resultado isolado não será usado como prova de funcionamento na campanha de Hoenn.

A campanha-base ganhou uma sessão persistente separada em
`build/baseline-hoenn`. O lançador sincroniza os três artefatos oficiais, remove
do processo todas as chaves de automação e perfil técnico e inicia o jogo sem
alterar o progresso. Reiniciar essa sessão preserva o save anterior em uma pasta
de backup. Assim, a próxima validação narrativa pode partir de `New Game` sem
tocar no save pessoal da raiz ou no save de exploração.

O primeiro percurso nessa sessão foi concluído manualmente desde `New Game` até
Oldale. Funcionaram a introdução, o caminhão, a casa, o relógio, a rival, o
resgate na Route 101, a escolha do inicial, a primeira batalha contra a rival, o
laboratório, a entrega da Pokédex e a chegada à cidade. O perfil do personagem C
mostrou zero insígnias, uma entrada na Pokédex e 23 minutos, estado coerente com
uma campanha nova sem o perfil artificial de mobilidade. O salvamento, o
fechamento da sessão, a reabertura pelo mesmo lançador e o `Continue` também
funcionaram normalmente. Isso conclui a primeira vertical slice persistente de
Hoenn sem usar estado narrativo artificial.

## Detalhe da Pokédex, armazenamento e batalha — 7 de setembro de 2026

O teste manual confirmou que a lista da Pokédex abre e fecha, a equipe funciona e Fly realizou viagens para vários destinos. Isso valida as correções anteriores nesses percursos básicos. O detalhe de uma espécie, o PC do Centro Pokémon e uma batalha contra a rival ainda encerraram o programa.

Os registros mapearam três acessos inválidos distintos. `UpdateSelectedMonSpriteId` indexava `gSprites` antes de verificar o marcador `0xFFFF` de uma vaga vazia. `m4aMPlayStop` recebeu um tocador de áudio nulo durante o percurso do armazenamento. `PlayAnimation` recebeu legitimamente um buffer de argumentos nulo para uma animação criada pelo motor, mas o desreferenciava. A ordem da verificação da Pokédex foi corrigida, a parada de áudio passou a aceitar ponteiro nulo e animações sem buffer usam argumento neutro zero.

O alvo PC foi compilado depois das três correções. A confirmação manual deve cobrir o detalhe de uma espécie registrada, o PC de armazenamento e uma batalha comum ou narrativa alcançada pelo fluxo real. O save de exploração não foi modificado nesta etapa.

## Pokédex, equipe e Fly — 7 de setembro de 2026

Depois de concluir o laboratório pelo fluxo real, o save B passou a conter a Pokédex e as Poké Bolas entregues pela rival. O teste seguinte revelou encerramentos ao abrir a Pokédex e ao sair do mapa de Fly.

A falha da Pokédex ocorreu em `CreatePokedexList`: `REGIONAL_DEX_COUNT` inclui a entrada `DEX_NONE`, mas a criação da lista percorria esse valor como se fosse uma espécie. A conversão devolvia zero e o índice dos bits de visto/capturado sofria underflow. A lista agora percorre somente as entradas reais.

O mapa de Fly liberava `sFlyMap` dentro de seu callback de saída e ainda atualizava os sprites no mesmo quadro. Os ícones acessavam o ponteiro já nulo. `CB2_FlyMap` agora encerra o quadro imediatamente após a liberação.

O perfil de mobilidade passou a reconhecer com segurança tanto o save com apenas Mudkip quanto a equipe completa já criada. Ele preserva os seis Pokémon e registra Tropius, Pelipper, Wailord, Lanturn e Breloom como vistos e capturados. O save posterior ao laboratório foi copiado para `build/save-backups/B-pos-laboratorio-20260907-001241.sav` antes da atualização. O executável e o save foram gerados com sucesso; Pokédex, equipe e Fly aguardam confirmação manual.

## Chamadas de eventos no PC — 7 de setembro de 2026

O teste manual confirmou abertura e fechamento da equipe e do perfil após as correções anteriores. Surf funcionou, mas Fly, Cut, entregas e algumas batalhas ainda apresentaram encerramentos.

O registro de exceção identificou uma chamada nativa a `IsFollowerFieldMoveUser` com `ROM_SIZE` (32 MiB) somado ao endereço. O Expansion usa o espelhamento de ROM do GBA para marcar funções que declaram efeitos; esse endereço não é executável no Windows. As macros `callnative`, `gotonative` e a tabela de `special` agora emitem endereços diretos no alvo PC, mantendo o comportamento GBA. A análise antecipada de efeitos permanece conservadora no PC, sem inferir metadados dos bits de endereços sujeitos a ASLR. Metadados nativos completos continuam pendentes.

As cinco referências compiladas à função identificada foram verificadas: endereços nativos presentes, referências ao espelho ausentes. A compilação foi concluída. Fly, Cut, entrega da rival e batalhas ainda exigem confirmação manual; a correção da causa registrada não comprova todos os caminhos relatados.

O comando `faceplayer` agora orienta o personagem pelas coordenadas do jogador. A direção anterior usava o inverso da direção do jogador, o que podia virar um segundo treinador para longe durante uma abordagem dupla. A validação visual desse ajuste também está pendente. O save B não foi recriado ou modificado nesta etapa.

## Correção do save de exploração em 6 de setembro de 2026

O primeiro save artificial em Oldale foi invalidado pelo teste manual: o mapa mudou, mas objetos do caminhão permaneceram. Esse teleporte foi removido. O save autêntico do personagem B foi preservado em backup e recebeu cinco Pokémon no nível 75, mantendo seu Mudkip no nível 8, a posição e o progresso. O perfil não concede a Pokédex artificialmente.

Os eventos de falha do Windows identificaram divisão por zero em `GetScaledHPFraction` e acesso a dados liberados em `VblankCb_TrainerCard`. O gerador agora calcula os atributos dos Pokémon e o fechamento do perfil desativa seus callbacks gráficos antes de liberar a memória. Compilação e gravação do save foram confirmadas; validação manual dos menus continua pendente. Falhas ao conversar com o professor e acessar o armazenamento ainda precisam de diagnóstico. Execuções normais gravam `runtime-last.log` no diretório do save.

Este registro substitui as descrições históricas abaixo sobre iniciar artificialmente em Oldale e substituir toda a equipe.

## Resumo

O projeto possui uma fundação GBA e um porte nativo para Windows. O porte não emula a ROM: ele recompila o código e substitui serviços de hardware por implementações de PC.

## Validado em execução

- inicialização SDL2 e renderização;
- tela de copyright, introdução, título e novo jogo;
- teclado e controle;
- save, carregamento e RTC;
- áudio MP2K não silencioso;
- menu de equipe e resumo;
- batalha selvagem de diagnóstico;
- pacote externo válido, ausente e corrompido;
- recursos externos de Pokémon, cries, tilesets, animações, samples, músicas e voicegroups.

## Baseline inicial validada manualmente

Em 1º de setembro de 2026, o carregamento dos primeiros mapas foi repetido depois da correção da procura pelo pacote externo. O interior do caminhão passou a apresentar chão, paredes, caixas e saída; a transição para Littleroot e a entrada na casa do personagem também foram percorridas manualmente.

Isso confirma que o conteúdo original não estava ausente. O programa havia sido iniciado em um diretório diferente e abria sem localizar corretamente o pacote de mapas e tilesets.

O carregador passa a procurar o pacote primeiro ao lado do executável, evitando que atalhos, launchers e ferramentas de teste dependam do diretório de trabalho do processo. Como mapas e tilesets já foram externalizados, pacote ausente ou inválido agora interrompe a inicialização com uma mensagem clara em vez de permitir uma sessão visualmente corrompida.

O teclado também preserva por um frame toda tecla recebida em `SDL_KEYDOWN`. Assim, um toque curto não desaparece quando `SDL_KEYDOWN` e `SDL_KEYUP` são processados antes da próxima amostragem do motor.

Durante a mesma validação foi identificada uma regressão visual nos menus: escolhas como gênero e confirmações continuavam funcionais, porém o glifo triangular que indica a opção selecionada podia não aparecer. Outros cursores, como o da tela de nome, foram confirmados visualmente. Como um caractere textual alternativo também não resolveu a sessão testada, o porte PC passou a desenhar diretamente um pequeno chevron no buffer da janela, sem depender da fonte. O alvo GBA mantém o triângulo original. O novo indicador foi confirmado visualmente em 1º de setembro de 2026.

Em 2 de setembro de 2026, uma execução técnica direta na Route 101 encontrou uma falha real nas conexões entre mapas externalizados: a borda visual tentava ler o marcador compilado do mapa vizinho, e não seus dados no pacote. A cópia das conexões passou a resolver primeiro os layouts de Oldale e Littleroot. Essa correção de carregamento continua válida.

Entretanto, a tentativa posterior de transformar esse cenário em checkpoint narrativo não foi aceita como evidência da história. Em teste manual, personagens apareceram em posições incompatíveis e o resgate apresentou Zigzagoon onde a referência esperada era Poochyena. Isso demonstrou que preencher parcialmente flags, variáveis e coordenadas pode criar tanto falso positivo quanto falso negativo. Os checkpoints narrativos e seus executores foram removidos.

A introdução percorrida manualmente — caminhão, chegada a Littleroot, casa, relógio e transmissão — permanece como evidência. A visita à rival, a Route 101, o resgate, a escolha do inicial, a primeira batalha da rival e o retorno ao laboratório ainda precisam ser validados como uma única sequência real. O fechamento inesperado observado depois da batalha da rival também permanece em investigação.

Em 6 de setembro de 2026, foi criado um save isolado de exploração. Ele parte de um `.sav` válido, preserva suas variáveis narrativas, posiciona a cópia em Oldale, substitui sua equipe, libera mobilidade e é gravado pelo sistema oficial do jogo. O arquivo recebeu cinco Pokémon de Hoenn no nível 75, os oito HMs, as oito insígnias e todos os destinos de Fly de Hoenn. Sua gravação retornou `SAVE_STATUS_OK`; a posição e os recursos artificiais desse save não contam como prova narrativa.

A rotina atual evita capturas em massa e trajetos automáticos baseados em coordenadas. O responsável pelo jogo verifica aspectos visuais a partir de instruções curtas; testes técnicos cobrem compilação, recursos e falhas nativas; eventos de história usam o percurso real ou saves autênticos gravados antes da cena. Consulte [Estratégia de desenvolvimento e testes](TESTING_STRATEGY.md).

## Implementado, mas ainda em validação

Os defeitos de texto observados em nomes de Pokémon, `Lv`, gênero, seleção de inicial, opções `YES/NO`, golpes e nomes de mapas foram rastreados até uma única rotina de cópia de glifos em `src/text.c`. Dois deslocamentos de 32 bits tinham comportamento indefinido: no x86 eles podiam apagar uma faixa completa de oito pixels e repetir o mesmo fragmento no tile seguinte. A operação foi tornada definida para todos os alvos. O build PC foi concluído; a confirmação visual dessa correção permanece pendente.

O primeiro contrato de gameplay derivado da planilha foi integrado:

- World Clock durante a execução alterado de 20× para 3×;
- calendário corrigido de 28 para 30 dias internos por estação;
- fases de transição dos dias 29–30 e 1–2 representadas por API própria;
- tempo real transcorrido com o jogo fechado aplicado em 3× ao carregar o save;
- âncora offline armazenada em duas variáveis antes não utilizadas, sem ampliar os SaveBlocks;
- menu inicial e submenus pausam o relógio por estado transitório, sem gravar um flag de pausa no save;
- popup de área apresenta um protótipo compacto com estação, dia sazonal e horário;
- clima lógico determinístico calculado por estação, região, área e blocos de seis horas;
- previsão das próximas 24 horas permanece estável e não consome o RNG de gameplay;
- mapa externo de Littleroot traduz o estado lógico em clima visual e pode atualizar o efeito durante a permanência no mapa; uma ocorrência de chuva foi observada em teste manual;
- popup de área passou a separar corretamente o texto ambiental do horário e agora inclui a condição climática;
- cálculo determinístico da troca de estação coberto por testes de fronteira;
- build nativo para Windows concluído depois da alteração.

O executável recompilado também passou por smoke test isolado: permaneceu estável durante a inicialização, validou o RTC nativo, abriu o pacote de 11.564 recursos e produziu frames da introdução sem usar o save de desenvolvimento.

O teste manual também encontrou duas limitações visuais do protótipo ambiental:

- o efeito de nuvens do estado `CLOUDY` aparece recortado em apenas algumas faixas ou blocos da tela;
- o popup concatena `LITTLEROOT TOWN` e os dados ambientais em largura insuficiente, causando sobreposição e quebra visual.

Esses defeitos não impedem a campanha-base, mas precisam ser corrigidos antes de o clima e o popup serem considerados prontos. Os textos ambientais atuais em inglês também são provisórios; a interface principal será escrita primeiro em português do Brasil.

Os casos de teste da pausa, dos nomes sazonais, da estabilidade climática no bloco de seis horas e da previsão através da meia-noite foram adicionados e compilados no alvo de testes. A execução automatizada continua pendente porque as ferramentas POSIX do test runner não compilam pelo MinGW e o serviço WSL não estava acessível nesta sessão. Ainda faltam o teste manual da pausa, a validação visual completa do popup e de Littleroot, a mudança climática ao vivo e um teste com fechamento e reabertura controlados. Portanto, D-023 e o protótipo de D-029 estão integrados em sua base, mas ainda não estão validados por completo.

Layouts de mapas foram externalizados em 884 recursos. O build foi concluído e `maps.o` diminuiu de 955.473 para 305.313 bytes. O caminhão, Littleroot, a primeira casa e suas transições foram validados com o pacote atual. O carregamento isolado da Route 101 e de suas bordas conectadas também foi validado. Ainda faltam:

- percorrer manualmente a história original até a Route 101 e a escolha do inicial;
- testar outros tipos de layout e transições;
- testar ausência e corrupção dessa família;
- confirmar o fallback;
- registrar capturas e logs do marco.

## Validação manual de 7 de setembro de 2026

A Pokédex passou a abrir, fechar e exibir páginas individuais normalmente. Fly, Surf, captura de Pokémon e batalhas contra treinadores também foram confirmados no save de exploração. Em uma sessão sem gravar o progresso, encontros selvagens continuaram estáveis nas rotas 101, 102 e 103; na rota 113 foram concluídas captura de Slugma, vitória contra Skarmory e vitória contra Spinda; nas rotas 132 e 134 foram confirmados, respectivamente, a coleta de um Rare Candy e o combate contra Swimmer Laurel e Swimmer Jack. Fugir de encontros também funcionou. Esses resultados validam as funções testadas, mas não substituem o percurso narrativo da campanha.

Dois fechamentos independentes continuam sendo tratados:

- o segundo encontro com Spinda atingiu `AnimShakeMonOrBattlePlatforms`; a falha não era específica da espécie, mas uma reconstrução incorreta de ponteiro a partir de campos assinados de 16 bits no executável nativo. A correção foi aplicada também aos outros callbacks encontrados com o mesmo padrão;
- o PC de armazenamento atingiu `CpuSet` ao preparar recursos gráficos. Três testes distinguiram os caminhos de mover, depositar e mover itens. O relatório preservado mostrou que caixas vazias tentavam carregar uma paleta de espécie ainda não inicializada; o teste de depósito chegou à movimentação do cursor e tentou ajustar um sprite de Pokémon segurado que ainda não existia. O estado do armazenamento passou a ser inicializado com zero, posições vazias usam uma paleta neutra e a prioridade só é alterada quando o sprite existe. Testes seguintes revelaram ainda um quadro de animação executado depois de liberar o estado compartilhado; o callback agora interrompe esse quadro. A correção foi validada manualmente com abertura e saída das três opções, depósito, retirada, movimentação entre box e equipe e troca de posições. A navegação de Mover Itens foi validada, mas a transferência efetiva permanece pendente por falta de um Pokémon com item no save testado. Relatórios que contêm uma exceção são arquivados automaticamente na execução seguinte.

Depois dessa correção, o mesmo save confirmou os Running Shoes, salvamento, Fly até Verdanturf, Cut na Route 117 e batalhas contra Pokémon Breeder Lydia, Bug Maniac Derek, Psychic Brandi e Triathlete Melina. O progresso posterior ao save foi descartado intencionalmente ao fechar o programa. Esse conjunto estabelece uma base estável de exploração para testes direcionados, sem declarar toda a campanha de Hoenn validada.

Uma auditoria preventiva encontrou outros locais que reconstruíam valores ou ponteiros de 32 bits a partir de campos assinados de 16 bits. As conversões foram tornadas explícitas em tarefas, animações, Poké Balls, experiência e Rock Smash. O alvo PC foi recompilado com sucesso; essas alterações reduzem uma classe conhecida de falhas nativas sem alterar as regras do jogo.

O pipeline de recursos agora possui verificação integral opcional. Em 7 de setembro de 2026, o pacote recompilado confirmou 11.564 recursos e 16.489.008 bytes, com todos os payloads, hashes e checksums correspondentes ao manifesto. Uma cópia com um byte deliberadamente alterado foi rejeitada por divergência de checksum, validando também o caminho negativo. A auditoria lógica confirmou 936 mapas em 76 grupos, 785 layouts, 442 layouts externalizados e 884 recursos de layout no pacote. Todos os layouts usados, conexões e destinos de warp resolvem para definições existentes. Quatro diretórios explicitamente não utilizados permanecem fora dos grupos e 19 eventos historicamente posicionados além das bordas são informados como notas, não ocultados como sucesso silencioso.

O histórico herdado do `pokeemerald-expansion` já possui encontros e filtros da Pokédex por manhã, dia, entardecer e noite. Essa estrutura será integrada ao World Clock em vez de duplicada.

A fundação do RotomDex mundial também foi iniciada sem liberar antecipadamente a antiga National Dex plana. O save agora possui um estado versionado de pesquisa regional, enquanto os registros globais existentes continuam preservando todo Pokémon visto ou capturado. A interface futura poderá apresentar catálogos de Kanto, Johto, Hoenn e Sinnoh liberados pelos respectivos professores e manter espécies observadas antes dessa liberação em uma área de notas de campo. Consulte [Arquitetura do RotomDex mundial](WORLD_DEX_ARCHITECTURE.md).

## Planejado

- campanhas regionais além da base atual;
- seleção da região inicial depois de `New Game`, usada também para testes de campanhas em desenvolvimento;
- sistema multilíngue separado da lógica do jogo;
- exportação e importação segura de saves entre computadores, com backup e validação;
- dois perfis pessoais com três recuperações por perfil;
- save e carregamento rápidos em estados seguros, além de Soft Reset separado;
- menu externo para controles, vídeo, áudio, aceleração e gestão de saves;
- pacote externo para eventos, gráficos gerais, fontes e scripts;
- testes automatizados mais amplos;
- empacotamento de release;
- revisão futura da necessidade de 64 bits.

Português do Brasil é o idioma principal e padrão de autoria do projeto. Outros idiomas serão traduções sobre a mesma lógica, não versões separadas do jogo.

A sequência de baseline e expansão está documentada em [Estratégia da versão-base](BASELINE_STRATEGY.md).

## Artefatos atuais

Os novos builds usam:

- `pokemon_regionalidades-pc.exe`;
- `pokemon_regionalidades.pak`;
- `pokemon_regionalidades.pgrsave` no PC e `.sav` como origem legada importável;
- `pokemon_regionalidades.cfg`;
- `pokemon_regionalidades.gba` no alvo GBA.

Ao iniciar pela primeira vez com o nome novo, o PC copia save e configuração antigos quando eles existem e os novos ainda não existem. Os arquivos antigos não são apagados.

## Última validação de build

Em 7 de setembro de 2026, os dois alvos foram recompilados depois da separação
das implementações específicas de plataforma. O resultado foi:

- executável nativo recompilado com sucesso;
- pacote de 16.489.008 bytes;
- 11.564 recursos indexados;
- leitura integral do índice do pacote concluída.
- ROM GBA limpa gerada com sucesso;
- orçamento registrado em 22,88 MiB de ROM, 217,74 KiB de EWRAM e 28,39 KiB de IWRAM.

O percurso manual anterior pela introdução, caminhão, Littleroot e casa inicial continua sendo a referência da sequência normal. Resultados obtidos por posicionamento narrativo artificial não são mais usados para ampliar essa afirmação.

O build GBA foi executado no PowerShell normal do responsável pelo projeto,
porque o ambiente isolado do Codex não acessa o serviço WSL do Windows. O
procedimento reproduzível está em [Compilando a versão para GBA](BUILDING_GBA.md).
## Fundação multirregional

- existe um registro compilado único para as quatro regiões do mundo planejado;
- Hoenn é a única campanha marcada como jogável;
- Kanto é identificado como conteúdo de mapas presente, ainda não como campanha pronta;
- Johto e Sinnoh permanecem explicitamente planejadas;
- a primeira tela regional aparece antes da introdução, consulta esse registro e não permite abrir uma campanha incompleta;
- introdução e ponto inicial agora fazem parte do contrato de cada região; o início de jogo consulta esse contrato em vez de estar ligado diretamente ao caminhão de Hoenn;
- somente a entrada de Hoenn está validada. As demais regiões não possuem ponto inicial ativo e não podem substituir silenciosamente a campanha-base;
- o Perfil 1 agora recebe permissão técnica para HMs sem fabricar insígnias narrativas;
- ao abrir o Perfil 1, insígnias artificiais antigas são reconciliadas com os líderes realmente derrotados, evitando que scripts como o de Slateport interpretem exploração como fim de história.
- a tela `Continue` também usa esse progresso narrativo real, inclusive antes da primeira correção persistente de um save técnico antigo.

A transição entre o seletor regional e a fala do professor ainda pode exibir por um instante resíduos gráficos no porte PC. A introdução e seus diálogos se recuperam e o defeito está classificado como visual, sem bloquear a fundação multirregional. A sobreposição refinada será retomada quando o seletor usar uma camada gráfica independente da introdução.

## Fundação de continuidade narrativa — 8 de setembro de 2026

Integração seguinte: laboratório e sapatos agora consultam os pré-requisitos.
A migração versão 2 distingue capacidade de correr e
progresso narrativo. Testes nativos de contrato passaram; a sequência visual
completa desta integração permanece aguardando teste manual isolado.

O save recebeu um bloco versionado para progresso regional sem alterar os
offsets anteriores do `SaveBlock3`. Cada região da V1 possui 128 marcos próprios;
recompensas possuem registros globais e regionais separados. A nova estrutura
de progresso usa 108 bytes; o `SaveBlock3` inteiro ocupa agora 128 dos 1.624
bytes disponíveis.

Os primeiros marcos de Hoenn — Littleroot, resgate de Birch, rival da Route 103,
Pokédex e Running Shoes — acompanham as flags canônicas em ordem. Saves criados
antes dessa fundação importam esses marcos na primeira consulta, sem exigir uma
campanha nova. Repetir a conclusão de um marco não duplica o estado.

A entrega segura de itens únicos só registra a recompensa depois de confirmar
que ela já está na mochila ou que foi adicionada com sucesso. Falta de espaço
mantém a recompensa pendente. Scripts receberam uma ponte para consultar
pré-requisitos, concluir marcos e entregar itens sem acessar diretamente o
formato do save.

O desbloqueio de catálogo da Pokédex também passou a usar a região atual. Isso
evita que conversar com um professor futuro em Kanto libere novamente o catálogo
da região inicial quando o jogador começou em Hoenn.

O produto final usará um único RotomDex. O professor inicial entrega o aparelho;
os demais professores instalam módulos regionais ou aprimoramentos. Visto e
capturado permanecem mundiais, enquanto a máscara regional controla as abas e
os conhecimentos técnicos disponíveis. As cinco Poké Balls iniciais continuam
como consumível comum e foram retiradas do registro de recompensas únicas.

O executável PC foi recompilado com sucesso. A estrutura e os testes de unidade
foram adicionados; a execução automatizada do test runner continua sujeita às
limitações já registradas do ambiente. Ainda não foram convertidos arcos inteiros
de Hoenn: essa migração será gradual e acompanhada por testes narrativos.

## Continuidade narrativa de Hoenn — 12 de setembro de 2026

A conversão gradual alcançou a Seafloor Cavern. Os marcos registrados agora
cobrem a campanha desde Littleroot até o despertar de Kyogre, incluindo os
arcos Devon, Mauville, Mt. Chimney, Fortree, Mt. Pyre, Magma Hideout, roubo do
submarino e Aqua Hideout.

Na Seafloor Cavern, a exploração física permanece livre. Integrantes da Equipe
Aqua, Archie e o Kyogre adormecido, porém, só aparecem quando a fuga do
submarino já foi concluída. O gatilho final também consulta o marco narrativo,
impedindo a execução da cena sobre personagens ocultos. O despertar registra
um novo marco e continua usando os estados originais para iniciar a crise
climática em Sootopolis.

O teste nativo da implementação real passou até esse marco. A conversão seguinte
separou em marcos a chegada à crise de Sootopolis, a conversa com Wallace, a
abertura narrativa de Sky Pillar, o despertar de Rayquaza e o encerramento da
crise. A porta de Sky Pillar deixou de ser uma barreira física de história, mas
Wallace e Rayquaza continuam ocultos até seus pré-requisitos.

A migração de saves anteriores agora usa a versão 15 do registro. A cena, sua
persistência após salvar e as tentativas antecipadas pelo Perfil 1 ainda
aguardam validação manual.

## Fechamento da lacuna de Mossdeep — 13 de setembro de 2026

A auditoria da sequência revelou que a primeira conversão ligava o Aqua
Hideout diretamente à Seafloor Cavern. A ordem correta agora contém três
marcos intermediários: derrota de Tate e Liza, defesa do Centro Espacial e
entrega narrativa de Dive por Steven.

Mossdeep continua fisicamente acessível. Antes da hora, os líderes não iniciam
a primeira batalha e a Equipe Magma não aparece no exterior nem nos dois
andares do Centro Espacial. Os gatilhos automáticos foram separados dos estados
persistentes do mapa para não tentar mover personagens ocultos. Depois da
vitória conjunta com Steven, a visita à casa dele conclui a entrega narrativa;
ter o HM antecipadamente não fabrica esse acontecimento.

O registro passou para a versão 16 sem renumerar nenhum identificador anterior.
Saves antigos inferem os três novos marcos apenas de flags e estados canônicos,
preservando os bits das demais regiões. O contrato nativo e a compilação PC
completa passaram após a integração. A validação visual e de persistência nos
dois perfis permanece manual.

## Liga de Hoenn no progresso multirregional — 13 de setembro de 2026

A continuidade registrada agora alcança o fim da campanha-base de Hoenn. A
conversa em que Wallace entrega Waterfall, a primeira vitória contra Juan, o
encontro com Wally na Victory Road, a entrada na Liga, cada integrante da Elite
Four e o título de Campeão receberam marcos encadeados.

Brawly recebeu um marco próprio e permanece uma ramificação flexível: pode ser
enfrentado em diferentes momentos depois do início de Hoenn, mas sua vitória e
o encontro com Wally precisam estar concluídos para entrar na Liga. Dessa forma,
a liberdade de exploração não permite terminar a região com uma insígnia
regional ausente.

Possuir Waterfall antecipadamente continua permitindo seu uso e não conclui a
conversa de Wallace. Quando a recompensa narrativa ocorre, o script reconhece
um HM já existente e evita duplicá-lo. A entrada da Liga deixou de depender da
suposição original de que bastava verificar a insígnia de Fortree; agora ela
consulta a sequência regional concluída até Wally e a vitória flexível contra
Brawly.

As verificações da Elite Four controlam a primeira passagem e também aceitam
novas tentativas depois que o jogo limpa as flags temporárias dos líderes. O
título de Hoenn permanece regional e não substitui futuras Ligas nem o
Campeonato Mundial. A versão 18 preservou os identificadores anteriores, o
contrato nativo passou até a Liga e o executável PC foi recompilado com sucesso.

Os textos herdados de Roxanne, Brawly, Wattson, Flannery, Norman, Winona, Juan e
Wallace também foram alinhados ao comportamento atual. Eles não afirmam mais
que uma insígnia libera Cut, Flash, Rock Smash, Strength, Surf, Fly ou Waterfall;
as insígnias continuam representando vitórias regionais, enquanto o uso de
campo depende de um Pokémon apto que conheça o movimento.

## Auditoria de abertura e variantes — 13 de setembro de 2026

A revisão posterior encontrou duas lacunas anteriores à primeira insígnia: o
tutorial de captura do Wally ainda não possuía marco próprio, e Roxanne ainda
aceitava a primeira batalha sem consultar o progresso mundial. A versão 19
acrescenta o tutorial ao final dos identificadores persistidos, passa a exigi-lo
antes das primeiras batalhas de Roxanne e Brawly e preserva rematches e saves
anteriores por migração canônica.

Também foi separado um inventário entre campanha principal, cenas secundárias e
pós-jogo. Emerald foi confirmado como baseline compilada de Hoenn. Ruby,
Sapphire e ORAS permanecem fontes classificadas e não linhas narrativas
simultâneas. Consulte [Auditoria de Hoenn](HOENN_EVENT_AUDIT.md) e [Política de
variantes](VERSION_VARIANTS_POLICY.md).

Na primeira fatia dos eventos secundários, a rival de Lilycove passou a aparecer
somente depois do encontro concluído na Route 119. Scott no Centro Pokémon de
Ever Grande passou a depender da vitória contra Juan, em vez de apenas verificar
a antiga insígnia de Fortree. Os dois atores são novamente ocultados depois de
suas conversas e nenhum deles bloqueia a campanha principal.

## Atualização pós-Liga da RotomDex — 13 de setembro de 2026

O retorno ao laboratório de Birch depois do primeiro título de Hoenn recebeu um
marco próprio e idempotente. A cena agora registra a atualização do arquivo de
pesquisa mundial da RotomDex sem ativar antecipadamente a antiga National Dex e
sem entregar automaticamente o inicial de Johto. Esse inicial permanece
reservado para uma etapa futura que tenha contexto narrativo multirregional.

Saves antigos que já haviam concluído a recompensa herdada preservam o Pokémon
recebido; estados intermediários antigos são normalizados sem duplicar a cena.
O contrato nativo de progressão passou até esse novo marco e o executável PC foi
recompilado com sucesso.

## Fechamento ao usar Spark no porte PC — 13 de setembro de 2026

Um teste no Perfil 2 encontrou uma exceção de divisão inteira por zero durante a
animação de Spark usada por um Minun selvagem. O intervalo de alternância visual
dessa variante é legitimamente zero; o comportamento herdado era tolerado no
alvo GBA, mas causava encerramento imediato no executável x86.

A animação elétrica agora interpreta intervalo zero como ausência de alternância
de visibilidade e mantém inalterado o comportamento dos intervalos positivos. O
porte PC recompilou com sucesso. Relatórios de exceção continuam sendo
preservados automaticamente como `runtime-crash-AAAAmmdd-HHMMSS.log` quando o
jogo é aberto novamente, permitindo retomar uma campanha sem perder o diagnóstico.

## Cadeia pós-jogo da S.S. Tidal — 13 de setembro de 2026

A versão 22 do progresso separa três marcos que antes dependiam principalmente
de `FLAG_SYS_GAME_CLEAR`: recebimento do S.S. Ticket, encontro com Scott a bordo
e primeira recepção na Battle Frontier. Os marcos são acrescentados ao final da
enumeração persistente e saves anteriores importam as flags canônicas sem
renumerar nenhum acontecimento existente.

Slateport e Lilycove agora só operam e exibem a S.S. Tidal quando o bilhete foi
recebido de forma narrativa e está na mochila. O percurso entre os dois portos
abre com o bilhete, enquanto a Battle Frontier só entra na lista depois do
convite válido de Scott. A recepção e o Frontier Pass exigem esse encontro, mas
uma chegada técnica antecipada ainda permite explorar sem concluir cenas fora
de ordem.

O contrato nativo passou por toda a sequência até a recepção da Battle Frontier
e o executável PC foi recompilado com sucesso. A cena completa, a viagem, o
salvamento e a repetição continuam como validação manual futura de pós-jogo.

Na versão 23, a primeira visita à casa de Scott também foi ligada à recepção.
Scott fica ausente se o mapa for alcançado antecipadamente, entrega os Battle
Points iniciais apenas depois da cerimônia e registra essa conversa em um marco
próprio. As recompensas posteriores por símbolos e sequências continuam usando
suas condições originais, que já evitam repetição e preservam a tentativa quando
não há espaço para a Berry ou decoração. O contrato e a compilação PC passaram
novamente após essa extensão.

## Desafio pós-Liga de Steven — 13 de setembro de 2026

A versão 24 acrescenta a batalha opcional de Steven em Meteor Falls ao progresso
regional. Steven não aparece na caverna antes de o jogador se tornar Campeão de
Hoenn, mesmo que o mapa seja alcançado por mobilidade técnica. Depois do título,
o desafio fica disponível independentemente da visita à Battle Frontier.

A vitória é persistida em um marco próprio e a flag herdada continua sendo
importada por saves anteriores. Perder a batalha não conclui o marco, e visitas
posteriores à vitória mantêm apenas o diálogo final. O texto também foi corrigido
para identificar corretamente o Centro Espacial de Mossdeep. O contrato nativo
e a compilação PC passaram após a integração.

## Pausa da auditoria interna da Battle Frontier — 13 de setembro de 2026

A revisão individual das sete instalações, símbolos e Frontier Brains foi
adiada para economizar esforço nesta fase. A Battle Frontier herdada continua
acessível após a cadeia validada de bilhete, navio, convite e recepção; portanto,
essa auditoria não bloqueia o desenvolvimento multirregional. Ela deverá voltar
antes de declarar o pós-jogo de Hoenn completamente validado.

O próximo bloco estrutural passa a ser a política de itens únicos e equipamentos
compartilhados. Exp. Share, bicicletas, Coin Case, Itemfinder, Go-Goggles,
bilhetes, roupas e equivalentes regionais precisam ser classificados como
consumível, item narrativo regional, ferramenta global ou variação cosmética.
Essa decisão é necessária antes de Kanto para impedir duplicação, sobrescrita e
scripts regionais incompatíveis no mesmo save mundial.

## Política de itens entre regiões — 14 de setembro de 2026

A pesquisa comparativa de Kanto, Johto, Hoenn e Sinnoh foi convertida em uma
política própria do projeto. TMs e HMs passam a ser aquisições permanentes; a
configuração compartilhada de TMs reutilizáveis foi ativada. Exp. Share passa a
ser uma capacidade global ativável no modelo moderno: participantes recebem
100% e integrantes elegíveis que não participaram recebem 50%, sem parcela dupla.

Bicicletas serão variantes colecionáveis trocadas pelo PC pessoal, Go Goggles integra a
capacidade geral de proteção ambiental, Coin Case representa uma carteira global
e Itemfinder torna-se melhoria cumulativa do RotomDex. Bilhetes podem preservar
o mesmo nome histórico, mas exibem navio, partida e destino, e suas permissões
internas ficam separadas por serviço ou rota.

O catálogo está em [Política de itens e equipamentos entre regiões](CROSS_REGION_ITEM_POLICY.md).
O núcleo já oferece o cálculo moderno inclusive para múltiplos participantes e
Double Battles. A migração do PC converte cópias equipadas do Exp. Share em uma
única capacidade global na bolsa de itens importantes, preserva a escolha de
ligado ou desligado e impede perda mesmo se bolsa e PC estiverem cheios. TMs e
HMs permanentes são reduzidos a uma única posse e novos recebimentos não criam
quantidades escondidas, inclusive quando o disco está guardado no PC pessoal.

O porte PC foi recompilado com sucesso depois dessa integração. A validação
manual restante é ensinar um TM, confirmar que ele permanece disponível,
alternar o Exp. Share e comparar a experiência de participantes e não
participantes. Os registros nativos de equipamento e viagem e a migração das
demais recompensas de Hoenn continuam antes da ativação de Kanto.

Durante uma Double Battle, o desmaio de Nincada revelou uma ordem interna de
equipe inválida: os seis identificadores chegaram como zero, e a preparação do
menu substituiu temporariamente todas as posições por cópias do primeiro
Pokémon. Como todas as posições também pareciam ser o membro já ativo, nenhuma
delas podia ser escolhida. O menu agora valida que a ordem contém exatamente os
seis identificadores distintos e, se necessário, a reconstrói a partir dos dois
Pokémon ativos antes de copiar qualquer dado. A compilação PC passou; falta
repetir manualmente um desmaio com substituição obrigatória em batalha dupla.

## Seguidores e paletas no PC — 19 de setembro de 2026

O protótipo de Pokémon seguindo foi ativado apenas no porte PC, sem alterar os
blocos legados do save. Na primeira validação manual, seis Pokémon exibiram
silhuetas corretas e cores incorretas. A causa identificada foi dupla: o
gerador do pacote PC não incluía as paletas de mundo recém-ativadas e o
carregamento do seguidor usava o identificador compilado em vez dos 32 bytes da
paleta no pacote. O gerador agora inclui essas paletas e o carregamento resolve
o recurso antes de aplicá-lo; a seleção normal/brilhante e feminina também foi
revisada. A compilação passou e o recurso do Marshtomp consta no pacote. Em
teste manual posterior, Swampert, Crobat, Aggron, Metang, Gardevoir e Trapinch
apareceram com as cores corretas. O defeito relatado para esses seis está
resolvido; isso não prova a cobertura de todas as espécies e formas. Mudança de
mapa, batalha e reabertura de save ainda precisam de validação específica do
acompanhante.

## Campanha de Hoenn e seguidor — 20 de setembro de 2026

Na campanha pessoal, o jogador concluiu os oito ginásios, a Elite Four e os
créditos. O pós-jogo está em validação: já recebeu do pai o bilhete para os
navios e obteve e reviveu os dois fósseis, um pela Mirage Tower e outro pelo
túnel em Fallarbor Town. Isso confirma o percurso principal testado, não todos
os desvios, eventos opcionais ou caminhos fora de ordem.

O seguidor está funcionando na campanha observada, inclusive com as cores
corretas dos seis Pokémon citados acima. Passa de protótipo sem validação visual
para recurso implementado e em acompanhamento de regressões. Não será exigido
testar exaustivamente cada espécie e cada bloco antes de avançar o projeto.
No PC, a capacidade de posições de cada bolso da mochila foi triplicada no
código. O jogador observou que a mochila está maior e conseguiu retirar itens
do PC pessoal. Ainda falta testar o uso próximo ao limite das posições novas e
a persistência dessas posições extras após salvar e reabrir.

## Lista de perfis e favoritos — 20 de setembro de 2026

A interface inicial passou a listar perfis de forma rolável e a exibir os
detalhes somente do perfil selecionado. O iniciador aceita novos identificadores
sem limite fixo de dois perfis, mantendo os caminhos antigos. Cada campanha
conserva três recuperações rotativas e agora pode guardar até cinco favoritos
fixos, independentes dessa rotação. Apagar um perfil o arquiva em vez de apagar
seus arquivos; remover um favorito também preserva a cópia anterior. Testes
temporários verificaram criação, importação, cinco favoritos, recusa do sexto,
restauração, remoção, arquivamento e isolamento de outro perfil. Ainda falta
validação visual manual com os perfis reais. Nenhum save pessoal foi alterado
durante esses testes.

## Preparação geográfica Cinnabar–Route 124 — 20 de setembro de 2026

Os prints marcados pelo jogador identificaram dez rochas de 2×2 blocos no sul
de Cinnabar Island e onze no trecho norte da Route 124. Somente esses 84
blocos foram substituídos por mar compatível com Surf e com o tileset de cada
mapa. A comparação binária confirmou o mesmo tamanho dos layouts e exatamente
40 blocos alterados em Cinnabar e 44 na Route 124; a verificação estrutural dos
936 mapas passou. Nenhuma conexão ou cena de Kanto foi ativada por essa edição.

## Abertura de Kanto em teste — 20 de setembro de 2026

O menu do PC agora distingue `INICIO EM TESTE` de campanha `JOGAVEL` e permite
selecionar Kanto sem declarar a região completa. A introdução usa o professor
Oak, Nidoran, Red ou Leaf e texto inicial em português adaptado aos caracteres
atualmente aceitos pelo motor. O novo jogo chega ao quarto da casa do jogador
em Pallet Town, sobre um bloco de piso livre. A aparência de campo usa os
sprites de Red ou Leaf conforme o gênero escolhido. As flags iniciais de FRLG
são preparadas junto das flags comuns e o nome padrão do rival em Kanto é Green;
isso evita usar o nome da rival de Hoenn sem alterar o formato de save.
Saves antigos sem estado mundial inicializado continuam sendo interpretados
como Hoenn, e não como Kanto apenas porque o valor inicial da variável é zero.

A compilação PC e as verificações do pacote de recursos e dos 936 mapas passaram.
O perfil 3 validou manualmente a introdução original de Kanto, a ausência da
sequência do caminhão, o quarto, o encontro com Oak, a escolha do inicial e a
entrada na primeira batalha. O antigo defeito gráfico do quarto não reapareceu.
A primeira batalha terminou de forma antecipada porque a IA herdou da abertura
de Hoenn a fuga automática do oponente quando o jogador chegou a 20% de HP. O
registro ampliado confirmou o resultado `MON_FLED`; a abertura de Kanto agora
mantém a batalha até uma condição normal de vitória ou derrota. Isso não era uma
permissão para o jogador fugir de uma luta entre treinadores. O nome do rival
também deixa de aparecer temporariamente como May durante a apresentação: antes
de o estado regional ser gravado, o texto passa a consultar diretamente o nome
que acabou de ser escolhido.

Um perfil originado em Hoenn encontrou Oak visível no meio de Pallet porque as
flags da abertura de Kanto não pertenciam àquela campanha. A transição agora
oculta esse Oak de campanha para visitantes, sem iniciar a narrativa regional.
Também foi corrigido o esgotamento de buffers gráficos ao atravessar repetidas
vezes a conexão Cinnabar–Route 124. A repetição da travessia foi validada
manualmente, mas revelou uma vista retangular antiga sobre o mar: a vista salva
agora é descartada antes da montagem do mapa quando muda o conjunto gráfico
primário. Essa última correção ainda aguarda validação manual.
Os diálogos e eventos importados de FRLG após a abertura ainda exigem revisão de
português, identidades regionais de treinadores e pré-requisitos narrativos.
Kanto continua em teste somente no porte PC.

## Janela do PC e interface em duas telas — 21 de setembro de 2026

O executável passa a ocultar apenas sua janela técnica, sem remover os registros
de diagnóstico de cada perfil e sem ocultar a janela SDL do jogo. O vídeo aceita
também a escala 1×, de 320×180,
como menor tamanho predefinido, e a criação da janela respeita desde o início a
escala e a opção de redimensionamento gravadas. A futura interface permanente
deixa de ser planejada como uma borda ao redor da câmera e passa a usar uma
segunda tela inferior, inspirada na organização de HeartGold e SoulSilver, para
informações ambientais e futuras interfaces próprias. Essa segunda tela é uma
decisão de planejamento. Seu primeiro protótipo passa a ocupar 320×180 abaixo
do jogo, mostrando permanentemente local, clima, estação, horário, data e turno.
Mapa, RotomDex e mochila aparecem apenas como reservas visuais, ainda sem ação.
O popup temporário de entrada em mapas foi desativado no PC para evitar
informação duplicada.

A primeira validação mostrou que o painel ocupava toda a largura de 320 pixels,
enquanto a imagem 3:2 do jogo ficava centralizada em uma área menor. O painel
passa a usar exatamente o mesmo retângulo renderizado pelo jogo. Mouse e troca
de foco por Start passaram à primeira implementação interativa no PC: Start
abre o menu na tela inferior e Start/B retornam ao mapa; direcionais e A
selecionam as ações. O mouse abre o painel pelo rodapé e aciona os botões.
O painel reutiliza a lista, os desbloqueios e as funções do menu original;
o menu sobreposto no mapa deixa de ser desenhado. As telas internas de bolsa,
equipe, Pokédex e confirmação de salvamento ainda usam a tela principal.
Os gráficos são provisórios: as folhas HGSS fornecidas por link não puderam
ser obtidas nesta etapa (servidor respondeu HTTP 403). Não foram importados
sprites nem animações dessas folhas. Validação manual dos controles pendente.

## Primeiro ciclo jogável de Kanto — 21 de setembro de 2026

Route 1, Viridian City e o Poké Mart de Viridian deixaram o modo de prévia e
passaram a carregar seus personagens, scripts e interações no PC. Esse recorte
forma um ciclo narrativo único: sair de Pallet, atravessar Route 1, receber o
pacote de Oak no mercado e retornar ao laboratório para a entrega e a etapa da
Pokédex. Os mapas não contêm treinadores próprios, portanto esta ativação não
introduz novas identidades de batalha que possam colidir com Hoenn. O ciclo
ainda aguarda validação manual completa antes de liberar o próximo trecho.
Os scripts e suas dependências foram incluídos na compilação; a cena automática
do Pacote de Oak é restrita a perfis iniciados em Kanto. Visitantes podem comprar
normalmente no mercado sem receber o pacote nem alterar a cena do laboratório.
Route 1/Viridian usam marcadores persistentes próprios para os dois itens,
o tutor e a descoberta da cidade. As três variáveis de progressão de Viridian
foram deslocadas para posições livres do save: seus números originais de FRLG
colidiam com eventos de Hoenn. Não é feita migração dos valores desses números
antigos, pois poderiam pertencer legitimamente à campanha de Hoenn.

### Centro Pokémon de Viridian

O primeiro andar foi ativado com seus NPCs e o ponto de retorno após derrota.
A enfermeira usa o fluxo compartilhado de cura, incluindo a atualização do
Pokémon seguidor, para visitantes e campanhas iniciadas em Kanto. O monitor
da máquina escolhe a arte de FRLG pelo layout do prédio, sem alterar o monitor
dos Centros de Hoenn. O segundo andar e seus serviços de comunicação continuam
fora desta ativação. Cura, recusa, acesso ao PC e retorno após derrota ainda
aguardam teste manual do usuário.

O commit 297ca08238 registra somente os perfis e favoritos já validados;
os avanços de Kanto e a interface inferior seguem separados, sem novo commit.

### Rota 2 e Floresta de Viridian

Route 2, as portarias sul e norte e Viridian Forest formam agora o segundo
trecho contínuo ativado da campanha. As tabelas selvagens de FireRed/LeafGreen
já existentes são usadas na rota e na floresta. Os seis itens visíveis, os dois
itens ocultos e a descoberta da floresta receberam marcadores persistentes
próprios, sem reutilizar progresso de Hoenn.

Os cinco Bug Catchers da floresta inauguram o espaço regional de treinadores:
Rick, Doug, Sammy, Anthony e Charlie possuem dados e identificadores exclusivos
de Kanto. A vitória de cada um é gravada no bloco nativo de progresso de Kanto,
fora da faixa antiga de flags de treinadores de Hoenn. Essa base evita colisões
agora e permite acrescentar os próximos treinadores na ordem da campanha sem
deslocar identidades ou o formato legado do save.

A compilação PC completa passou com 14.938 recursos, 936 mapas, 785 layouts e
1.570 recursos de layout verificados. O executável não foi aberto nesta etapa.
O teste manual seguinte deve cobrir a travessia Viridian–Route 2–portaria sul–
floresta–portaria norte, encontros selvagens, coleta persistente dos itens e a
derrota persistente dos cinco treinadores após salvar e recarregar.

## Correções do ciclo inicial de Kanto — 21 de setembro de 2026

A primeira batalha do laboratório e o tutorial de captura agora recebem uma
identificação explícita de tutorial de Kanto. Isso impede o núcleo Emerald de
selecionar Birch, Wally, o Pokémon e os controladores de Hoenn para essas cenas.
A marcação foi aplicada também ao caminho específico de batalha de treinador do
rival, inclusive ao resultado de derrota. A tela de nome do jogador passa a
pedir o ícone regional de novo jogo, permitindo que Red ou Leaf seja escolhido
no lugar do avatar de Hoenn.

O fechamento encontrado no primeiro Bug Catcher da Floresta foi localizado na
expansão de texto de batalha. O sistema opcional de fala no meio da luta usava o
identificador regional `0x1000` diretamente como índice da tabela antiga de
Hoenn e acabava lendo um ponteiro fora dela quando a vida ficava baixa. Os
treinadores de Kanto deixam de consultar essa tabela; encontro, derrota e fala
pós-batalha continuam ativos normalmente. Uma tabela regional de falas especiais
poderá ser criada depois sem reintroduzir a colisão.

A interface de perfis agora nomeia claramente campanha atual, histórico
automático e save fixo. Reiniciar uma campanha mantém os saves fixos disponíveis
no perfil recriado, além da cópia integral arquivada. O teste automatizado dessa
preservação passou. A recompilação PC passou novamente com 14.938 recursos, 936
mapas, 785 layouts e 1.570 recursos de layout verificados. Avatar, derrota do
rival, tutorial de captura e treinadores da floresta ainda exigem a repetição do
teste manual no executável novo.
A integração seletiva de Johto usa a revisão
`167aa6d537b109bb229c231ddce4616974c4da71` de `pokehns-expansion`.
Nove mapas e seus layouts entraram no mundo único: Cherrygrove City, Route 29,
New Bark Town, Route 27, Tohjo Falls, Route 26, Route 26 North, Route 28 e
Mt. Silver Outside. Há tilesets e encontros selvagens diurnos/noturnos da
fonte. A Route 26 North aponta para a Route 22 que já existia em Kanto, sem
importar cópias de Route 22, Route 23 ou Indigo Plateau de Johto. O desvio da
fronteira foi ajustado para produzir uma faixa compartilhada de 20 tiles;
o caminho físico nessa faixa ainda precisa de desenho e teste de caminhada.

Objetos, falas e eventos da fonte ficaram de fora; as duas portas de Tohjo
Falls mantêm somente as saídas de retorno à Route 27. Scripts vazios evitam
referências indefinidas. Mapas importados usam `world_enabled`, exigido pelo
gerador do porte PC para incluir Johto na tabela de mapas. A falta desse campo
fazia a câmera acessar um grupo vazio e fechar o jogo. O teste de execução com
save descartável abriu, posicionou 149 mapas externos e desenhou o atlas sem
exceção. O importador reexecutável está em
`tools/pokemon_go_world/import_johto_hns_foundation.py`.

## Expansão do terreno de Johto — 22 de setembro de 2026

O importador seletivo agora registra 53 mapas/áreas de Johto com layouts,
24 tilesets e 79 entradas de encontros selvagens da mesma revisão de
`pokehns-expansion`. Inclui o restante da malha externa de cidades e rotas,
Ruins of Alph, Ilex Forest, National Park, Lake of Rage, Mt. Silver e áreas da
Safari Zone. Não traz a segunda Kanto, nem Azalea Town, Route 32 ou Route 33.
As três áreas reservadas também não aparecem na tabela de recursos, no atlas
da câmera ou na tabela de encontros. Warps para destinos omitidos são removidos
e os restantes são renumerados para não apontarem a saídas incorretas.

O alinhamento com Hoenn continua provisório. A auditoria geométrica indica
interseção de Route 33 com Route 114 (20 tiles) caso a rota reservada seja
importada na posição atual; Route 34 e Ilex Forest já importadas também
intersectam Routes 114/115. Esses mapas fornecem o terreno para inspeção, mas
não representam uma decisão final sobre a posição mundial nem liberam a
campanha de Johto. NPCs e eventos narrativos permanecem fora desta importação.
A compilação PC com verificação do pacote passou (15.140 recursos, 989 mapas),
e um teste de execução com save descartável abriu e desenhou o atlas da câmera
livre sem fechar o jogo. Falta testar manualmente a caminhada pelas novas
bordas e decidir o reposicionamento geográfico antes de ativar os três mapas
reservados.

## Ajuste espacial de Johto — 22 de setembro de 2026

Azalea Town, Routes 32/33 e as portarias da Ilex Forest e de Ruins of Alph
entraram no conjunto seletivo: 60 mapas/áreas de Johto, sem duplicar Kanto.
O importador agora interpreta corretamente os índices de warp escritos como
texto na fonte e preserva 62 saídas válidas entre mapas incluídos. Ilex
Forest usa portarias em ambas as pontas, sem ligação direta de borda à Route
34. Johto foi projetada 30 blocos ao norte em relação ao encaixe provisório
com Hoenn. A travessia direta Route 22–Route 26 North foi suspensa até existir
um corredor real que preencha o vão; não há teletransporte substituto.

A auditoria encontra zero sobreposições entre os mapas externos ancorados de
Johto e Hoenn. Ruins of Alph foi 10 blocos para oeste, como solicitado, mas
ainda cruza Violet City e Route 32 em pequena área; a correção total precisa
de um segundo ajuste. Áreas externas sem conexão direta no grafo, como a
Safari Zone e partes de Mt. Silver, ainda precisam de âncora visual. A
campanha de Kanto não foi promovida a completa apenas pela presença de mapas:
404 mapas da região seguem desativados, com eventos e identidades narrativas
que precisam de validação própria antes de uma liberação geral.

A auditoria preliminar `tools/pokemon_go_world/audit_kanto_campaign.py`
explica por que não foi acionada uma chave global de história nesta etapa:
nos scripts desses 404 mapas aparecem 311 nomes de flags, dos quais 281 estão
definidos como `0` no conjunto compartilhado, incluindo
`FLAG_DEFEATED_BROCK`; 50 nomes de variáveis e 246 nomes de treinadores não
constam do cabeçalho principal de Hoenn. A contagem é textual e precisa ser
confirmada conforme cada arco for portado, mas basta para impedir uma ativação
segura em massa. O próximo trabalho narrativo deve reservar identidades
persistentes de Kanto, adaptar treinadores e progressão, então ativar e testar
cada arco até a Liga sem misturar recompensas e saves de Hoenn.

O pacote atualizado foi compilado em uma cópia isolada com 15.170 recursos,
verificação de integridade aprovada e teste automático da câmera concluído
sem encerramento inesperado. O jogo principal estava aberto e o Windows
bloqueou a substituição de `pokemon_regionalidades.pak` e do executável no
diretório principal. A cópia de teste está em `build/johto-preview-stage`;
o jogo principal permanece intacto até poder receber a atualização.

## Primeiro capítulo de Pewter — 22 de setembro de 2026

Pewter City, seu Ginásio, Centro Pokémon e Mercado foram promovidos de prévia
para eventos ativos no porte PC. Uma lista de eventos `pc_events` mantém as
fontes originais de FireRed intactas para aquele build e habilita apenas os
objetos e portas já adaptados no jogo unificado. As três portas ativas da
cidade têm retorno validado; museu e casas continuam fechados até receberem
eventos próprios. Route 3 e Mt. Moon ainda são terreno de prévia, sem capítulo
narrativo ou treinadores ativados.

Liam e Brock usam IDs de treinador regionais de Kanto, preservando o espaço de
Hoenn. A vitória contra Brock e a entrega da TM39 têm flags persistentes
separadas; a insígnia de Hoenn não é concedida por esse evento. Falta integrar
a exibição e os efeitos da Boulderbadge na interface regional. A compilação
PC, a integridade do pacote e a verificação dos 996 mapas passaram. Também foi
conferida estaticamente a ida e volta dos warps desse pequeno capítulo. A
batalha, cura e compra ainda precisam de uma rodada de teste manual no jogo.
