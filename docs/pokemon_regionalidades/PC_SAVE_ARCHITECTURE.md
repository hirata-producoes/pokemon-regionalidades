# Arquitetura de save nativo para PC

## Decisão

O save principal de Pokémon Regionalidades no PC será um contêiner próprio,
versionado e extensível. O save Emerald de 131.072 bytes será preservado dentro
dele como um bloco de compatibilidade, mas deixará de definir o tamanho máximo
da campanha.

Essa migração deve acontecer antes da integração jogável de Kanto, Johto e
Sinnoh. Adiar a separação faria sistemas multirregionais dependerem dos setores
fixos do cartucho e aumentaria o custo de migração e de novos testes no futuro.

O alvo GBA continua usando o formato legado. Ele serve como referência de
regressão do núcleo de Hoenn, mas não limita o save do produto para PC.

No PC, as consultas e alterações feitas pelo jogo já usam diretamente o
inventário nativo depois do carregamento. As estruturas antigas permanecem como uma projeção de
compatibilidade verificada separadamente antes de cada gravação. Dessa forma,
uma divergência não é escondida nem gravada silenciosamente.

## O que `u16` significa

`u16` é apenas um inteiro sem sinal de 16 bits, com valores de 0 a 65.535. Ele é
útil também em computadores modernos quando essa faixa representa corretamente
o dado. Sua presença não é, isoladamente, uma limitação do GBA.

O problema aparece quando o tamanho de um tipo faz parte de uma estrutura
gravada de forma crua. Hoje `ItemSlot.quantity` usa `u16` e várias estruturas são
distribuídas em setores fixos de 4 KiB. Trocar todos os `u16` por tipos maiores
deslocaria os campos seguintes, alteraria checksums e invalidaria saves, scripts
e código que depende desses layouts.

Por isso, o projeto separará três decisões:

| Decisão | Exemplo | Tipo indicado |
|---|---|---|
| regra visível de gameplay | máximo de 99.999 unidades por item | `u32`, com limite lógico de 99.999 |
| contador técnico duradouro | geração do save e relógio acumulado | `u64` |
| identidade ou integridade | hash do arquivo | sequência de bytes, por exemplo SHA-256 |

Um inteiro de 256 bits não traz vantagem para inventário, dinheiro, mapas ou
progresso. Ele ocupa 32 bytes por valor, não possui operações nativas comuns e
exige conversões especiais. `u32` comporta mais de 4,29 bilhões e `u64`, mais de
18 quintilhões; ambos oferecem ampla margem para os sistemas planejados. Um hash
de 256 bits continua apropriado para integridade, mas não como tipo padrão de
contadores do jogo.

## Formato lógico do contêiner

O arquivo nativo terá um cabeçalho pequeno e um diretório de blocos. Números
multibyte terão ordem de bytes definida pelo formato, e não serão obtidos
gravando diretamente uma estrutura C inteira.

```text
save nativo
├── cabeçalho: assinatura, versão, geração e diretório
├── LEGACY: imagem Emerald original de 128 KiB
├── WORLD: progresso global e regional
├── ROTOMDEX: observações mundiais e módulos regionais
├── INVENT: inventário nativo e quantidades de 32 bits
├── GEAR: roupas, bicicletas e variações cosméticas
├── TIME: relógio, calendário e avanço offline
└── blocos futuros: regiões, missões e sistemas ainda não criados
```

Cada bloco terá:

- identificador estável;
- versão própria de esquema;
- posição e tamanho de 64 bits;
- checksum;
- indicação de obrigatório ou opcional.

Uma versão nova migra apenas os blocos que conhece. Blocos opcionais
desconhecidos são preservados sem alteração ao regravar, permitindo que uma
ferramenta compatível não apague silenciosamente dados de uma atualização mais
recente. Um bloco obrigatório desconhecido impede o carregamento e apresenta
uma mensagem clara em vez de tentar interpretar dados incorretos.

### Formato 1 implementado

