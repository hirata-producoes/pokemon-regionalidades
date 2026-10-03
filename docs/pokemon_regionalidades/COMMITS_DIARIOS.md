# Regra vigente de publicação

Um único commit por dia, no fuso America/Cuiaba. Conferir histórico local e
remoto atualizado antes de criar/enviar. Se já houver commit na data pedida,
parar e pedir confirmação. Não usar amend/rebase/force push para contornar.
Datas atribuídas a pedido não demonstram quando todo o conteúdo foi criado.
“Continue” não autoriza push e não se publica automaticamente em outro dia.

Direção de04/10/2026: reservar pelo menos um commit por etapa F desenvolvida;
etapas grandes devem ser divididas em lotes coerentes e testáveis. Isso não
autoriza declarar fases incompletas como concluídas nem criar datas extras.
Alterações ainda não divididas em lotes não têm contagem final de commits.

## Registro antigo, sem validade operacional

O texto abaixo descreve uma fila passada. Não executar seus comandos ou o
script mencionado. A fila vigente exige inspeção e autorização por publicação.

Os commits locais foram criados em 21/09/2026. O GitHub mostra a data gravada
em cada commit, nao a hora do `push`. Dois deles (`95b5cfa213` e `a35f3fcb2e`)
ja foram enviados em 22/09/2026 com a data original. Corrigir sua apresentacao
exigiria reescrever o historico remoto; este procedimento conserva os dois.

Os quatro commits ainda pendentes sao publicados por uma arvore de trabalho
separada. Cada publicacao recria apenas o proximo commit, com a data e a hora
reais daquele dia, preservando o conteudo e a mensagem. A arvore principal,
inclusive suas alteracoes nao salvas em commit, nao e modificada pelo processo.

Execute **somente um** destes comandos por dia, a partir de 23/09/2026, na
ordem abaixo. Eles funcionam mesmo quando o PowerShell estiver em `C:\Users\Rafael`:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\Users\Rafael\Documents\Codex\2026-08-02\pokemon-go-world-pc\tools\pokemon_go_world\publish_daily_commit.ps1" -Step 2
powershell -ExecutionPolicy Bypass -File "C:\Users\Rafael\Documents\Codex\2026-08-02\pokemon-go-world-pc\tools\pokemon_go_world\publish_daily_commit.ps1" -Step 3
powershell -ExecutionPolicy Bypass -File "C:\Users\Rafael\Documents\Codex\2026-08-02\pokemon-go-world-pc\tools\pokemon_go_world\publish_daily_commit.ps1" -Step 4
powershell -ExecutionPolicy Bypass -File "C:\Users\Rafael\Documents\Codex\2026-08-02\pokemon-go-world-pc\tools\pokemon_go_world\publish_daily_commit.ps1" -Step 5
```

O script consulta o GitHub antes de agir, compara o conteudo remoto com o
commit anterior e impede dois envios no mesmo dia. Em caso de divergencia,
ele para sem sobrescrever o remoto. Depois da serie, a linha local de
desenvolvimento e a linha publicada terao hashes diferentes para os mesmos
conteudos; elas devem ser reconciliadas antes do proximo `push` comum.
