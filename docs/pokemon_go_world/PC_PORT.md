# Porte nativo para PC

Este alvo compila diretamente o código decompilado do jogo para Windows/SDL2. Ele não executa uma ROM dentro do mGBA.

## Estado da implementação

- Base preservada: Pokémon GO World em `feature/pc-port`.
- Versão encontrada no repositório: `expansion/1.16.3-69-g84a694709-dirty` (mais nova que a 1.13 inicialmente mencionada).
- Referência: `gradenGnostic/pokeemerald-multiplatform`, commit `2f35335eff69ea1a08ebf19e64ea4d97ff6c0a05`.
- Licença e atribuição da camada reutilizada: `docs/pokemon_go_world/THIRD_PARTY_NOTICES.md`.
- Porte original compilado e inicializado no Windows.
- Camada SDL2, renderer, BIOS, DMA e áudio CGB integrados na base Expansion.
- Todo o código C, mapas, scripts, gráficos e músicas do projeto passam pela compilação nativa.
- Primeiro executável Windows ligado com sucesso: `pokemon_go_world-pc.exe`.
- Tela de copyright, introdução, título completo com `PRESS START` e fluxo de `NEW GAME` confirmados visualmente no Windows.
- Primeiro mapa confirmado: o jogo conclui `CB2_NewGame`, entra em `CB2_Overworld` e renderiza o interior do caminhão inicial.
- Teste automatizado permaneceu estável por mais de 100 segundos, incluindo a criação do personagem e a entrada no mapa.
- Controles e renderização dinâmica confirmados: o personagem se movimenta no caminhão, atravessa a saída e chega a Littleroot com a primeira conversa da mãe renderizada.
- Save nativo validado em duas execuções: o jogo grava `pokemon_go_world.sav` com 131.072 bytes e apresenta `CONTINUE` carregando o mapa salvo após reiniciar.
- Backend SII RTC do PC validado contra a data e hora locais do Windows. O projeto continua respeitando `OW_USE_FAKE_RTC = TRUE`, portanto a jogabilidade usa o relógio persistido no save; se essa opção for desativada, o backend nativo segue o relógio do sistema e persiste seu deslocamento em `pokemon_go_world.cfg`.
- Áudio MP2K nativo integrado: sequenciador, mixer Direct Sound, cries e os quatro canais CGB geram áudio estéreo `float` a 42.060 Hz para a fila SDL2.
- O áudio foi validado com buffers não silenciosos durante a introdução e com uma regressão automatizada de 90 segundos que chegou ao menu de itens sem falha.
- Batalha selvagem de diagnóstico validada com Bulbasaur nível 50 contra Pikachu nível 45: transição, sprites, cries, HUD, menus, turnos, HP, sono e animações de `Thunder` e `Discharge` funcionaram no alvo PC.
- Pacote externo versionado integrado com índice e offsets de 64 bits, leitura sob demanda, cache por recurso, validação de limites, FNV-1a e CRC32. O formato e o fluxo de adição estão documentados em `docs/pokemon_go_world/RESOURCE_PACK.md`.
- Os 12 recursos usados pela tela de título Emerald foram removidos de fato do executável PC e carregados de `pokemon_go_world.pak`: gráficos comprimidos, tilemaps e paletas do logo, Rayquaza, nuvens, versão, brilho e banners. Suas definições originais continuam no alvo GBA.
- A configuração ativa gera automaticamente 2.950 imagens e 2.889 paletas externas de Pokémon, incluindo sprites frontais, traseiros, diferenças de gênero, formas e ovos. `gSpeciesInfo` continua selecionando os mesmos símbolos; somente o alvo PC os resolve no pacote.
- Imagens de Pokémon são lidas transitoriamente e liberadas logo após a descompressão, evitando que visitar muitas espécies faça o cache crescer indefinidamente.
- Paletas são validadas como blocos de pelo menos 32 bytes e mantidas no pequeno cache sob demanda. Imagem ou paleta ausente/corrompida usa a interrogação correspondente quando disponível; sem pacote, um bloco zerado seguro impede leitura inválida.
- A configuração ativa também gera 1.420 ícones e 1.031 pegadas externas. O menu de equipe foi validado carregando os ícones animados de Bulbasaur e Pikachu com 1.024 bytes cada; sem pacote, os espaços ficam vazios sem leitura inválida.
- O pacote válido, o pacote ausente e um pacote com dados corrompidos foram testados. Nos dois últimos casos o jogo continua de maneira segura, usando o fallback quando ele existir e sem entregar dados corrompidos ao descompressor.
- A batalha Bulbasaur contra Pikachu foi repetida com sprites externos. Um Pikachu propositalmente corrompido foi rejeitado pelo CRC32 e substituído visualmente pelo sprite de interrogação, sem afetar o sprite traseiro de Bulbasaur.
- Multiboot e recursos específicos de comunicação do hardware GBA permanecem isolados por stubs; multiboot não fará parte do alvo PC.
- Tela de resumo validada a partir do menu de equipe, incluindo sprite/paleta externos, dados, habilidade, descrição, memo e transição para renomear. Duas escritas de texto que alcançavam a borda do buffer receberam margens apenas no alvo PC; uma execução automatizada de 15 segundos terminou sem exceção.
- A configuração ativa gera 1.067 cries externos (7,74 MiB). O mixer os carrega e mantém em cache somente quando cada espécie toca; as tabelas montadas permanecem imutáveis e o GBA conserva as amostras compiladas originais.
- Batalha de regressão confirmou `Cry_Pikachu` e `Cry_Bulbasaur` vindos do pacote. Sem o pacote, ambos são omitidos com segurança e a batalha continua sem exceção.
- O executável agora mede 24.171.265 bytes (aproximadamente 23,05 MiB), o pacote com 9.369 entradas mede 12.985.552 bytes (aproximadamente 12,38 MiB) e `data/sound_data.o` caiu para aproximadamente 0,96 MiB no PC.
- Próximo marco: migrar tilesets e outras famílias grandes.