O cabeçalho ocupa 40 bytes e contém a assinatura `PGRSAVE`, versão do
contêiner, tamanho do cabeçalho, geração de 64 bits, quantidade e tamanho das
entradas, CRC32 do diretório e CRC32 do próprio cabeçalho. Cada entrada ocupa 48
bytes: identificador de oito bytes, versão de esquema, flags, posição, tamanho
armazenado, tamanho lógico e CRC32 do conteúdo.

O formato 1 possui um bloco obrigatório `LEGACY`, versão 1 e tamanho exato de
131.072 bytes. O arquivo inicial com apenas esse bloco possui 131.160 bytes.
Offsets e tamanhos são codificados explicitamente em little-endian; o programa
não grava uma estrutura C crua e, portanto, não depende do alinhamento ou da
arquitetura do compilador.

O primeiro bloco expansível implementado é `WORLD`, esquema 1. Em vez de copiar
as matrizes de capacidade limitada do `SaveBlock3`, ele possui um cabeçalho de
32 bytes seguido por registros de oito bytes para cada marco narrativo ou
recompensa concluída. Cada registro guarda tipo, região e identificador de 32
bits em little-endian. O tamanho cresce apenas com o progresso real e novos
identificadores não exigem deslocar estruturas antigas. O programa recusa um
`WORLD` obrigatório com versão, assinatura, tipo, região, identificador,
duplicata ou tamanho que não reconheça, mesmo se os checksums externos tiverem
sido recalculados.

No PC, o esquema 1 reserva 4.096 marcos narrativos por região e 1.024
recompensas globais ou por região. A representação em memória custa cerca de
2,7 KiB; no arquivo, posições ainda não utilizadas não ocupam espaço. O espelho
legado conserva somente os primeiros 128 marcos e 64 recompensas para o alvo GBA
e para a migração de saves antigos.

## Compatibilidade e migração

O carregador seguirá esta ordem:

1. reconhecer e validar o contêiner nativo;
2. migrar sequencialmente cada bloco conhecido até a versão atual;
3. na ausência do contêiner, reconhecer um `.sav` Emerald válido de 131.072
   bytes e importá-lo como `LEGACY`;
4. criar o contêiner ao lado da origem, sem sobrescrever a importação;
5. confirmar uma gravação somente depois de validar cabeçalho, diretório,
   tamanhos e checksums.

O arquivo ativo e as três recuperações passam a proteger o contêiner inteiro.
A escrita continua usando arquivo temporário e substituição atômica.

Importar e exportar deixam de ser operações simétricas:

- **exportação nativa:** conserva toda a campanha e é o formato recomendado para
  transferência entre computadores;
- **importação legada:** converte um save Emerald para o formato nativo;
- **exportação legada:** só será oferecida como exportação de compatibilidade e
  avisará quando sistemas exclusivos do PC não puderem ser representados.

Nunca será prometida uma conversão de volta ao GBA que descarte silenciosamente
regiões, inventário ou progresso mundial.

## Estratégia para o inventário

O limite atual de 999 não existe porque um `u16` seja pequeno: 999 cabe com
folga em 16 bits. Ele também está codificado nas telas, na quantidade de dígitos
e nas rotinas de compra, venda, descarte e armazenamento.

O inventário nativo usará quantidades `u32`, com limite de gameplay inicialmente
definido em 99.999. O limite lógico ficará separado do tipo armazenado, portanto
poderá ser ajustado depois sem alterar novamente o formato.

A migração não será feita aumentando `struct ItemSlot` dentro do bloco legado.
O bloco `INVENT` se tornará a fonte oficial no PC; uma ponte de compatibilidade
manterá somente a parte representável no núcleo legado enquanto os menus e APIs
forem migrados. Isso evita deslocar estruturas antigas e permite revisar cada
operação com testes específicos.

## Custo no computador

O custo relevante não é o número de bits, mas a clareza do esquema e a segurança
da migração. Mesmo que os novos blocos somem vários megabytes, leitura e gravação
serão pequenas para um computador mediano. Recursos grandes, como mapas, áudio e
gráficos, continuam no pacote externo; o save guarda apenas estado mutável.

O contêiner deve impor limites máximos por bloco e no arquivo total para detectar
corrupção ou arquivos maliciosos. Liberdade de expansão não significa aceitar
tamanhos sem validação.

