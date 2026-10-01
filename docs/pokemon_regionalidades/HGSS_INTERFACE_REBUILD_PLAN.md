# Reconstrução da interface em duas telas — plano de execução

Versão 1 — 01/10/2026. Direção solicitada por Rafael: reconstruir a interface
com HeartGold/SoulSilver como referência principal, preservando o jogo único
de PC. **Este documento é planejamento, não declaração de implementação.**

O acompanhamento operacional completo permanece na árvore local de desenvolvimento.
Este lote seletivo está registrado em [PUBLICACAO_2026-10-01_02.md](PUBLICACAO_2026-10-01_02.md).
Este plano substitui a sequência dos planos anteriores de tela secundária.
`DS_SCREEN_ARCHITECTURE.md` passa a descrever o protótipo anterior, rejeitado
como solução definitiva após os novos relatos. Código existente não é aprovação.

## 1. Resultado pretendido e limites

Reconstruir apresentação, navegação, animações, recursos, texto e relação entre
as duas telas. Portar partes verificadas do código de interface HGSS quando
viável; adaptar serviços dependentes de DS para PC. Não colocar um segundo
motor de combate, uma segunda equipe ou outro sistema de saves dentro da GUI.

Preservar mapas, campanha, progresso, IDs, itens, espécies e perfis pessoais.
"Do início" significa iniciar a reconstrução da interface: não reiniciar a
história de Johto ou remodelar seus mapas neste trabalho. Defeitos de contexto
que afetem a interface, como a localização incorreta no Fly, entram na auditoria.

Decisões já dadas pelo usuário:

- HGSS é a referência principal de funcionamento e organização.
- HUD de nome/gênero/nível/HP/EXP fica na principal; comandos e golpes embaixo.
- Pokémon/base/HUD do jogador devem ocupar a região inferior útil da tela
  principal, e não continuar presos à antiga linha dos comandos de Emerald.
- Retirar "O que vamos fazer?"; usar ícones de tipo, não nomes extensos.
- Foco e confirmação devem ter estados/animação do componente original;
  não reutilizar a borda laranja acrescentada ao protótipo.
- Fonte deve ser legível, consistente e com acentos corretos; qualidade não
  fica limitada à resolução do console. Não comprimir frases até caberem.
- Não misturar, num fluxo migrado, novo desenho com transições/diálogos visuais
  de Emerald. A reutilização das regras não exige reutilizar suas telas.

Platinum e as folhas anexadas continuam como recursos secundários identificados.
Um componente de Platinum só entra como adaptação registrada e coerente, não
como substituto silencioso de um componente HGSS ausente. Funções DS que nosso
jogo não possui não viram botões inoperantes ou funcionalidades inventadas.

## 2. Regras de execução: como não pular partes

1. Uma fase principal em andamento. A próxima começa após o critério de saída
   da atual, salvo trabalho estritamente necessário num pré-requisito identificado.
2. Cada parte tem entrada, saída, dependências, cenários de teste e evidência.
   "Compila" ou "tem um print bonito" não é critério suficiente de fechamento.
3. Fechar um módulo não é fechar todo seu fluxo. As dependências externas
   ficam apontadas por ID e fase; não podem desaparecer do acompanhamento.
4. Parte incompleta não chega ao executável de uso normal como definitiva.
   Desenvolvimento e capturas usam build separado e perfis de QA isolados.
5. Não apagar a implementação anterior antes de termos substituição validada
   e uma cópia local recuperável do estado inicial, incluindo não rastreados.
6. Nada de corrigir um efeito visual acrescentando um terceiro desenho por cima.
   Localizar o proprietário da cena, seu estado e o dado/recurso esperado.
7. Mudança de conceito, exclusão de funcionalidade ativa ou adaptação material
   fora desta direção precisa de decisão explícita; silêncio não significa aceite.
8. Se um contrato fechado falhar depois, reabrir a parte responsável, corrigir
   a causa e repetir os testes dependentes. Não perseguir o sintoma em cada tela.

### Estados do acompanhamento

`Pendente` → `Em execução` → `Pronta para integrar` → `Em validação` → `Concluída`.
`Aguarda dependência` identifica a parte e o pré-requisito; não é "concluída".
`Reaberta` identifica regressão posterior. Revisão visual técnica, gameplay
e aceite do usuário são evidências distintas, registradas sem substituição.

