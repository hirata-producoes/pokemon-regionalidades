# Créditos e referências

Este projeto existe porque diferentes comunidades documentaram, reconstruíram e ampliaram a arquitetura de Pokémon Emerald. Os créditos abaixo separam a base herdada do trabalho específico deste repositório.

## Coordenação do projeto

- **Hirata Produções**: direção do projeto Pokémon Regionalidades, definição de escopo, conteúdo regional e manutenção do repositório.

## Bases técnicas

- [pret/pokeemerald](https://github.com/pret/pokeemerald): descompilação de referência de Pokémon Emerald;
- [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion): sistemas, conteúdo e melhorias usados como base, na linha 1.16.3 desta árvore de código;
- [gradenGnostic/pokeemerald-multiplatform](https://github.com/gradenGnostic/pokeemerald-multiplatform): referência para a adaptação multiplataforma e o uso de SDL2;
- [SDL2](https://github.com/libsdl-org/SDL): janela, entrada, áudio e integração com o sistema operacional;
- [mGBA](https://github.com/mgba-emu/mgba): emulador de referência para validação do alvo GBA.

## Trabalho específico deste repositório

- integração progressiva do `pokeemerald-expansion` com um alvo nativo para PC;
- camada SDL2 e adaptações de plataforma;
- pacote externo de recursos;
- conteúdo e estrutura multirregional;
- documentação técnica e didática em português;
- compatibilidade entre nomes legados e a identidade Pokémon Regionalidades.

## Como registrar novas referências

Ao importar uma solução, acrescente o link, o autor, a licença e a versão ou commit utilizado. Se o código tiver sido modificado, descreva brevemente a adaptação. Isso facilita auditoria, atualização e aprendizado.

Os arquivos `CREDITS.md`, `FEATURES.md`, o `CHANGELOG.md` da base e a pasta `docs/` também contêm créditos detalhados de projetos upstream. O conteúdo herdado é mantido para preservar autoria e histórico.
