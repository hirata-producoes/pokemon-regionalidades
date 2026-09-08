# Perfis, saves e recuperação no PC

## Objetivo

O porte para PC deverá permitir que duas pessoas joguem sem misturar progresso e
que cada perfil recupere uma gravação anterior caso encontre um soft lock, uma
falha ou uma incompatibilidade durante o desenvolvimento.

Essa camada pertence ao produto para PC. O núcleo compartilhado continua usando
o formato de save do jogo sempre que possível, mas o programa escolhe qual
arquivo será apresentado ao motor antes de iniciar a sessão.

## Conceitos separados

| Conceito | Finalidade |
|---|---|
| Perfil de jogador | Campanha pessoal de uma pessoa |
| Save ativo | Estado carregado normalmente pelo jogo naquele perfil |
| Ponto de recuperação | Uma das três versões anteriores confirmadas do save ativo |
| Save rápido | Solicitação de gravação pelo sistema oficial em um estado seguro |
| Carregamento rápido | Reinicialização controlada da sessão usando um save validado |
| Save de exploração | Ferramenta técnica isolada, com Fly, HMs e recursos de teste |
| Save narrativo | Evidência autêntica gravada depois de alcançar uma cena pelo fluxo real |

O save de exploração não será confundido com uma campanha pessoal nem promovido
automaticamente para a campanha real.

Durante o desenvolvimento atual, o responsável escolheu reservar o perfil 1 da
interface para exploração e testes e o perfil 2 para a campanha normal de Hoenn.
Essa atribuição é explícita e aparece nos títulos; ela não transforma resultados
do perfil de exploração em evidência de continuidade narrativa.

## Estrutura lógica planejada

No Windows, os perfis usam `%LOCALAPPDATA%\Pokemon Regionalidades`. A
organização lógica é:

```text
dados-do-jogador/
  configuracao-global/
  perfis/
    perfil-1/
      perfil.json
      atual.sav
      recuperacao-1.sav
      recuperacao-2.sav
      recuperacao-3.sav
    perfil-2/
      perfil.json
      atual.sav
      recuperacao-1.sav
      recuperacao-2.sav
      recuperacao-3.sav
  desenvolvimento/
    exploracao/
    marcos-narrativos/
```

Os nomes exibidos poderão ser escolhidos pelos jogadores. Identificadores
internos não dependerão do nome para evitar perda de vínculo ao renomear um
perfil.

## Gravação segura e rotação

Cada gravação concluída deverá seguir uma transação:

1. gravar em um arquivo temporário no mesmo volume;
2. conferir tamanho, versão, setores e checksums;
3. preservar o save ativo na rotação de três recuperações;
4. substituir o ativo de maneira atômica;
5. manter o ativo anterior se qualquer etapa falhar.

A interface mostrará data, horário interno, local, tempo de jogo e versão do
formato para que o jogador saiba qual recuperação está escolhendo. Recuperar um
save também criará uma cópia de segurança do estado que estava ativo.

Como os arquivos podem ser criados no mesmo segundo, o horário do Windows não é
usado sozinho para distingui-los. O backend já lê o contador de geração gravado
nos setores completos do próprio formato Emerald e também apresenta um hash
SHA-256; os dados amigáveis da campanha serão acrescentados à interface gráfica.

### Fundação já implementada

O porte PC já confirma o arquivo somente depois que o motor termina a gravação
sem registrar setores danificados. O conteúdo novo é escrito primeiro em
`<save>.pending`; o save ativo anterior é então preservado em
`<save>.recovery-1`, com rotação até `recovery-3`, e a substituição final é
atômica. Repetir exatamente a mesma gravação não consome uma recuperação.

Essa camada funciona para qualquer caminho de save usado atualmente, inclusive
as sessões isoladas. Ela foi validada por um teste técnico com cinco gerações de
arquivo, pela recompilação do jogo completo e por duas gravações sucessivas na
campanha-base, que produziram o ativo e duas recuperações de 131.072 bytes.

O backend já aceita caminhos explícitos e o lançador provisório separa os perfis
1 e 2. A abertura direta do executável mantém o arquivo da pasta atual, evitando
uma migração silenciosa durante o desenvolvimento. Uma importação só ocorre por
pedido explícito e é recusada quando o perfil de destino já possui progresso.

A primeira interface gráfica para Windows chama esse backend para abrir,
importar, exportar e restaurar. Ela não acessa os arquivos por uma segunda lógica própria.
O acabamento, os nomes personalizados e a integração ao menu externo definitivo
permanecem posteriores à validação funcional dessa etapa.

## Save rápido, carregamento rápido e Soft Reset

O primeiro save rápido não será um despejo da memória do processo. Savestates
crus capturariam ponteiros, callbacks, áudio, recursos SDL e posições sujeitas a
mudança entre builds, tornando o arquivo perigoso durante o desenvolvimento.

O save rápido usará o serializador oficial. Se a solicitação ocorrer durante uma
transição, script crítico, batalha ou outra tela não segura, ela será recusada
com uma explicação ou ficará pendente até o próximo ponto seguro. O carregamento
rápido validará o arquivo e reinicializará o motor antes de aplicá-lo.

Soft Reset é uma função diferente: reinicia a sessão e retorna ao fluxo inicial
sem encerrar o programa. Ele não grava automaticamente nem substitui um ponto de
recuperação.

## Configuração fora do save

Vídeo, áudio, idioma, aceleração e mapeamento de controles ficam fora do save da
campanha. A primeira tela compartilhada já configura teclado, ações principais
do controle XInput e multiplicador de aceleração. Vídeo, áudio, idioma e perfis
por dispositivo permanecem posteriores. Uma configuração ausente usa padrões
seguros sem impedir o carregamento dos perfis.

## Ordem de implementação

1. validar a primeira sequência completa de Hoenn e o save normal;
2. implementar a fundação de gravação atômica e três recuperações;
3. criar o diretório de dados e a seleção de dois perfis;
4. migrar com backup o save único existente;
5. integrar o menu externo de configuração;
6. acrescentar save rápido, carregamento rápido e Soft Reset;
7. validar migração entre versões e concluir os dados amigáveis exibidos pela
   interface.

Nenhuma etapa substitui o save atual sem backup e teste de recuperação.
