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

## Fontes primárias de conteúdo regional

- [pret/pokefirered](https://github.com/pret/pokefirered): Kanto e Ilhas Sevii;
- [pret/pokecrystal](https://github.com/pret/pokecrystal): Johto clássico;
- [pret/pokeheartgold](https://github.com/pret/pokeheartgold): Johto e Kanto em HeartGold/SoulSilver;
- [pret/pokeplatinum](https://github.com/pret/pokeplatinum): referência principal de Sinnoh em Platinum;
- [pret/pokediamond](https://github.com/pret/pokediamond): diferenças e conteúdo de Diamond/Pearl.

Essas reconstruções servem para conferir mapas, eventos, encontros e progressão. Código de Nintendo DS não é importado diretamente como código GBA ou PC.

## Demakes e referências de adaptação

- [Pokémon Heart & Soul](https://github.com/PokemonHnS-Development/pokemonHnS): Johto e Kanto recriados sobre Modern Emerald;
- [Pokémon Heart & Soul 2.0](https://github.com/PokemonHnS-Development/pokehns-expansion): versão baseada também em `pokeemerald-expansion`, prioritária para estudar integração;
- [documentação do Heart & Soul](https://github.com/PokemonHnS-Development/pokehns-expansion-documentation): inventário extraído de conteúdo, encontros, treinadores e progressão;
- [Pokémon Platinum Demake](https://github.com/sinnoh-remakes/pokeemerald-platinum): Sinnoh adaptada à estrutura de `pokeemerald-expansion`;
- [Sinnoh-pokeemerald-expansion](https://github.com/LiderMorti00/Sinnoh-pokeemerald-expansion): referência complementar de gráficos, tilesets e estrutura;
- [Pokémon World GBA](https://github.com/unsupo/Pokemon-World-GBA): estudo complementar de integração multirregional;
- [Modern Emerald](https://github.com/resetes12/pokeemerald): linhagem técnica do Heart & Soul original.

Esses projetos não são tratados como autoridades oficiais. Antes de importar qualquer parte, devem ser registrados o commit, a origem do arquivo, a cadeia de créditos, as diferenças em relação ao jogo de referência e as condições de reutilização. A presença pública de código no GitHub não deve ser interpretada automaticamente como licença irrestrita.

## Trabalho específico deste repositório

- integração progressiva do `pokeemerald-expansion` com um alvo nativo para PC;
- camada SDL2 e adaptações de plataforma;
- pacote externo de recursos;
- conteúdo e estrutura multirregional;
- documentação técnica e didática em português;
- compatibilidade entre nomes legados e a identidade Pokémon Regionalidades.

## Como registrar novas referências

Ao importar uma solução, acrescente o link, o autor, a licença e a versão ou commit utilizado. Se o código tiver sido modificado, descreva brevemente a adaptação. Isso facilita auditoria, atualização e aprendizado.

Uma sugestão recebida em fórum, issue ou rede social não autoriza automaticamente
a cópia de código ou recursos. Antes de incorporar uma contribuição, registre a
origem, confirme a licença e preserve a autoria adequada. Ideias gerais podem ser
discutidas publicamente; arquivos e implementações continuam sujeitos às suas
licenças e permissões próprias.

Os arquivos `CREDITS.md`, `FEATURES.md`, o `CHANGELOG.md` da base e a pasta `docs/` também contêm créditos detalhados de projetos upstream. O conteúdo herdado é mantido para preservar autoria e histórico.