Todo fechamento registra: versão/build, dados/perfil de QA, testes executados,
capturas dos estados relevantes, limitações e próximo ID permitido.

## 3. Dependências e marcos

| Fase | Entrega | Pré-requisito para começar | O que pode ser considerado fechado |
| --- | --- | --- | --- |
| F0 | Inventário e referência verificável | Plano vigente | Escopo/material/baseline, não interface |
| F1 | Contratos das duas telas e da ligação com o jogo | F0 | Arquitetura e estados especificados |
| F2 | Recursos, fonte, componentes e infraestrutura | F1 | Base comum testada, não aplicativos |
| F3 | Palco/HUD/mensagens e seleção de batalha | F2 | Núcleo de apresentação/seleção |
| F4 | Equipe e resumo | F3 | Seleção/troca/resumo; ações dependentes da bolsa continuam abertas |
| F5 | Bolsa e ações de itens na equipe | F4 | Ciclo bolsa/equipe completo |
| F6 | Batalha integrada, variantes e regressões | F3 + F4 + F5 | Primeiro fluxo jogável completo |
| F7 | Exploração e menu de acesso | F6 | Entrada/saída dos aplicativos |
| F8 | Contexto de localização, mapa/atlas/Fly | F7 | Navegação/localização/destinos |
| F9 | Pokédex | F8 | Aplicativo e consultas regionais |
| F10 | Perfil, opções, save e demais entradas inventariadas | F9 | Aplicativos restantes do escopo |
| F11 | Auditoria final, desempenho e retirada do protótipo | F0–F10 | Migração final, com evidências |

O ciclo equipe ↔ bolsa é dividido por responsabilidade: F4 entrega a seleção
e o resumo usados pela bolsa; F5 entrega os pedidos de item usados pela equipe.
Não exigir o serviço futuro para fechar o recorte interno F4 e depois alegar
que ele foi implementado. O conjunto só fecha em F5 e a batalha completa em F6.

Marcos: M1 = F2 base comum; M2 = F6 batalha completa; M3 = F10 aplicativos;
M4 = F11 substituição final. **F3 não é M2; número de testes não substitui marco.**

## 4. Fases e partes executáveis

### F0 — levantar a referência e proteger a base

**Objetivo:** saber exatamente o que vamos substituir e de quais recursos
dependemos, antes de decidir recortes ou copiar código.

- **F0.1 Baseline recuperável:** inventariar alterações locais, registrar hashes
  de executável/pacote/assets, preservar fontes alteradas e não rastreadas.
  Separar evidências antigas de capturas da reconstrução. Não usar reset/clean
  nem encerrar a sessão pessoal para obter o baseline.
- **F0.2 Inventário do projeto:** listar aplicativos, estados, modos de batalha,
  entradas/saídas, callbacks/controladores, dados e caminhos de desenho ativos.
  Incluir diálogos, trocas, uso de item, seleções obrigatórias e erros; não só
  o estado principal bonito. Identificar o que é funcionalidade ativa e o que
  é opção de compilação sem uso no PC.
- **F0.3 Referência HGSS:** obter uma revisão identificada por commit e mapear
  módulos e dependências. Registrar cada trecho como portado, adaptado ou novo.
  Mapear animações, paletas, coordenadas, textos e arquivos de recursos pelos
  dados reais. Prints de folhas não demonstram sequência/timing de animação.
- **F0.4 Cobertura dos materiais:** relacionar cada componente aos estados
  normal/foco/pressionado/desabilitado, animação e contexto. Identificar arquivos
  ausentes e formatos que precisam de conversão. Pedir material apenas quando
  a lacuna for demonstrada; não usar recorte de outro jogo para escondê-la.
- **F0.5 Catálogo inicial de problemas:** reproduzir relatos em ambiente de QA
  quando possível; guardar contexto, passos e esperado. Classificar hipótese
  separadamente de causa comprovada. O roteiro inicial está no acompanhamento.

**Saída:** inventário com nenhuma entrada ativa sem destino no plano; referência
fixada, cobertura de materiais, baseline recuperável e fila de problemas.
Material essencial ausente bloqueia a parte correspondente; não bloqueia a
auditoria independente, nem autoriza inventar um equivalente definitivo.

