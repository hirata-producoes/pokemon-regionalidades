# Porte nativo para PC

Este alvo compila diretamente o código decompilado do jogo para Windows/SDL2. Ele não executa uma ROM dentro do mGBA.

## Estado da implementação

- Base preservada: Pokémon GO World em `feature/pc-port`.
- Versão encontrada no repositório: `expansion/1.16.3-69-g84a694709-dirty` (mais nova que a 1.13 inicialmente mencionada).
- Referência: `gradenGnostic/pokeemerald-multiplatform`, commit `2f35335eff69ea1a08ebf19e64ea4d97ff6c0a05`.
- Porte original compilado e inicializado no Windows.
- Camada SDL2, renderer, BIOS, DMA e áudio CGB integrados na base Expansion.
- Todo o código C, mapas, scripts, gráficos e músicas do projeto passam pela compilação nativa.
- Primeiro executável Windows ligado com sucesso: `pokemon_go_world-pc.exe`.
- Teste de fumaça concluído: o processo permaneceu estável durante 8 segundos.
- O executável mede 36.155.607 bytes (aproximadamente 34,48 MiB), portanto o alvo PC já não está preso ao limite de ROM de 32 MiB.
- Áudio MP2K, multiboot e recursos específicos do hardware GBA estão temporariamente isolados por stubs; multiboot não fará parte do alvo PC e o áudio será substituído pela implementação nativa.
- Próximo marco: confirmar visualmente a sequência de abertura e chegar ao primeiro mapa.

## Como compilar

No PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_pc.ps1
```

O script usa a toolchain WinLibs i686 e o SDL2 instalados ao lado do repositório. Os caminhos podem ser sobrescritos pelos parâmetros `-ToolchainRoot`, `-SdlRoot` e `-PythonPath`.

O alvo interno também está disponível como `make pc` quando `make`, GCC i686, SDL2 e Python já estiverem configurados no ambiente.

## Sobre 32 bits e tamanho

O executável continua sendo de 32 bits nesta fase porque scripts e tabelas do jogo armazenam ponteiros de 32 bits. Isso não impõe o limite de ROM de 32 MiB. O executável do PC pode crescer muito além disso e, numa etapa posterior, os recursos serão carregados de um pacote externo.

## Próximos marcos

1. ~~Compilar todos os objetos do Expansion.~~
2. ~~Linkar o primeiro executável Pokémon GO World para PC.~~
3. Confirmar a tela inicial e chegar ao primeiro mapa.
4. Validar controles, vídeo, áudio, RTC e save.
5. Validar batalhas, animações e menus do Expansion.
6. Introduzir o pacote externo de recursos.