## Etapas de implementação

### Fase 1 — Contêiner sem mudança de gameplay

- [x] implementar leitura, validação e escrita do cabeçalho e diretório;
- [x] guardar o save atual integralmente em `LEGACY`;
- [x] converter cópias dos dois perfis sem alterar as origens;
- [x] adaptar recuperação, importação, exportação e informações da interface;
- [x] testar save novo, importação legada, duas gerações, checksum inválido,
  preservação de bloco opcional futuro e recuperação;
- [x] validar manualmente a primeira abertura, gravação e reabertura de cada
  perfil real.

Estado: concluída em 9 de setembro de 2026. Os perfis 1 e 2 foram abertos,
gravados e reabertos normalmente. Seus contêineres e primeiras recuperações
nativas passaram novamente pela validação estrutural e de checksums; os `.sav`
de origem permaneceram intactos.

### Fase 2 — Estado multirregional nativo

- [x] criar a API genérica para consultar, acrescentar e atualizar blocos sem
  reconstruir o formato em cada sistema;
- [x] definir o esquema 1 de `WORLD` e espelhar nele o progresso regional e as
  recompensas reais ao salvar no PC;
- [x] unir `WORLD` e o estado legado ao carregar, preservando qualquer avanço
  monotônico existente em apenas um dos lados;
- [x] tornar `WORLD` a fonte oficial e retirar a capacidade futura desses dados do
  espaço apertado de `SaveBlock3`;
- [x] mover a máscara inicial de módulos regionais do RotomDex para `ROTOMDEX`;
- [x] importar os dados já existentes sem apagar nem sobrescrever os perfis;
- [x] manter adaptadores temporários para o alvo GBA e saves legados.

`ROTOMDEX` possui agora um esquema obrigatório reconhecido pelo executável e
pelas ferramentas de perfis. Ele guarda cumulativamente os módulos regionais
liberados, une o valor legado durante a migração e nunca substitui uma região já
obtida. `INVENT` também possui seu primeiro esquema obrigatório e é a autoridade
do inventário durante o gameplay no PC. Os identificadores `GEAR` e `TIME`
continuam reservados e só serão obrigatórios depois que seus esquemas forem
implementados. Blocos opcionais desconhecidos continuam preservados sem alteração.

No PC, `WORLD` é a fonte oficial desses dados. O programa consulta e altera o
estado nativo, unindo no carregamento qualquer progresso monotônico ainda
presente somente no bloco legado. Os identificadores que cabem no formato antigo
também são espelhados em `SaveBlock3`; esse espelho é uma ponte de compatibilidade
e não limita mais a capacidade da campanha nativa.

Além dos testes unitários, uma cópia isolada da campanha-base foi salva, reaberta
e salva outra vez pelo executável real. O segundo ciclo preservou `WORLD` e as
recuperações do contêiner. Nesse estado concreto, seis registros ocuparam apenas
80 bytes. Depois disso, os dois perfis pessoais também foram gravados e reabertos
normalmente com o bloco `WORLD` presente. A ampliação e os maiores identificadores
permitidos pelo esquema receberam testes automáticos separados.

Uma sessão técnica isolada também gravou e reabriu o conjunto `LEGACY`, `WORLD`
e `ROTOMDEX`. O terceiro bloco tem 24 bytes, valida versão, quantidade de regiões,
bits permitidos e campos reservados antes de a campanha ser aceita.

### Fase 3 — Inventário amplo

- [x] definir e validar o esquema 1 de `INVENT`, preservando compartimento,
  posição, item e quantidade de cada espaço;
- [x] gravar e reabrir o primeiro retrato nativo em uma sessão isolada, sem
  alterar ainda o gameplay dos perfis pessoais;
- [x] tornar `INVENT` a fonte oficial no PC para o subconjunto já representável
  pelo gameplay, preservando a imagem legada como projeção de compatibilidade;
- [x] usar `u32` nas funções centrais de consultar, adicionar e remover itens,
  mantendo por enquanto o limite legado;
- [x] separar o espaço persistido Emerald (`LegacyItemSlot`) do valor usado pelas
  operações em memória (`ItemSlot` com quantidade de 32 bits), com verificações
  de tamanho que protegem o layout antigo;
