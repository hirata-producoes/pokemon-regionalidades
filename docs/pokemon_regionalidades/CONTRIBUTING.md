# Como contribuir

Contribuições podem ser técnicas, artísticas, documentais ou de testes. Pessoas iniciantes são bem-vindas; o importante é descrever com clareza o que foi alterado e como a mudança foi verificada.

## Como ajudar sem programar

Relatos de experiência também são contribuições. É possível ajudar percorrendo
Hoenn gradualmente, revisando textos em português do Brasil, comparando um evento
com o jogo de referência, conferindo créditos ou discutindo decisões de interface
e progressão. Um comentário geral pode orientar uma investigação, mas somente um
relato reproduzível deve ser tratado como validação de correção.

Ao compartilhar o projeto em fóruns ou redes sociais, deixe claro que Hoenn é a
base jogável em validação e que as outras regiões ainda não são campanhas
completas. Não anuncie ROM, instalador ou lançamento público que o repositório não
oferece.

## Antes de começar

1. leia o [README principal](../../README.md) e o [STATUS.md](STATUS.md);
2. confirme se a tarefa pertence ao projeto ou à base upstream;
3. para alterações grandes, registre primeiro o objetivo e a abordagem em uma issue;
4. não inclua ROMs comerciais, saves pessoais ou material sem autorização.

## Fluxo recomendado

Crie uma branch com um nome explicativo:

```powershell
git switch -c feature/nome-da-funcionalidade
```

Faça mudanças pequenas e relacionadas entre si. Um bom commit explica o resultado:

```text
Adiciona transição entre Hoenn e Kanto
Corrige carregamento do save no alvo PC
Documenta o formato do pacote de recursos
```

Antes de enviar:

- compile o alvo afetado;
- use o save de exploração para sistemas gerais e um save narrativo autêntico para eventos de história;
- confira `git diff` e `git status`;
- atualize a documentação quando o comportamento público mudar;
- informe limitações conhecidas.

Não reinicie a campanha inteira a cada alteração nem produza capturas em massa sem uma finalidade de diagnóstico. Consulte [Estratégia de desenvolvimento e testes](TESTING_STRATEGY.md).

## Estilo de documentação

- use português do Brasil como idioma principal e escreva de forma clara;
- mantenha textos novos para o jogador em português do Brasil; traduções entram pela futura camada de localização;
- preserve identificadores técnicos em inglês quando forem herdados ou compartilhados com projetos upstream;
- explique a sigla ou termo técnico na primeira ocorrência;
- use exemplos executáveis quando ajudarem;
- diferencie fato validado, hipótese e planejamento;
- evite emojis, linguagem promocional e afirmações sem evidência;
- não apague o contexto histórico de decisões antigas.

Scripts PowerShell que exibem texto com acentos devem ser mantidos em UTF-8 com
BOM e testados também no Windows PowerShell 5.1 (`powershell.exe`). UTF-8 sem BOM
pode ser interpretado como ANSI nessa versão e até transformar pontuação em
delimitadores inválidos.

## Organização do código

O namespace interno `pokemon_go_world` é legado da fase anterior. Não faça uma substituição global: scripts, includes e saves podem depender desses nomes. Novos nomes públicos devem usar `pokemon_regionalidades`; mudanças internas serão feitas por migrações controladas.

Ao reutilizar código de outro projeto, registre:

- repositório e autor;
- versão ou commit;
- arquivos adaptados;
- licença aplicável;
- diferença entre o original e a adaptação.

## Relatando problemas

Inclua:

- sistema operacional;
- commit testado (`git rev-parse --short HEAD`);
- alvo PC ou GBA;
- passos para reproduzir;
- resultado esperado e resultado observado;
- log ou captura de tela, sem dados pessoais.

Antes de anexar um log, confira se ele não contém nome de usuário do Windows,
caminhos pessoais ou localização de saves particulares.

## Pull requests

Uma pull request deve ter escopo compreensível e explicar os testes realizados. Alterações na base `pokeemerald-expansion` que não sejam específicas deste projeto também podem ser candidatas a uma contribuição no repositório upstream.