### F1 — especificar o contrato antes do desenho

**Objetivo:** definir quem controla cada tela em todos os estados e como isso
se conecta ao nosso jogo, sem acoplar o novo desenho aos BGs de Emerald.

- **F1.1 Matriz de telas:** para exploração, ações, golpes, alvo, mensagens,
  bolsa, equipe, resumo e retorno, definir conteúdo superior/inferior, animações
  que continuam, simulação pausada ou ativa, foco e efeito de cancelar. A regra
  superior não será simplesmente "sempre congelar o último framebuffer".
- **F1.2 Contrato de aplicação:** entrada, carregamento, ativa, transição,
  espera, confirmação, saída e retorno ao chamador. Incluir seleção obrigatória,
  falha de recurso e troca de contexto. Um dono de entrada e um resultado
  explícito; clique não pode confirmar também no menu que acabou de abrir.
- **F1.3 Contrato de dados/ações:** distinguir dados canônicos, valores exibidos
  durante animação e estado transitório de interface. Fazer tabela de IDs HGSS
  → IDs locais e ligar às regras atuais; não copiar tabelas de espécie/item/golpe
  nem renumerar saves. Prever número variável de combatentes/alvos, não fixar
  a arquitetura em apenas dois Pokémon.
- **F1.4 Geometria PC:** usar a composição HGSS como matriz de referência,
  especificar coordenadas lógicas, proporção, escala física, limites, fontes e
  transformação de mouse. 320×180 antigo não é requisito. Não alongar glifos
  ou todo o framebuffer; decidir recomposição do palco e áreas úteis antes
  de aplicar novos deslocamentos a atores/bases/HUD.
- **F1.5 Decisões visuais:** componentes HGSS, usos excepcionais de Platinum,
  fonte comum em melhor resolução e adaptações para quatro regiões/itens modernos.
  Desenhar composições de todos os estados principais, não só uma tela inicial.

**Saída:** matrizes de estado/dados/entrada/geometria sem lacunas nas entradas
inventariadas. Adaptações significativas precisam estar decididas. Protótipos
visuais não viram código de produção; valores exatos vêm desta fase, não deste plano.

### F2 — construir a base comum de apresentação

**Objetivo:** resolver as dependências compartilhadas antes dos aplicativos.

- **F2.1 Pipeline de recursos:** preservar originais; converter/identificar
  sprites, molduras, paletas e animações; gerar metadados de fonte/ID/estado,
  transparência, dimensões e créditos. Validar índice, recorte e integridade.
- **F2.2 Texto:** uma tabela explícita de Unicode/glifos, verificada visualmente
  contra a folha; uma regra de métricas para medir e desenhar. Cobrir Ç/ç,
  Ã/ã/Õ/õ, agudos/circunflexos, descidas p/g/j/y, gênero e pontuação. Espaçamento
  e espessura não podem depender de arredondamento diferente em cada tela.
  Textos extensos usam layout/quebra/rolagem/truncamento previsto, não achatamento.
- **F2.3 Componentes:** estados normal, foco, pressão, indisponível e animação.
  Separar movimento de seleção da confirmação. Um relógio de apresentação
  determinístico, com pausa definida; reproduzir sequências verificadas da
  referência, sem substituir foco por um retângulo extra.
- **F2.4 Infraestrutura:** compositor por tela, entrada por dono, navegação,
  pilha/retorno, carga/liberação, cache e tratamento de falha de recursos.
  A camada de compatibilidade adapta serviços; não desenha telas Emerald no
  meio de um aplicativo novo. Criar build de desenvolvimento separada.
- **F2.5 Bancada de componentes:** galeria rotulada de todos os estados,
  animações e amostras de texto na mesma escala da execução final. Fixtures
  apenas nesta bancada; recursos/dados simulados não entram no jogo final.

**Testes:** identidade dos acentos, linha de base, I/i/l/W e espaços, escalas,
mouse versus teclado/controle, cancelamento durante animação, carga repetida
e falha de recurso. Amostras: "ESTAÇÃO", "MANHÃ", "OPÇÕES", "REGIÃO",
"POKÉMON", "INÍCIO", "ÁGUA", nomes máximos e misturas de caixa.

