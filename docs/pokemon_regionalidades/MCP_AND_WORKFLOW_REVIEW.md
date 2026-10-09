# MCPs e continuidade — avaliação de 11/10/2026

Escopo: ferramentas disponíveis nesta sessão, configuração local visível e fontes
oficiais dos projetos. Não houve medição de tokens, latência ou custo por plugin.
Nenhuma integração foi instalada, removida ou teve permissões alteradas.

## O que existe e o que foi comprovado

Há ferramentas MCP de GitHub, gerenciamento do aplicativo, revisão de código,
plugins e outros serviços na sessão. O GitHub foi usado com sucesso para ler o
README e comparar a ponta remota. A configuração local também declara node_repl.
Disponibilidade de uma ferramenta não prova uso contínuo, consumo por chamada
ou gargalo. Skills de documentos/planilhas/imagens são capacidades diferentes
de um servidor MCP; contar skills não mede custo de execução.

Superpowers, Linear, PostHog e Datadog apareceram disponíveis no catálogo, todos
com installed=false na consulta de 11/10. TeaRAG não apareceu nessa busca; o
catálogo não é exaustivo. O projeto de pesquisa foi identificado pela fonte abaixo.

Não há evidência suficiente para culpar um MCP específico. Os problemas
comprovados no acompanhamento foram: cronologia repetida em três arquivos,
instruções de retomada já superadas, índice parado em F1.5 e estado público
desalinhado do código local. Esses pontos foram corrigidos na revisão documental.
O histórico de pequenas entregas preparatórias também exige foco no próximo
resultado integrado, sem acrescentar testes por mera existência de recursos.

## Adequação ao projeto

| Ferramenta | Utilidade possível | Recomendação atual |
| --- | --- | --- |
| GitHub MCP | Conferir remoto, alterações, PRs e publicações | Manter para operações remotas; busca/edição do código local continuam diretas |
| Superpowers | Método de planejamento, depuração, testes e revisão | Não instalar automaticamente. O fluxo completo pode acrescentar planejamento, trabalho paralelo e ciclos de teste ao processo já existente; aproveitar princípios em uma tarefa delimitada antes de adotar o conjunto |
| Linear | Quadro de tarefas, responsáveis e dependências | Opcional se responsável quiser um quadro compartilhado. Usar como visualização dos IDs do plano, sem segunda fila concorrente ou cópia de todas as evidências |
| PostHog | Análise de uso, erros e experimentos a partir de dados instrumentados | Adiar: ainda falta terminar a interface local. Não substitui captura visual, depurador ou registro de sessão do jogo |
| Datadog | Consulta de métricas, logs e traces já enviados à plataforma | Adiar enquanto não houver operação/telemetria que justifique manutenção, coleta e custo adicionais |
| TeaRAG | Pesquisa de recuperação e compressão de contexto | Não integrar como requisito do jogo. É framework de pesquisa sobre FlashRAG, com modelos/índices e dependências próprias, não um plugin pronto que reduz a conta desta conversa |

As recomendações da última coluna são avaliação para este projeto, não resultados
de benchmark. A economia de cerca de60% relatada pelo TeaRAG pertence aos modelos
e experimentos dos autores; não pode ser transferida ao Codex ou ao nosso repositório.

## Melhoria imediata e verificável

Ler uma retomada curta e buscar o trecho necessário por arquivo/símbolo. Preservar
origem, receita, consumidor e evidência, sem inserir relatórios inteiros no chat.
Usar os testes estreitos existentes; o runner nativo já aceita --only. Uma falha
nova justifica ampliar a investigação, não reabrir toda F2.1. Atualizar status por
substituição e registrar execução uma única vez. Não repetir build por edição de
Markdown nem abrir o jogo para validar documentação.

Se houver suspeita concreta de lentidão de ferramenta, registrar a operação,
tempo/timeout e tamanho da resposta; comparar a mesma tarefa pelo caminho direto.
Somente então restringir ferramentas ou desativar uma integração identificada.
O Codex documenta enabled/disabled_tools e limites de saída por ferramenta;
existência desses controles não demonstra que uma configuração esteja cara.

## Fontes consultadas

- [MCP no Codex: configuração e limites de ferramentas](https://learn.chatgpt.com/docs/extend/mcp?surface=cli).
- [Superpowers: fluxo e integração](https://github.com/obra/superpowers).
- [Linear MCP](https://linear.app/docs/mcp).
- [PostHog MCP: implementação e capacidades](https://github.com/PostHog/posthog/blob/master/services/mcp/README.md).
- [Datadog MCP oficial](https://github.com/datadog-labs/mcp-server).
- [TeaRAG: pesquisa, instalação e resultados declarados](https://github.com/Applied-Machine-Learning-Lab/TeaRAG).

Fontes pesquisadas durante a revisão iniciada em08/10; disponibilidade do catálogo
reconferida em11/10. Não criar nova fase do jogo para experimentar essas ferramentas.
