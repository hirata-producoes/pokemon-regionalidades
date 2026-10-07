# Referência HGSS e feedback de batalha — lote P16

Publicação preparada em 07/10/2026 a partir do trabalho de investigação existente.
O conteúdo é de referência e ferramentas; não implementa uma segunda tela no port.
A data de publicação não é apresentada como data de criação de todos os arquivos.

Referência: pret/pokeheartgold, revisão
`9d8b7591f09b65804da2fb2dfd56f320633e0d36`.

## Conteúdo reutilizável

- Auditor de identidade e diretórios NARC, com validação de limites dos membros.
- Leitor LZ10 e metadados de sequências NANR: duração, modo e offsets.
- Manifesto com hashes de 271 arquivos selecionados e índices de três arquivos
  NARC; não redistribui seus pixels nem comprova cobertura de toda a interface.
- Evidência das quatro sequências do cursor, com quatro quadros de seis ticks
  por sequência; não equivale à reprodução visual no PC.
- Bancada C que extrai as funções de feedback da revisão original e exercita
  disponibilidade, origem da entrada, botões, golpes e cancelamento com serviços
  gráficos simulados. Não executa SDK/renderizador nem modifica saves.

Dependência já publicada: `tools/pokemon_go_world/ui_flow_source.py`.
O checkout HGSS é externo e precisa estar limpo, na revisão fixada. Python usa
somente biblioteca padrão nestes testes. A bancada nativa usa o GCC Winlibs
i686 instalado em `../toolchains/winlibs-i686-r4-tar/mingw32/bin/gcc.exe`;
esse caminho é uma dependência do ambiente local, não instalação automática.

## Verificação do lote na árvore de publicação

Em 07/10: 12 testes unitários passaram; manifesto da referência e evidência do
cursor conferidos contra o checkout fixado; bancada C compilada e executada,
com oito combinações de disponibilidade e rastros de botões/golpes/cancelamento.
Nenhuma compilação integral ou nova validação de gameplay foi realizada.

Reprodução a partir da raiz do projeto, substituindo `<HGSS>` pelo checkout:

```text
python -m unittest discover -s tools/pokemon_go_world/tests -p "test_hgss*.py"
python tools/pokemon_go_world/audit_hgss_ui_reference.py --reference <HGSS> --manifest docs/pokemon_regionalidades/evidence/2026-10-01-rebuild/hgss-reference-lock.json
python tools/pokemon_go_world/audit_hgss_animation.py --reference <HGSS> --verify docs/pokemon_regionalidades/evidence/2026-10-01-rebuild/hgss-cursor-animation.json
python tools/pokemon_go_world/test_hgss_feedback_native.py --reference <HGSS>
```

Se o checkout pertence a outro usuário, a exceção Git safe.directory deve ser
restrita ao caminho e à execução; não tornar todos os diretórios confiáveis.

Equipe/bolsa/resumo e aplicativos auxiliares continuam nos lotes seguintes da
investigação. Este lote não conclui F0.3 inteiro, F2.1, conversão 3D ou integração.
O planejamento revisado e o trabalho recente de desenvolvimento ficam reservados
para publicações próprias; não foram misturados a este lote anterior.