- [x] manter um espelho nativo vivo durante as alterações de mochila e PC,
  incluindo os fluxos que limpam ou restauram matrizes completas;
- [x] recusar a gravação se o espelho vivo divergir do inventário legado durante
  a etapa de transição;
- [x] elevar o limite de gameplay do PC para 99.999 por posição;
- [x] revisar mochila, PC, lojas, coleta, scripts, descarte e telas numéricas;
- [x] testar na codificação os limites 0, 999, 1.000, 99.998, 99.999 e a
  rejeição de 100.000;
- [x] carregar e gravar 99.999 pelo executável real em uma cópia isolada e
  confirmar que a projeção Emerald não reduz o bloco nativo;
- [x] impedir que a validação de uma Poké Bola leia um alvo de equipe ausente;
- [x] validar manualmente o lançamento, a captura, a apresentação e o desconto
  de 99.999 para 99.998, confirmando depois o valor no bloco `INVENT`;
- [x] retirar a conversão obsoleta de quantidade para um campo de tarefa de 16
  bits durante a ordenação da mochila;
- [x] corrigir e adicionar um caso de regressão para a compactação do PC de
  itens quando uma pilha é retirada por inteiro;
- [x] validar manualmente descarte, depósito, retirada parcial, retirada
  integral e compactação visual do PC de itens com cinco dígitos;
- validar manualmente compra e venda acima de 999.

O esquema reserva seis compartimentos — as cinco partes da mochila e o PC de
itens —, até 4.096 posições em cada um e quantidades de 0 a 99.999. Os registros
de 16 bytes são esparsos, portanto posições vazias não aumentam o arquivo.
`INVENT` é a fonte do gameplay no PC; `LEGACY` recebe uma projeção limitada a
999 por posição para conservar o formato e o alvo GBA.

O cabeçalho possui um marcador explícito de autoridade. O valor inicial indica
“espelho legado” e exige que as duas representações coincidam integralmente ao
carregar. Esses saves são promovidos para autoridade nativa na gravação seguinte.
Quando o valor é “nativo”, `INVENT` alimenta diretamente as operações do gameplay.
A projeção Emerald conserva o item e limita somente sua quantidade espelhada a
999; a quantidade completa continua no bloco nativo e é conferida antes de cada
gravação. Posições ainda inexistentes nas estruturas atuais continuam sendo
recusadas, nunca descartadas silenciosamente.

As assinaturas centrais da mochila e do PC de itens agora recebem e calculam
quantidades em 32 bits. A mochila da Battle Pyramid permanece explicitamente
legada e recusa pedidos acima de `u16`; ela não é confundida com o inventário
normal. No PC, menus e operações normais usam agora o teto de 99.999; no GBA, os
mesmos símbolos continuam resolvendo para 999.

O novo caso de regressão em `test/bag.c` foi compilado com sucesso. A execução
da suíte herdada não chegou ao teste porque a montagem completa parou antes em
`test/pokemon.c`, com deslocamentos ARM grandes demais; essa limitação da
infraestrutura de testes é separada dos builds PC e GBA, que continuam passando.

### Fase 4 — Novos sistemas

- adicionar equipamentos cosméticos, estado mundial, missões e dados regionais
  como blocos independentes;
- cada bloco nasce com versão, migração, limites e testes;
- nenhuma nova região volta a depender de espaço livre nos setores do GBA.

## Critério para começar outra região

Kanto, Johto ou Sinnoh só entra como campanha jogável depois que a Fase 1 estiver
concluída e validada. A Fase 2 deve preceder a primeira progressão regional real.
A Fase 3 pode ser desenvolvida em Hoenn antes da integração de conteúdo, pois
Hoenn fornece todos os fluxos necessários para testar o inventário sem misturar
falhas de formato com falhas de uma região nova.

O lançador isolado de inventário preserva a sessão já criada por padrão. A
opção `-Recreate` é a única que volta a copiar e preparar a origem; isso impede
que uma reabertura comum pareça uma falha de persistência ao substituir
deliberadamente o arquivo de teste.