## Compatibilidades resolvidas para o primeiro mapa

- Flash/save do cartucho substituído por armazenamento nativo em memória e arquivo.
- Rotinas de descompressão que executavam código copiado para RAM adaptadas para execução direta, compatível com DEP/NX do Windows.
- Espelho de ROM usado pelo interpretador de scripts do GBA removido somente no alvo PC; o alvo GBA conserva a marcação original.
- Fila de DMA processada no host durante carregamentos síncronos, reproduzindo o progresso que o VBlank assíncrono realiza no hardware.
- Consultas de cópia de fundo também drenam a fila DMA no PC. Isso corrige inicializadores em laço síncrono, como a tela de equipe, que no GBA progridem pela interrupção de VBlank.
- Sondagem RFU desativada no PC e encerramentos de tela protegidos contra callbacks de um quadro já liberado.
- GPIO do RTC do cartucho substituído, somente no alvo portátil, por um backend SDL2 baseado no relógio do sistema.
- Persistência do flash conectada a um arquivo próprio do projeto, sem alterar o formato do save GBA.
- Driver ARM/DMA do MP2K substituído no alvo PC pelo sequenciador e mixer C reutilizados do porte multiplataforma, preservando os dados de músicas, instrumentos e cries do projeto.
- Layouts de 32 bits de instrumentos, tracks e canais mantidos e verificados para continuar compatíveis com as tabelas montadas do Expansion.
- Cópias DMA imediatas entre buffers normais convertidas em cópias de CPU no alvo PC; transferências GBA continuam inalteradas.
- Callbacks de VBlank/HBlank das transições são desligados antes de liberar os dados temporários, evitando uso após liberação entre o campo e a batalha.

## Como compilar

No PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_pc.ps1
```

O script usa a toolchain WinLibs i686 e o SDL2 instalados ao lado do repositório. Os caminhos podem ser sobrescritos pelos parâmetros `-ToolchainRoot`, `-SdlRoot` e `-PythonPath`.

O alvo interno também está disponível como `make pc` quando `make`, GCC i686, SDL2 e Python já estiverem configurados no ambiente.

## Controles atuais

- Direcional: setas.
- A e B: `Z` e `X`.
- Start e Select: `Enter` e `\`.
- L e R: `A` e `S`.
- Acelerar: `Espaço`.
- Reiniciar: `Ctrl+R`; pausar: `Ctrl+P`.
- Controle XInput: direcional/analógico, A, X, Start, Back, ombros e gatilho direito para acelerar.

## Sobre 32 bits e tamanho

O executável continua sendo de 32 bits nesta fase porque scripts e tabelas do jogo armazenam ponteiros de 32 bits. Isso não impõe o limite de ROM de 32 MiB. Recursos já podem ser carregados sob demanda de um pacote externo com offsets de 64 bits; tela de título, imagens, paletas, ícones, pegadas e cries de Pokémon já foram migrados.

## Próximos marcos

1. ~~Compilar todos os objetos do Expansion.~~
2. ~~Linkar o primeiro executável Pokémon GO World para PC.~~
3. ~~Confirmar a tela inicial e chegar ao primeiro mapa.~~
4. ~~Validar controles, vídeo, RTC e save.~~
5. ~~Implementar e validar o áudio MP2K nativo.~~
6. ~~Validar batalhas, cries, animações e menus do Expansion.~~
7. ~~Introduzir o pacote externo de recursos.~~
8. Migrar progressivamente gráficos, áudio e demais dados grandes, removendo suas cópias compiladas somente do alvo PC.