**Saída:** todas as amostras representam os caracteres corretos; componentes
completos de estado e infraestrutura testada. Não iniciar telas para compensar
erro de fonte/escala/foco que pertence a esta base.

### F3 — reconstruir palco, HUD e núcleo da batalha

**Objetivo:** fechar a apresentação principal e o diálogo de ações/golpes.

- **F3.1 Palco:** separar cenário, bases, atores, efeitos, HUD e mensagens.
  Recompor a área inteira: não repetir a faixa neutra provisória para fingir um
  cenário completo. Posicionar conjunto do jogador embaixo conforme F1; preservar
  proporção, ancoragem, camadas, deslocamentos e efeitos, inclusive nas mensagens.
- **F3.2 HUD:** aplicar recursos/layout aprovados, identidade correta, gênero,
  status, HP e EXP animados. Fonte e símbolos comuns; números do oponente não
  devem expor informação que o motor não disponibiliza ao jogador.
- **F3.3 Ações/golpes:** menus inferiores completos, PP/tipo/categoria e slots
  vazios, navegação, foco/pressão, indisponibilidade e B/retorno. Retirar a frase
  rejeitada. O resultado passa ao controlador real, que decide validade/consumo.
- **F3.4 Mensagens/transições:** entrada na batalha, espera, execução, recusa,
  cancelamento, fim e retorno ao campo. Não alternar posições da arena conforme
  a presença dos antigos comandos; evitar reutilizar textura de outro contexto.

**Testes:** um golpe válido/inválido, PP zero, tipo variável, dano/cura/EXP,
status, vitória/derrota/fuga no recorte disponível e cenários grama/água/caverna.
Verificar os estados em movimento, não somente seus últimos quadros.

**Saída:** núcleo de apresentação e seleção integrado às regras reais. Entradas
bolsa/equipe ainda dependem de F4/F5: identificadas apenas no build de desenvolvimento.
Não distribuir esse recorte nem chamá-lo de batalha completamente migrada.

### F4 — equipe e resumo como aplicativos completos no seu recorte

- **F4.1 Equipe:** substituir o aplicativo visual, não cobrir suas janelas.
  Grade/componentes da referência, ícones/paletas corretos, nomes, gênero,
  HP/status/nível/item/ovo, vagas vazias e navegação nos estados inventariados.
- **F4.2 Seleção e troca:** entrada do campo e da batalha, troca voluntária e
  obrigatória após KO, restrições, confirmação/cancelamento e reorganização.
  Diálogos e feedback pertencem à nova apresentação; regras continuam canônicas.
- **F4.3 Resumo:** páginas, estatísticas, movimentos, detalhes disponíveis,
  navegação entre membros e retorno preservando seleção/chamador.
- **F4.4 Dependência de item:** definir pedido/resposta usado pela futura bolsa.
  "Dar/trocar item pela bolsa" fica `Aguarda F5`, não falso botão funcional.

**Saída:** equipe → resumo → equipe e seleção/troca funcionam em execução real
no recorte; ações dependentes de itens seguem rastreadas. Não migrar a aparência
da bolsa para contornar ausência de seu controlador nesta fase.

### F5 — bolsa e fechamento do ciclo de itens

- **F5.1 Aplicativo:** bolsos reais do nosso jogo, lista/quantidade/rolagem,
  ícones, descrição, navegação e entrada de campo/batalha. Categoria HGSS é
  mapeada ao modelo local: não alterar inventário para caber num desenho DS.
- **F5.2 Operações:** usar, registrar, dar/tirar/trocar, descartar, ordenar e
  confirmações nos contextos ativos. Itens sem alvo, com alvo, inválidos,
  esgotados e de captura passam pelas mesmas regras de uso atuais.
- **F5.3 Interligação:** bolsa → equipe → resultado → bolsa/batalha, inclusive
  cancelamentos, recusas, texto e mudança de quantidade/HP/status/espécie.
  Fechar F4.4 com dados reais, sem duplicar operações em widgets.
- **F5.4 Transições:** eliminar a animação visível de troca de bolso Emerald
  do caminho novo. Não mascarar com um painel estático enquanto ela executa.

**Saída:** ciclo de itens completo por mouse/teclado/controle, sem apresentar
estado ou diálogo do protótipo. Toda ação ativa listada em F0 tem resultado,
cancelamento e retorno. Bolsa/equipe agora podem receber validação conjunta.

