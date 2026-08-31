# Pokémon Regionalidades

Pokémon Regionalidades é um projeto de estudo e desenvolvimento de jogos que explora como transformar uma base criada para Game Boy Advance em uma experiência multirregional com versão nativa para PC.

O projeto começou sobre o `pokeemerald-expansion`, que por sua vez deriva da descompilação comunitária de Pokémon Emerald. A lógica de mapas, batalhas, menus, scripts e Pokémon continua compartilhada com o alvo GBA. No PC, uma camada SDL2 substitui serviços específicos do console, como vídeo, entrada, áudio, relógio e armazenamento.

> O projeto está em desenvolvimento. A fundação técnica funciona, mas o jogo mundial completo ainda não está pronto.

## Por que este repositório existe

Este trabalho tem três objetivos complementares:

- desenvolver uma aventura que conecte várias regiões em uma única jornada;
- estudar arquitetura de jogos, C, build systems, formatos binários e portabilidade;
- documentar decisões e resultados de forma útil para iniciantes, colaboradores e portfólio profissional.

Não é necessário dominar emulação ou programação de baixo nível para começar a ler. A documentação apresenta os termos técnicos quando eles se tornam necessários e aponta os arquivos responsáveis por cada parte.

## Estado atual

O porte para Windows já compila o jogo nativamente, renderiza por SDL2, aceita teclado e controle, salva, usa RTC, reproduz áudio MP2K e executa mapas, menus e batalhas. Milhares de recursos são lidos de `pokemon_regionalidades.pak` sob demanda.

Layouts de mapas já foram preparados para o pacote externo e compilam, mas sua validação completa em execução ainda está em andamento. Consulte [Estado do projeto](docs/pokemon_regionalidades/STATUS.md) para diferenciar protótipo, implementação e planejamento.

## Regiões

Hoenn é a base jogável atual. Kanto possui dados de FireRed/LeafGreen e uma primeira integração própria em Pallet Town. Johto, Sinnoh, Hisui, Unova, Kalos, Alola, Galar, Paldea e Ilhas Laranja fazem parte do planejamento, com definições de região e iniciais já preparadas, mas ainda não são campanhas completas.

Veja [Regiões e jogos de referência](docs/pokemon_regionalidades/REGIONS.md).

## Duas formas de executar

### Alvo nativo para PC

O programa é recompilado para Windows. Ele não abre uma ROM dentro de um emulador:

```text
código e dados do projeto
        |
        +-- compilador x86 ----> pokemon_regionalidades-pc.exe
        |
        +-- geradores ---------> pokemon_regionalidades.pak
```

O pacote externo pode crescer além do limite tradicional de 32 MiB da ROM GBA. Ele mantém um índice de 64 bits e carrega recursos quando são necessários.

### Alvo GBA

O alvo GBA continua disponível para estudo e compatibilidade. Ele produz `pokemon_regionalidades.gba` e continua sujeito às limitações reais do console. A ROM e os saves compilados localmente não são distribuídos neste repositório.

## Começando

- [Compilar no Windows](docs/pokemon_regionalidades/BUILDING_WINDOWS.md)
- [Compilar o alvo GBA](docs/pokemon_regionalidades/BUILDING_GBA.md)
- [Entender a arquitetura](docs/pokemon_regionalidades/ARCHITECTURE.md)
- [Consultar o roteiro](docs/pokemon_regionalidades/ROADMAP.md)
- [Aprender como contribuir](docs/pokemon_regionalidades/CONTRIBUTING.md)
- [Consultar créditos e referências](docs/pokemon_regionalidades/CREDITS_AND_REFERENCES.md)
- [Acompanhar o histórico próprio](PROJECT_CHANGELOG.md)

O processo completo está no [Relatório técnico do porte PC](docs/pokemon_go_world/RELATORIO_TECNICO_PORTE_PC.md). Ele preserva nomes e caminhos históricos quando isso é necessário para reproduzir etapas anteriores.

## Organização do código

| Caminho | Conteúdo |
|---|---|
| `src` | lógica do jogo e implementações de plataforma |
| `src/platform` | SDL2, renderização, BIOS, DMA, áudio e pacote externo |
| `include` | interfaces, tipos, constantes e configurações |
| `data` | mapas, scripts e dados montados |
| `graphics` e `sound` | recursos gráficos e de áudio |
| `resources/pc` | manifesto do pacote externo |
| `tools/pokemon_go_world` | ferramentas e geradores; o nome é legado |
| `docs/pokemon_regionalidades` | documentação atual |
| `docs/pokemon_go_world` | registros anteriores à troca de nome |

O namespace interno `PGW_*` e alguns diretórios antigos foram mantidos temporariamente. Renomeá-los de uma só vez produziria uma alteração ampla e difícil de revisar. Eles serão migrados gradualmente.

## Base e referências

- [pret/pokeemerald](https://github.com/pret/pokeemerald)
- [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion), versão-base 1.16.3
- [gradenGnostic/pokeemerald-multiplatform](https://github.com/gradenGnostic/pokeemerald-multiplatform), commit `2f35335eff69ea1a08ebf19e64ea4d97ff6c0a05`
- [SDL](https://github.com/libsdl-org/SDL)

Os créditos herdados permanecem em [CREDITS.md](CREDITS.md). As atribuições do porte estão em [THIRD_PARTY_NOTICES.md](docs/pokemon_go_world/THIRD_PARTY_NOTICES.md).

## Uso educacional e portfólio

O repositório pode ser estudado por temas: C, Git, compilação cruzada, Makefiles, abstração de hardware, formatos binários, hashes, CRC, relocações, carregamento sob demanda, testes de jogos e documentação técnica.

O código não deve ser tratado como exemplo perfeito ou produto concluído. Os documentos registram acertos, limitações, falhas encontradas e decisões provisórias porque esses elementos também fazem parte do aprendizado.

## Avisos legais

Pokémon Regionalidades é um projeto independente, não oficial e sem afiliação com Nintendo, Game Freak, Creatures Inc. ou The Pokémon Company.

Pokémon e propriedades relacionadas pertencem aos seus respectivos titulares. A presença de código e recursos de diferentes origens não significa que todo o conteúdo esteja sob uma única licença. Consulte [Avisos legais e distribuição](docs/pokemon_regionalidades/LEGAL.md) antes de redistribuir qualquer build.
