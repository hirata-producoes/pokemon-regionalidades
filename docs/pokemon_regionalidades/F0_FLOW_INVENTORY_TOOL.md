# Inventário de fluxos da interface — F0.2

Lote P15, data solicitada06/10/2026. Complementa o extrator lexical publicado
em02/10 com [escopo explícito](HGSS_UI_FLOW_SCOPE.json), auditor e testes.

[audit_ui_flow_inventory.py](../../tools/pokemon_go_world/audit_ui_flow_inventory.py)
classifica os catálogos de bolsa, equipe, resumo e menu, localiza entradas,
indexa estados/callbacks e registra hashes das fontes. Valores sem classificação,
duplicados, removidos e entradas não encontradas fazem a auditoria falhar.
Inclui ramos condicionais inativos: uma referência textual não demonstra que
o fluxo está acessível na configuração PC.

O recorte foi validado sobre a árvore publicada, que difere do desenvolvimento.
A primeira execução detectou IsPokedexPlusHGSSActive ausente nessa base.
O escopo publicado usa as entradas existentes CB2_OpenPokedex e
CB2_OpenPokedexPlusHGSS. A classificação local mais recente continua reservada
para o lote que publicar suas mudanças de runtime; não copiamos esse runtime
para satisfazer a auditoria.

```text
python tools/pokemon_go_world/audit_ui_flow_inventory.py --output build/ui-flow-inventory.json
python tools/pokemon_go_world/audit_ui_flow_inventory.py --verify build/ui-flow-inventory.json
python -m unittest discover -s tools/pokemon_go_world/tests -p test_ui_flow_inventory.py
```

[Inventário da árvore de publicação](evidence/2026-10-06-publication-flow-inventory/inventory.json)
registra a conferência deste lote. As fases no escopo descrevem destinos de
migração; não atestam implantação, alcance em runtime ou conclusão de F0 inteira.
O plano de reconstrução continua sendo a referência de implementação.
