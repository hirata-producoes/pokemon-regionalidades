# Como contribuir

Contribuições podem ser técnicas, artísticas, documentais ou de testes. Pessoas iniciantes são bem-vindas; o importante é descrever com clareza o que foi alterado e como a mudança foi verificada.

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
- execute o cenário alterado;
- confira `git diff` e `git status`;
- atualize a documentação quando o comportamento público mudar;
- informe limitações conhecidas.

## Estilo de documentação

- escreva em português claro;
- explique a sigla ou termo técnico na primeira ocorrência;
- use exemplos executáveis quando ajudarem;
- diferencie fato validado, hipótese e planejamento;
- evite emojis, linguagem promocional e afirmações sem evidência;
- não apague o contexto histórico de decisões antigas.

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

## Pull requests

Uma pull request deve ter escopo compreensível e explicar os testes realizados. Alterações na base `pokeemerald-expansion` que não sejam específicas deste projeto também podem ser candidatas a uma contribuição no repositório upstream.

