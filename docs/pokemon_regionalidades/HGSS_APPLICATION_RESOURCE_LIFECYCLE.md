# Ciclo de recursos e métricas das fontes — P17b

Complemento da [referência P17a](HGSS_APPLICATION_REFERENCE.md), baseado na
revisão pokeheartgold `9d8b7591f09b65804da2fb2dfd56f320633e0d36`.
É investigação preservada, não implementação dos aplicativos nem aprovação
visual. Publicação preparada em 11/10/2026; data do commit atribuída a
10/10/2026 por solicitação do usuário.

## Fronteiras da sessão

- Bolsa: o chamador fornece contexto e inventário; a seleção retorna pela view.
  `Bag_Exit` libera os recursos da sessão, incluindo janelas, mensagens, NARC,
  callbacks e heap. A interface não deve aplicar novamente o efeito do item.
- Equipe: `PartyMenuApp_Init/Main/Exit` prepara e encerra renderer, janelas,
  strings, mensagens, fonte e heap. A configuração das engines DS não define,
  por si só, a tela lógica equivalente no PC.
- Resumo: os argumentos conservam origem, slot, modo e páginas habilitadas.
  Os NARC 39, 162 e 180 correspondem a `a/0/3/9`, `a/1/6/2` e `a/1/8/0`.
  Encerrar a sessão libera recursos; o chamador consome o resultado e pode
  reabrir a equipe preservando a seleção.

O inventário e hashes desses recursos já estão em
`HGSS_APPLICATION_REFERENCE.json` (P17a); não são republicados aqui.
Esses contratos orientam a adaptação, mas não provam todos os caminhos de
liberação, ausência de vazamentos ou funcionamento dos aplicativos no port.
Não encerram o restante de P17 nem a fase de integração.

## Fonte: IDs e avanços, sem renderer novo

[Manifesto de métricas](evidence/2026-10-01-rebuild/hgss-font-metrics.json):
seis membros de fonte (0, 1, 2, 3, 4, 10), 509 glifos por membro e 32 caracteres
verificados por fonte, incluindo acentos de português. Contém cabeçalhos,
avanços e hashes; não inclui bitmaps, arquivos NARC ou ROM.

O índice do glifo é o ID do caractere menos um. A largura vem da tabela
própria de cada fonte; não deve ser deduzida de uma grade de espaçamento fixo.
O auditor rejeita IDs inválidos, dimensões incompatíveis e dados truncados.
Métricas corretas não comprovam legibilidade, alinhamento, sombra ou aparência.

Ferramentas entregues:

- `tools/pokemon_go_world/audit_hgss_font.py`: leitura da referência fixada e
  conferência do manifesto com `--reference <pasta> --verify <manifesto>`.
- `tools/pokemon_go_world/tests/test_hgss_font.py`: testes sintéticos de limites,
  índices, avanços e truncamento.

Reprodução na raiz: `python -m unittest discover -s tools/pokemon_go_world/tests -p test_hgss_font.py`.
Depois executar o auditor com a cópia local limpa da revisão fixada e o
manifesto acima. A dependência `audit_hgss_ui_reference.py` já foi publicada em P16.
Nenhum teste deste lote abre o jogo ou acessa saves.