### F6 — validar e completar o fluxo inteiro de batalha

- **F6.1 Percurso completo:** começar batalha → ações → golpes/bolsa/equipe/
  resumo → usar/cancelar/recusar → executar → próximo turno → fim → campo.
- **F6.2 Variantes:** implementar alvos, combatentes/HUD adicionais, duplas e
  modos especiais efetivamente ativos no PC, conforme F0. Gimmicks e opções
  modernas recebem adaptação explícita; não ficam fora por falta de moldura.
  Link/replay/tutoriais só entram se identificados como recurso ativo do produto.
- **F6.3 Casos-limite:** KO e seleção obrigatória, captura, fuga impedida,
  troca impedida, ausência de PP, subida de nível/aprendizado e entradas
  transitórias. Conferir que não consomem duas ações nem liberam cancelar quando proibido.
- **F6.4 Revisão/estabilidade:** comparar estados/animações com a referência,
  executar navegação repetida, verificar limpeza e medir custo do novo fluxo.

**Saída M2:** batalha migrada nas variantes do escopo e aceita tecnicamente,
com evidência de gameplay e revisão visual apresentada. Problemas de aparência
fundamental ou transição reabrem F2–F5; não avançar ao mapa para adiá-los.

### F7 — exploração e menu de acesso

Substituir o dashboard improvisado pela organização auditada/aprovada da
referência. Integrar local/tempo/clima ao espaço previsto, sem cartões extras
inventados. Aplicativos de jornada que não existem no jogo precisam de decisão
de escopo, não de um simulador sem dados reais.

Partes: **F7.1** composição/foco; **F7.2** abertura/fechamento/atalhos e política
de pausa; **F7.3** retorno das aplicações concluídas e estados indisponíveis.
Entradas ainda não migradas ficam na fila de desenvolvimento, não são releases.

**Saída:** menu consistente e ligado aos aplicativos reais; campo e controle
do personagem não recebem a entrada reservada ao menu. Verificar entrada de
novo jogo/introdução/troca de mapa como regressão de propriedade, sem reescrever história.

### F8 — localização, mapa/atlas e Fly

- **F8.1 Resolvedor de localização:** região/mapa atual → âncora exterior/
  posição global → marcador. Auditar interiores, cavernas, espaços compartilhados
  e dados ausentes; não usar origem do perfil nem Slateport como destino genérico.
  Caso desconhecido deve ser explicitamente desconhecido, não uma cidade errada.
- **F8.2 Aplicativo:** mapas/regiões/camadas, legenda, posição e seleção segundo
  a referência, com adaptação das quatro regiões do nosso mundo.
- **F8.3 Fly:** destinos permitidos, confirmação/cancelamento, custo/regra atual,
  saída e chegada corretas. Separar posição do jogador da mira de seleção.
- **F8.4 Atlas:** integrar recursos existentes sem confundir mapa global com
  coordenadas locais/warps. Reutilizar a localização nas futuras áreas da Pokédex.

**Saída:** teste em cidade, rota, casa, Centro, caverna e área sem seção; regiões
disponíveis atravessadas com retorno correto. Sinnoh ainda sem campanha não
será apresentado como campanha validada; testar dados de contexto em bancada.

### F9 — Pokédex

Partes: **F9.1** catálogo/estados visto/capturado e seleção; **F9.2** páginas,
tipos/formas/dados disponíveis e som; **F9.3** modos regionais/nacional e busca/
filtros; **F9.4** áreas/localização usando F8 e retorno ao menu.

Um catálogo universal, projeções regionais; nenhum catálogo diferente criado
para cada tela. Não adicionar captura/visto ou desbloqueio para exibir uma página.

**Saída:** abrir/buscar/trocar modo/consultar/voltar preserva seleção e regras;
contagens, espécies/formas/áreas vêm dos dados reais. Funcionalidades ausentes
no motor são lacunas explicitadas e decididas, não botões sem efeito.

### F10 — perfil, opções, save e entradas restantes

Migrar **F10.1** perfil/insígnias/dados; **F10.2** opções e prévias; **F10.3**
salvar/carregar/confirmar/erro/voltar; **F10.4** entrada de texto e demais
aplicações/diálogos associados às duas telas identificados em F0.

