# Compilando a versão para Windows

Este guia mostra como gerar o executável nativo para PC. Você não precisa entender todo o sistema de compilação antes de começar; os detalhes técnicos aparecem ao lado das instruções práticas.

## O que será gerado

O alvo para Windows produz:

- `pokemon_regionalidades-pc.exe`: programa principal;
- `SDL2.dll`: biblioteca de janela, teclado, controle e áudio;
- `pokemon_regionalidades.pak`: pacote externo obrigatório de recursos;
- `pokemon_regionalidades.pgrsave`: save nativo extensível criado durante o jogo;
- `pokemon_regionalidades.cfg`: configuração local.

O executável não é uma ROM sendo aberta por um emulador. Ele é uma compilação nativa do mesmo código do jogo, adaptada para usar SDL2 no lugar do hardware do Game Boy Advance.

## Requisitos

- Windows 10 ou 11 de 64 bits;
- PowerShell;
- Python 3;
- compilador MinGW-w64 compatível;
- SDL2 para MinGW;
- ferramentas do projeto, incluindo `agbcc` e GNU Make.

O repositório não inclui toolchains completos nem ROM comercial. Consulte também [LEGAL.md](LEGAL.md).

## Compilação automatizada

Abra o PowerShell na raiz do projeto e execute:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_pc.ps1 -Jobs 4
```

O número em `-Jobs` indica quantas tarefas podem ser compiladas em paralelo. Em computadores mais modestos, use `-Jobs 2`.

Em checkpoints e antes de publicar uma versão, acrescente `-VerifyResources`:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_pc.ps1 -Jobs 4 -VerifyResources
```

Essa opção lê integralmente o pacote externo e confirma cabeçalho, índice, nomes, hashes, limites, alinhamento, checksums e correspondência com o manifesto. Ela também verifica grupos de mapas, IDs, layouts, conexões, destinos de warp e os recursos de layout externalizados. É propositalmente opcional porque a leitura de todos os recursos seria um custo desnecessário em cada compilação incremental.

Se as dependências estiverem em locais diferentes dos padrões usados pelo script, informe os caminhos explicitamente. Para consultar todos os parâmetros:

```powershell
Get-Help .\tools\pokemon_go_world\build_pc.ps1 -Detailed
```

## Execução

Depois de uma compilação bem-sucedida:

```powershell
.\pokemon_regionalidades-pc.exe
```

Controles de teclado atuais:

| Ação do GBA | Tecla no PC |
|---|---|
| Direcional | Setas |
| A | Z |
| B | X |
| Start | Enter |
| Select | Backspace |
| L / R | A / S |
| Acelerar enquanto estiver pressionado (5x) | Espaço |
| Pausar/continuar o programa | Ctrl + P |
| Reiniciar o jogo | Ctrl + R |

No controle XInput, o direcional ou analógico esquerdo movimenta o personagem, `A` confirma, `X` volta, `Start` abre o menu do jogo, `Back` funciona como Select, os botões superiores funcionam como L/R e o gatilho direito acelera enquanto estiver pressionado.

Esses são os padrões. O botão `Configurações` na tela de perfis permite remapear
o teclado e A, B, Start, Select, L, R e aceleração no controle XInput. Também
permite escolher aceleração de 2× a 10× e restaurar todos esses valores. Feche o
jogo antes de salvar; as opções são compartilhadas pelos dois perfis e entram em
vigor na próxima abertura. Volume e vídeo ainda não entraram nessa primeira tela.

Consulte também [CONTROLS.md](CONTROLS.md) para uma explicação voltada a quem está jogando, incluindo a diferença entre os botões do GBA e os botões adicionais de um controle moderno.

## Recursos externos

Por padrão, o programa procura `pokemon_regionalidades.pak` ao lado do executável. Também é possível apontar outro arquivo:

```powershell
$env:POKEMON_REGIONALIDADES_RESOURCE_PACK = 'D:\jogos\recursos\regionalidades.pak'
.\pokemon_regionalidades-pc.exe
```

O nome antigo `POKEMON_GO_WORLD_RESOURCE_PACK` continua aceito para não quebrar ambientes já configurados.

Como mapas, tilesets, áudio e outros dados já foram externalizados, o pacote é obrigatório. Se ele estiver ausente ou inválido, o programa encerra com uma mensagem clara em vez de continuar com cenários pretos ou recursos zerados.

## Compatibilidade com saves antigos

Quando o contêiner nativo ainda não existe, o programa reconhece
`pokemon_regionalidades.sav` ou o nome anterior `pokemon_go_world.sav`, preserva
o arquivo antigo e cria `pokemon_regionalidades.pgrsave`. A mesma regra de
preservação vale para o arquivo de configuração.

Mesmo com essa migração automática, mantenha cópias de segurança antes de testar versões em desenvolvimento.

## Save de exploração

Para explorar Hoenn sem refazer a campanha a cada teste, gere uma cópia de desenvolvimento:

```powershell
.\tools\pokemon_go_world\create_dev_mobility_save_pc.ps1 -Recreate
```

O gerador aceita `pokemon_regionalidades.pgrsave` ou um `.sav` Emerald da raiz
como origem, preserva esse arquivo e grava a cópia nativa em
`build/dev-save-exploracao`. A cópia inicia em Oldale e recebe cinco Pokémon de
Hoenn no nível 75, todos os HMs, as oito insígnias e todos os destinos de Fly de
Hoenn. Essa posição artificial serve apenas à exploração e não valida o
progresso da história.

