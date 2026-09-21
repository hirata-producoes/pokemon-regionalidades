# Perfis, saves e recuperação no PC

## Objetivo

O porte para PC permite criar perfis conforme a necessidade, sem um limite fixo
de dois espaços, e recuperar uma gravação anterior caso encontre um soft lock,
uma falha ou uma incompatibilidade durante o desenvolvimento.

Essa camada pertence ao produto para PC. Durante a primeira baseline, o programa
escolhe qual imagem de flash Emerald será apresentada ao motor. Antes de ativar
outras regiões, cada perfil migrará para o contêiner nativo descrito em
[Arquitetura de save nativo para PC](PC_SAVE_ARCHITECTURE.md): a imagem antiga
continuará dentro dele como compatibilidade, sem limitar os dados novos.

## Conceitos separados

| Conceito | Finalidade |
|---|---|
| Perfil de jogador | Campanha pessoal de uma pessoa |
| Save ativo | Estado carregado normalmente pelo jogo naquele perfil |
| Ponto de recuperação | Uma das três versões anteriores confirmadas do save ativo |
| Favorito | Uma de até cinco cópias verificadas de saves escolhidos explicitamente, fixas e separadas da rotação automática |
| Save rápido | Solicitação de gravação pelo sistema oficial em um estado seguro |
| Carregamento rápido | Reinicialização controlada da sessão usando um save validado |
| Save de exploração | Ferramenta técnica isolada, com Fly, HMs e recursos de teste |
| Save narrativo | Evidência autêntica gravada depois de alcançar uma cena pelo fluxo real |

O save de exploração não será confundido com uma campanha pessoal nem promovido
automaticamente para a campanha real.

Durante o desenvolvimento atual, o responsável usa o primeiro espaço para
exploração e testes e o segundo para a campanha normal de Hoenn. Esses são usos
daquelas campanhas, não nomes fixos do produto, e resultados da exploração não
se tornam evidência de continuidade narrativa.

### Interface de perfis implementada

A interface mostra uma lista rolável de perfis existentes e o botão **Criar
perfil**. A seleção exibe a campanha, o save atual, até três recuperações e até
cinco favoritos. Há ações para abrir, renomear, importar, exportar, reiniciar
ou retirar o perfil da lista mediante confirmação. Cada perfil tem uma campanha
independente, permitindo testar o início de Kanto e a entrada a partir de
Hoenn sem sobrescrever a campanha pessoal.

Cada favorito é uma cópia própria validada do save atual ou de uma recuperação,
não um dos três arquivos que a rotação substitui. Ele permanece na seleção até
ser removido pelo usuário. Restaurar um favorito ou uma recuperação preserva o
save ativo anterior. Remover favorito ou perfil move os arquivos para uma pasta
recuperável. Os perfis 1 e 2 preexistentes conservam os mesmos caminhos, nomes
e saves. O teste automatizado usa perfis temporários, valida cinco favoritos e
recusa o sexto. O usuário validou a tela de perfis com suas campanhas; o teste
automatizado de gerenciamento foi repetido com sucesso em 21/09/2026, usando
somente saves temporários.

## Estrutura de dados dos perfis

No Windows, os perfis usam `%LOCALAPPDATA%\Pokemon Regionalidades`. A
organização lógica é:

```text
Pokemon Regionalidades/
  config/
  profiles/
    profile-1/, profile-2/, ... profile-N/
      profile.json
      pokemon_regionalidades.pgrsave
      pokemon_regionalidades.pgrsave.recovery-1
      pokemon_regionalidades.pgrsave.recovery-2
      pokemon_regionalidades.pgrsave.recovery-3
      favorites/
        favorite-1.pgrsave ... favorite-5.pgrsave
  profiles-archived/
```

Os nomes exibidos já podem ser escolhidos pelos jogadores. Identificadores
internos não dependem do nome, portanto renomear um perfil não muda seu diretório
nem perde o vínculo com a campanha.

A seleção mostra o nome escolhido para o perfil. Nome do personagem, tempo
total de jogo e versão usada na última gravação ainda serão acrescentados. Em um
perfil vazio, a pessoa pode renomear o espaço antes de iniciar ou importar uma
campanha.

Uma ação explícita de reinício retira o progresso ativo e as recuperações
daquele perfil somente depois de confirmação clara. A pasta anterior inteira é
movida para um diretório `profile-N-reset-DATA-HORA`, no mesmo diretório dos
perfis, antes que o espaço vazio seja recriado com o nome personalizado. Assim,
a campanha nova não herda gerações ou recuperações e o estado anterior continua
recuperável. Exportar permanece a forma recomendada de guardar uma cópia
portátil.

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

Os nomes acima já são usados pela interface. Perfis anteriores continuam
visíveis pelo `.sav` até sua primeira abertura; o executável cria o `.pgrsave`
ao lado e preserva a origem. Recuperações antigas permanecem selecionáveis
durante a transição e são substituídas gradualmente pela rotação nativa.

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
O nome personalizado e o reinício individual já usam esse backend. Um teste
isolado renomeia um perfil, importa uma cópia, valida a interface, reinicia a
campanha e confirma que o nome permaneceu, que o perfil ativo ficou vazio e que
o save anterior continuou idêntico na pasta de segurança. Os dados amigáveis da
campanha e o acabamento definitivo permanecem posteriores.

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

A migração do contêiner nativo é uma etapa estrutural intermediária obrigatória
entre os itens 5 e 6. Ela será concluída antes da integração jogável de outra
região.

Nenhuma etapa substitui o save atual sem backup e teste de recuperação.
