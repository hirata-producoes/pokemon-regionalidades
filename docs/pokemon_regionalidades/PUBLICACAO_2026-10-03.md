# Publicação seletiva — 03/10/2026

Solicitada por Rafael em04/10/2026: somente o lote preparado mais antigo ainda
pendente, com autoria e registro em03/10/2026, America/Cuiaba. Consulta remota
atualizada: ponta8a5cac75ed; nenhum commit dessa data encontrado.

Comparação por equivalência de patches: cbdaca20c0 corresponde a539e9e817a,
portanto Viridian não é republicada. Dois commits locais têm conteúdo exclusivo:
2a5fb228d6 (consolidação multirregional) e9177bbbfcd (Route3/validação).
O primeiro é reaplicado na árvore de publicação sem reescrever o remoto.
Correções posteriores de perfis preservadas pelo merge da aplicação.
O script antigo publish_daily_commit.ps1 não integra o lote publicado;
a regra diária vigente prevalece sobre o registro antigo trazido pelo lote.

Este conteúdo antecede a reconstrução HGSS e inclui a interface provisória
daquele momento; publicar não significa aprová-la como interface definitiva.
Não contém o desenvolvimento F0/F1/F2.1 posterior nem comprova essas fases.
Sem executáveis ou saves pessoais. A data atribuída não é a data de criação
de todos os arquivos nem a data do envio ao GitHub.

Verificações desta preparação: sete testes lexicais passaram; teste de perfis
passou com saves sintéticos em pasta temporária do workspace e sem abrir o jogo.
Primeira tentativa fora dessa pasta falhou por permissão; repetição permitida
passou. Diff sem erros de whitespace. Não houve nova compilação ou gameplay;
essa verificação não certifica todo o runtime histórico deste lote.

Após este lote, resta um commit histórico preparado,9177bbbfcd. As alterações
posteriores da árvore de desenvolvimento ainda não estão divididas em uma fila
com número definitivo. Reservar commits por etapa F e dividir etapas grandes,
respeitando um único commit por data e autorização para cada publicação.