Para abrir a cópia depois de criada:

```powershell
.\tools\pokemon_go_world\run_dev_mobility_save_pc.ps1
```

Esse recurso serve para mapas e sistemas gerais. Ele não altera posições de NPCs nem tenta simular eventos de história. Use a campanha normal ou saves gravados antes das cenas para validar progressão narrativa. Consulte [Estratégia de desenvolvimento e testes](TESTING_STRATEGY.md).

## Perfis pessoais no PC

A interface atual permite abrir dois perfis com saves independentes. Ela ainda
receberá acabamento e informações adicionais da campanha, mas já concentra as
operações funcionais de perfil e pode ser aberta com:

```powershell
powershell -ExecutionPolicy Bypass -File `
    .\tools\pokemon_go_world\open_player_profiles_pc.ps1
```

Ela permite abrir os dois perfis, importar um `.sav` Emerald ou `.pgrsave`
somente em um perfil vazio e escolher uma recuperação existente. Também há uma
ação `Exportar save`, que cria um `.pgrsave` portátil, valida sua estrutura e
checksums e confirma por SHA-256 a cópia escrita.
Se um arquivo exportado for substituído depois da confirmação do jogador, a
versão anterior recebe o sufixo `before-export` em vez de ser descartada.

Cada perfil pode receber um nome personalizado. `Reiniciar campanha` exige
confirmação, cria um perfil ativo vazio e move a pasta anterior inteira para uma
cópia de segurança datada ao lado dos perfis; nenhum save é apagado diretamente
por essa ação.

O botão `Criar atalho` adiciona à área de
trabalho um acesso direto a essa janela, sem exibir o terminal nas execuções
seguintes. Para diagnóstico ou automação, o lançador direto continua disponível
indicando `1` ou `2`:

```powershell
.\tools\pokemon_go_world\run_player_profile_pc.ps1 -Profile 1
.\tools\pokemon_go_world\run_player_profile_pc.ps1 -Profile 2
```

Os dados ficam em `%LOCALAPPDATA%\Pokemon Regionalidades`. A configuração de
vídeo, áudio e controles é compartilhada, mas cada perfil possui seu próprio
save e sua própria rotação de três recuperações.

Para transformar uma cópia da campanha-base no primeiro perfil, faça a
importação somente na primeira abertura:

```powershell
.\tools\pokemon_go_world\run_player_profile_pc.ps1 `
    -Profile 1 `
    -ImportSave '.\build\baseline-hoenn\pokemon_regionalidades.pgrsave'
```

O arquivo de origem é preservado. Se o perfil já possuir um save, o lançador
cancela a importação em vez de substituí-lo. Nas execuções seguintes, omita
`-ImportSave`. O executável aberto diretamente continua usando o save da pasta
atual, preservando a compatibilidade durante esta fase de transição.

Enquanto a interface gráfica não estiver pronta, as versões disponíveis podem
ser consultadas sem abrir o jogo:

```powershell
.\tools\pokemon_go_world\run_player_profile_pc.ps1 -Profile 1 -ListRecoveries
```

A coluna `Geracao` vem do contador interno dos setores completos do save. Por
isso ela distingue gravações mesmo quando o Windows mostra o mesmo horário para
arquivos criados muito próximos.

Para validar isoladamente o contêiner, a importação legada, a preservação de
blocos opcionais, corrupção e rotação atômica:

```powershell
powershell -ExecutionPolicy Bypass -File `
    .\tools\pokemon_go_world\tests\test_pc_save_container.ps1
```

Para restaurar uma versão, feche primeiro todas as janelas do jogo e escolha de
1 a 3:

```powershell
.\tools\pokemon_go_world\run_player_profile_pc.ps1 `
    -Profile 1 `
    -RestoreRecovery 2
```

A restauração é atômica. O save que estava ativo é preservado com o sufixo
`before-restore` antes que a recuperação escolhida assuma seu lugar.

## Problemas comuns

### `SDL2.dll` não foi encontrada

Confirme que a DLL está no mesmo diretório do executável e que sua arquitetura corresponde à do programa.

### O compilador ou o Python não foi encontrado

Use os parâmetros do script para informar os caminhos corretos. Evite alterar o código apenas para corrigir uma instalação local.

### O programa abre e fecha imediatamente

Execute pelo PowerShell para enxergar a mensagem de erro. Verifique também se o pacote de recursos está íntegro.

### Uma alteração não apareceu no jogo

Confirme que o arquivo foi salvo, que a compilação terminou sem erro e que você está executando o binário recém-gerado.

### `mapjson` falha com `Error 87` ou `Parâmetro incorreto`

O pipeline herdado pode tentar enviar centenas de caminhos de mapas em uma única linha de comando. No Windows, uma regeneração global pode ultrapassar o limite aceito por `CreateProcess`. Isso é um limite do gerador atual, não uma indicação de JSON inválido.

Enquanto o gerador não usar arquivo de resposta ou processamento em lotes, alterações em `data/maps/*/map.json` devem ser feitas e regeneradas pelo ambiente Linux/WSL documentado para o alvo GBA. Não apague arquivos gerados nem reduza a lista de mapas para contornar o erro, pois isso pode produzir uma árvore incompleta.

## Validação mínima

Antes de publicar uma alteração no alvo PC, verifique:

1. o executável inicia;
2. a tela inicial aparece;
3. teclado e controle respondem;
4. o mapa carrega sem artefatos graves;
5. salvar, fechar e carregar preserva o progresso;
6. áudio, menus e uma batalha simples funcionam.