Cada subparte fecha antes de iniciar a seguinte. Não ampliar para toda a
campanha, lojas ou PC de armazenamento silenciosamente: F0 distingue aplicativo
afetado pelo novo compositor de redesign extra. Nenhuma entrada ativa pode
ficar sem migração ou decisão de exclusão aprovada antes de F11.

**Saída M3:** aplicativos do escopo completos; texto comum e dados reais.
Persistência usa o serviço existente; testar gravação e leitura em perfis QA,
inclusive falha/cancelamento. Nenhum perfil pessoal é convertido automaticamente.

### F11 — fechamento final e retirada do caminho provisório

- **F11.1 Auditoria cruzada:** executar a matriz completa sem usar o protótipo,
  revisão de telas e transições, contextos regionais, novos/carregados e modos.
- **F11.2 Desempenho:** comparar baseline e novo build em mesma máquina/cenário;
  medir tempo por quadro, picos, carga, memória e texturas após ciclos repetidos.
  Definir limites a partir do baseline/F1 e investigar regressão antes de declarar
  otimização. Não decodificar folhas ou alocar recursos de tela por glifo/quadro.
- **F11.3 Limpeza:** remover desenho/entrada/transições e remendos do painel
  rejeitado só após substituir todos os seus consumidores. Preservar regras,
  testes válidos e documentação histórica; retirar testes que exigem o defeito.
- **F11.4 Distribuição:** verificar recursos/fontes, diretório limpo de instalação,
  primeira execução/retorno/carregamento e roteiro para Rafael. Atualizar estado
  vigente com o que foi realmente validado e o que exige decisão futura.

**Saída M4:** nenhum componente ativo depende da interface rejeitada; nenhuma
combinação nova/velha inesperada; fluxos/evidências completos e regressões sem
bloqueadores. Só então substituir a versão de uso normal, com jogo fechado
voluntariamente. Commit/publicação são ações separadas, quando autorizadas.

## 5. Como testar sem "consertar" uma parte que ainda não existe

Testes continuam durante o desenvolvimento, com alcance declarado. O que muda
é a interpretação do resultado, não a obrigação de testar.

| Momento | O que testar | O que esse teste NÃO prova |
| --- | --- | --- |
| Contrato definido | Estado/dados/resultado/propriedade | Desenho final ou fidelidade |
| Componente pronto | Glifo/recorte/métricas/estados/timing/hitbox | Fluxo de aplicativo |
| Módulo pronto | Dados reais + regras + entrada/saída local | Retorno por módulo futuro |
| Dependências integradas | Percurso completo e cancelamentos/erros | Todos os modos/regiões |
| Marco jogável | Gameplay + vídeo/capturas + comparação visual | Aprovação de partes fora do marco |

Exemplo: antes de F5, botão de item ainda não abrir a nova bolsa é ausência
registrada, não motivo para mostrar a bolsa Emerald atrás de um painel. Mas
um Ç errado em F2 é defeito real da base, não algo que se resolve com a bolsa.

Para cada comportamento inesperado:

1. Registrar estado/contexto, passos, esperado, observado e evidência.
2. Conferir o contrato e se todos os pré-requisitos daquele cenário existem.
3. Classificar: `defeito confirmado`, `ausência planejada`, `regressão`,
   `hipótese`, `divergência de referência` ou `decisão necessária`.
4. Ausência: vincular à parte futura; não corrigir visualmente e não marcar o
   cenário aprovado. Hipótese: investigar sem alterar arquitetura por suposição.
5. Defeito de contrato já implementado: corrigir na origem e criar regressão.
   Crash, corrupção de dados, clique duplicado e vazamento grave são tratados
   imediatamente mesmo antes do marco completo.
6. Repetir o cenário original e os dependentes afetados; registrar fechamento.

Uma execução sintética é rotulada como sintética. Gameplay usa o fluxo real ou
um save produzido pelo jogo; não fabricar flags de história para provar menus.
Screenshots finais não comprovam pressão/transição: capturar sequência/vídeo
ou estados intermediários. Não usar contagem de testes como prova de aparência.

## 6. Fechamento de qualquer fase

