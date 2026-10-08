# Referência de equipe, bolsa e resumo — P17a

Publicado como recorte de investigação F0.3, não implementação dos aplicativos.
Referência pokeheartgold fixada em 9d8b7591f09b65804da2fb2dfd56f320633e0d36,
conforme o auditor de referência já publicado em P16.

O manifesto HGSS_APPLICATION_REFERENCE.json registra hashes, 38 entradas de
despacho da bolsa, 23 do resumo, 27 recursos da equipe e diretórios de quatro
arquivos NARC. Números de estados não recebem significados inventados; chamadas
Assembly não comprovam alcance nem execução de todos os caminhos.

Ferramentas:
- audit_hgss_applications.py: inventário reproduzível com --reference e --verify.
- test_hgss_application_state_native.py: extrai funções da referência local e
  compila teste em memória de cursores da bolsa e movimento do painel da equipe.
- tests/test_hgss_applications.py: seis testes do parser e formatos.

Verificação na árvore de publicação em 08/10/2026: seis testes passaram; fixture
C passou, incluindo separação campo/batalha, escopo do reset e deslocamento do
painel. Não abriu jogo, não validou interface PC e não comprova ciclo inteiro
de recursos. Esse restante de P17 fica reservado, sem declarar F0.3 publicada
por completo. Arquivos gráficos/ROM, executáveis e saves não estão neste lote.

Reprodução: executar as ferramentas acima apontando --reference para a cópia
local fixada. O teste nativo usa o compilador Windows da pasta irmã toolchains,
como a bancada P16. Não redistribui as funções extraídas no arquivo gerado.
