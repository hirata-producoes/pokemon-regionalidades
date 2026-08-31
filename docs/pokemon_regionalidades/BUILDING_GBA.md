# Compilando a versão para GBA

O alvo GBA preserva a possibilidade de gerar uma ROM compatível com emuladores e, futuramente, com hardware adequado. Ele é útil como referência técnica e como forma de comparar o comportamento com a versão nativa para PC.

## Limite importante

O mapeamento tradicional do Game Boy Advance oferece até 32 MiB de espaço endereçável para a ROM. MiB significa **mebibyte**, uma medida de bytes: 1 MiB equivale a 1.048.576 bytes. Não significa megabit.

Por esse motivo, o alvo GBA não é a solução para armazenar indefinidamente todas as regiões, músicas e imagens. O alvo PC e o pacote externo de recursos existem para superar essa limitação arquitetural.

## Ambiente recomendado

No Windows, o fluxo recomendado utiliza WSL2 com uma distribuição chamada `Ubuntu`. O script PowerShell chama o ambiente Linux e organiza os caminhos necessários.

Se WSL2 e Ubuntu ainda não estiverem instalados, abra o PowerShell como administrador na raiz do projeto e execute:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\install_wsl2.ps1
```

Reinicie o Windows se for solicitado e conclua a configuração inicial do Ubuntu. Depois, em um PowerShell normal, instale as dependências do build:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_rom.ps1 -InstallDependencies
```

Depois, para compilações normais:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\build_rom.ps1
```

O resultado esperado é:

```text
pokemon_regionalidades.gba
```

## Teste em emulador

O mGBA é a referência prática recomendada para testar o alvo GBA. Abra a ROM gerada e verifique inicialização, mapas, áudio, save e batalhas.

Uma ROM maior que 32 MiB exigiria um formato, um mapper e um emulador personalizados. Isso deixaria de ser um alvo GBA convencional. Para este projeto, investir no porte nativo para PC é mais sustentável.

## Relatório de memória

Quando os arquivos de mapa da compilação estiverem disponíveis, execute:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pokemon_go_world\memory_report.ps1
```

O relatório ajuda a identificar quais seções consomem mais ROM e quanto espaço ainda existe.

## O que não deve ser enviado ao GitHub

- ROMs compiladas (`*.gba`);
- saves pessoais (`*.sav`);
- toolchains baixados localmente;
- arquivos de propriedade de terceiros sem permissão de redistribuição.

O GitHub deve armazenar o código-fonte, a documentação e os recursos que podem ser legalmente redistribuídos. Cada pessoa compila sua própria cópia.
