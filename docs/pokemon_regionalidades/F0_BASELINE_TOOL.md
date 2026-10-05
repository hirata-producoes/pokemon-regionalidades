# Ferramenta de baseline recuperável — F0.1

Lote P14, data de commit solicitada05/10/2026; preparado em06/10.
Publica a ferramenta e os testes já desenvolvidos para registrar uma cópia
recuperável de fontes, recursos e runtime antes da reconstrução da interface.

[snapshot_ui_baseline.py](../../tools/pokemon_go_world/snapshot_ui_baseline.py)
oferece inventário, criação de ZIP/manifesto em diretório novo fora do projeto
e conferência de tamanho/SHA-256 de cada entrada. Detecta mudanças nas fontes
durante a captura e recusa o fechamento de um snapshot inconsistente.
Exclui saves, perfis, configurações pessoais, metadados privados e cache.
Histórico Git e ferramentas externas não fazem parte do ZIP.

Exemplo, a partir da raiz, usando Python3.11 ou posterior:

```text
python tools/pokemon_go_world/snapshot_ui_baseline.py --inventory
python tools/pokemon_go_world/snapshot_ui_baseline.py --destination ../baseline-nova
python tools/pokemon_go_world/snapshot_ui_baseline.py --verify ../baseline-nova
python -m unittest discover -s tools/pokemon_go_world/tests -p test_ui_baseline_snapshot.py
```

Restauração deve ocorrer em diretório vazio. O arquivo ZIP é um artefato local;
não deve ser adicionado automaticamente ao GitHub. Este lote não inclui saves,
executáveis, arquivos de máquina ou o backup completo do desenvolvimento.

Os três testes usam arquivos sintéticos para conferir escopo, exclusões e
detecção de corrupção. Passaram na árvore de publicação isolada. A publicação
da ferramenta não declara a interface reconstruída nem valida gameplay.
