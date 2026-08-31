# Compilando a versão para Windows

Este guia mostra como gerar o executável nativo para PC. Você não precisa entender todo o sistema de compilação antes de começar; os detalhes técnicos aparecem ao lado das instruções práticas.

## O que será gerado

O alvo para Windows produz:

- `pokemon_regionalidades-pc.exe`: programa principal;
- `SDL2.dll`: biblioteca de janela, teclado, controle e áudio;
- `pokemon_regionalidades.pak`: pacote externo opcional de recursos;
- `pokemon_regionalidades.sav`: save criado durante o jogo;
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
| Select | Barra invertida |
| L / R | A / S |
| Acelerar | Espaço |

Controles XInput também podem funcionar, dependendo do dispositivo e da configuração do SDL2.

## Recursos externos

Por padrão, o programa procura `pokemon_regionalidades.pak` ao lado do executável. Também é possível apontar outro arquivo:

```powershell
$env:POKEMON_REGIONALIDADES_RESOURCE_PACK = 'D:\jogos\recursos\regionalidades.pak'
.\pokemon_regionalidades-pc.exe
```

O nome antigo `POKEMON_GO_WORLD_RESOURCE_PACK` continua aceito para não quebrar ambientes já configurados.

## Compatibilidade com saves antigos

Quando o novo save ainda não existe, o programa tenta copiar `pokemon_go_world.sav` para `pokemon_regionalidades.sav`. O arquivo antigo é preservado. A mesma regra vale para o arquivo de configuração.

Mesmo com essa migração automática, mantenha cópias de segurança antes de testar versões em desenvolvimento.

## Problemas comuns

### `SDL2.dll` não foi encontrada

Confirme que a DLL está no mesmo diretório do executável e que sua arquitetura corresponde à do programa.

### O compilador ou o Python não foi encontrado

Use os parâmetros do script para informar os caminhos corretos. Evite alterar o código apenas para corrigir uma instalação local.

### O programa abre e fecha imediatamente

Execute pelo PowerShell para enxergar a mensagem de erro. Verifique também se o pacote de recursos está íntegro.

### Uma alteração não apareceu no jogo

Confirme que o arquivo foi salvo, que a compilação terminou sem erro e que você está executando o binário recém-gerado.

## Validação mínima

Antes de publicar uma alteração no alvo PC, verifique:

1. o executável inicia;
2. a tela inicial aparece;
3. teclado e controle respondem;
4. o mapa carrega sem artefatos graves;
5. salvar, fechar e carregar preserva o progresso;
6. áudio, menus e uma batalha simples funcionam.