- [ ] Pré-requisitos concluídos e partes do recorte terminadas.
- [ ] Decisões/IDs/fonte/recursos rastreáveis; nenhuma adaptação silenciosa.
- [ ] Estados normais, inválidos, vazios, de erro e cancelamento cobertos.
- [ ] Dados reais e regras canônicas, sem lógica duplicada na GUI.
- [ ] Ambos os painéis e seus retornos coerentes nos cenários da fase.
- [ ] Testes adequados ao nível, capturas e revisão registrados.
- [ ] Falhas classificadas; nenhum bloqueador do recorte aberto.
- [ ] Dependências futuras listadas, sem afirmar fluxo final incompleto.
- [ ] Custos medidos quando pertinentes; carga/limpeza sem regressão conhecida.
- [ ] Próximo ID permitido indicado; se regressão, partes reabertas indicadas.

## 7. Realismo, mudanças de plano e duração

Não há prazo confiável antes de F0/F1: precisamos contar estados reais,
recursos ausentes, dependências DS e variantes ativas. Não prometer migração
inteira em um turno. Dividir trabalho em lotes pequenos compiláveis; limitar
o lote ao que pode receber sua validação correspondente.

Ao final de F0/F1, estimar esforço por parte e registrar riscos. Reestimar a
parte quando surgir dependência nova; não trocar de prioridade silenciosamente.
Fases de bolsa/equipe, variantes de batalha, localização e Pokédex têm maior
risco de integração que uma alteração isolada de botão. Otimização vem de
medição; fidelidade vem de referência e revisão, não de termos no relatório.

Mudança necessária: registrar motivo, evidência, contratos afetados, novos
pré-requisitos e fases reabertas. Se muda intenção/funcionalidade/design material,
pedir decisão a Rafael. Documento de execução e estado de retomada devem ser
atualizados juntos. Não manter dois planos concorrentes como vigentes.

## 8. Referências e pontos locais de partida

HGSS tem módulos próprios de entrada/animação de batalha e aplicativo de
equipe, mas depende de serviços DS; não é biblioteca PC pronta. Consultar
[battle_input.c](https://github.com/pret/pokeheartgold/blob/master/src/battle/battle_input.c),
[party_menu.c](https://github.com/pret/pokeheartgold/blob/master/src/party_menu.c)
e [INSTALL.md](https://github.com/pret/pokeheartgold/blob/master/INSTALL.md).
Fixar o commit e a cobertura restante em F0; links de branch são navegação,
não a versão auditada congelada.

Pontos locais a auditar, sem obrigatoriedade de reaproveitar implementação:

- Composição/entrada: `src/platform/sdl2.c`, `gba_easy_draw.c`, `pc_screen.c`.
- Protótipo visual: `secondary_battle_view.h`, `secondary_application_view.h`,
  `secondary_panel_layout.h`, `ds_battle_assets.h`.
- Texto/geração: `pc_ui_text.c`, `pc_hd_text.c`, `secondary_panel_font.h`,
  `prepare_common_fonts.py` e metadados da fonte.
- Dados/controladores: `pc_battle_panel.c`, `party_menu.c`, `item_menu.c`,
  `pokemon_summary_screen.c`, `region_map.c` e módulos de Pokédex.
- Materiais: `graphics/reference/ds_ui`, `graphics/pc_panel/sources`,
  `graphics/fonts/regionalidades/sources` e relatório técnico anexado.
- Verificação existente: `test_pc_screen_native.py`,
  `render_secondary_panel_preview.py`, `verify_ds_ui_assets.py`,
  `TESTING_STRATEGY.md`. Testes que preservam escolhas rejeitadas precisam ser
  reformulados, não usados para certificar o protótipo como referência correta.

## 9. Situação na criação do plano

Planejamento redigido; F0 ainda não executada integralmente. As fases F1–F11
não foram implementadas por este documento. Nenhum runtime, executável, pacote
ou save alterado neste turno. Próxima tarefa: **F0.1**, seguida de F0.2–F0.5.

Atualização de execução, 01/10/2026: F0.1/F0.2 concluídas no escopo de auditoria;
F0.3 em execução (referência comportamental registrada localmente),
sem liberar F1 antes da saída de F0.
A auditoria F0, a matriz de fluxos e o acompanhamento detalhado permanecem
na árvore de desenvolvimento, reservados para publicações futuras. A descrição
acima registra a criação do plano, não substitui o acompanhamento posterior.
