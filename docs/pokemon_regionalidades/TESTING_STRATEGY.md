# Estratégia de desenvolvimento e testes

## Objetivo

O projeto não deve repetir a campanha inteira depois de cada alteração. Ao mesmo tempo, um teste não pode inventar posições de personagens, encontros ou variáveis narrativas: um estado impossível pode produzir falso positivo ou falso negativo.

A estratégia usa três trilhas separadas:

1. a campanha real, para validar história, eventos e continuidade;
2. um save de exploração, para testar mapas e sistemas sem reconstruir a narrativa;
3. perfis pessoais, para jogar durante o desenvolvimento sem alterar evidências técnicas.

## Regra principal

Não são usados checkpoints que teleportam o jogador para dentro de uma cena e tentam recriar manualmente seu estado. O caso da Route 101 demonstrou o risco: objetos posicionados fora da sequência original alteraram o resgate e o encontro esperado.

Uma transição narrativa só é considerada validada quando foi alcançada pelo fluxo real do jogo ou por um save gravado pelo próprio jogo depois desse fluxo.

## Abrir a campanha-base de Hoenn

A campanha narrativa possui uma sessão persistente própria. Ela copia juntos o
executável, o pacote e a SDL2 atuais, mas não modifica flags, mapas, equipe,
itens, Pokédex ou posição do jogador:

```powershell
.\tools\pokemon_go_world\run_hoenn_baseline_pc.ps1
```

Na primeira execução, escolha `New Game` e percorra a história normalmente. Nas
execuções seguintes, o save criado pelo próprio jogo continuará disponível em
`build/baseline-hoenn`. Esse diretório não é compartilhado com o save da raiz nem
com `build/dev-save-exploracao`.

Para reiniciar a validação desde `New Game`, use `-NewSession`. O save anterior é
movido para `build/baseline-hoenn/backups` em vez de ser apagado:

```powershell
.\tools\pokemon_go_world\run_hoenn_baseline_pc.ps1 -NewSession
```

## Save de exploração

O save de exploração é criado a partir de uma cópia de um `.sav` válido. O jogo carrega essa cópia, aplica o perfil e a grava novamente por seu sistema normal de save; assim, setores e checksums não são editados por endereços fixos.

O perfil:

- preserva a posição, os objetos, as variáveis narrativas e o Mudkip do save de origem;
- exige uma equipe com apenas o Mudkip e acrescenta os cinco Pokémon nas vagas livres;
- cria cinco Pokémon de Hoenn no nível 75;
- distribui Cut, Fly, Surf, Strength, Flash, Rock Smash, Waterfall e Dive;
- registra os cinco Pokémon adicionados como vistos e capturados;
- não precisa fabricar insígnias: qualquer movimento de campo conhecido pode
  ser usado desde o início, independentemente da história;
- libera os destinos de Fly de Hoenn;
- habilita os menus mínimos necessários para a exploração.

A tentativa anterior de posicionar a cópia em Oldale foi retirada: ela preservava objetos do caminhão e criava um estado inconsistente. Agora o perfil parte do save autêntico do personagem B. A Pokédex continua dependendo da aquisição normal pela história. Os Pokémon adicionados recebem o cálculo completo dos atributos, incluindo HP máximo.

Esse save serve para observar mapas, transições comuns, interiores, colisões, renderização, áudio, clima e sistemas gerais. Ele não prova que a história chegou corretamente àquele lugar.

## Criar o save

Depois de compilar o alvo PC, execute na raiz do projeto:

```powershell
.\tools\pokemon_go_world\create_dev_mobility_save_pc.ps1 -Recreate
```

O resultado fica em:

```text
build/dev-save-exploracao/pokemon_regionalidades.pgrsave
```

O gerador trabalha numa pasta isolada e não modifica o `.pgrsave` nem o `.sav`
da raiz. Ele abre uma execução temporária e oculta, carrega o save de origem,
aplica o perfil, chama o sistema oficial de gravação e encerra depois de
confirmar `SAVE_STATUS_OK` no log.

Para escolher outro save de origem:

```powershell
.\tools\pokemon_go_world\create_dev_mobility_save_pc.ps1 `
    -SourceSave 'C:\caminho\meu-save.sav' `
    -Recreate
```

## Abrir o save depois de criado

```powershell
.\tools\pokemon_go_world\run_dev_mobility_save_pc.ps1
```

Essa execução não reaplica o perfil. Alterações feitas e salvas pelo jogador continuam na cópia de exploração.

## Operações com inventário amplo

O teste manual de descarte, depósito e retirada usa outra cópia isolada. Ele
prepara 99.999 unidades no primeiro espaço de Poké Bolas e no primeiro espaço do
PC de itens, sem alterar o save de exploração nem os dois perfis pessoais:

```powershell
.\tools\pokemon_go_world\run_inventory_operations_test_pc.ps1
```

Execuções seguintes preservam e reabrem o save dessa sessão. Para descartar a
cópia e preparar novamente as quantidades iniciais, use `-Recreate`. O progresso
feito nela continua sendo descartável e não deve ser usado como campanha. O
teste deve confirmar uma redução na mochila, uma retirada parcial do PC, um
depósito de volta, a retirada da pilha inteira e a permanência dos valores
depois de salvar e reabrir a mesma sessão.

## Perfis pessoais durante o desenvolvimento

Os dois perfis pessoais não compartilham arquivos com o save de
exploração. Cada perfil terá um save ativo e três recuperações validadas. Jogar,
salvar ou carregar em um perfil pessoal não mudará o estado usado em testes
técnicos e não será registrado automaticamente como prova de uma cena narrativa.

A fundação de gravação atômica já está ativa nos caminhos atuais. Cada alteração
real do save preserva até três arquivos `.recovery-N`; uma gravação idêntica não
faz a rotação avançar. Um teste isolado confirmou o ativo e as três gerações após
cinco gravações. A interface permite abrir os dois perfis, importar, exportar e
restaurar recuperações. Ambos os perfis reais foram abertos, salvos e reabertos
com sucesso no contêiner nativo em 9 de setembro de 2026.

O lançador da baseline continua reservado ao percurso narrativo de referência e
o lançador de exploração à mobilidade e aos sistemas. A arquitetura está em
[Perfis, saves e recuperação no PC](SAVE_PROFILES.md).

## O que ainda exige a campanha real

- introdução e escolha de nome e gênero;
- posição e aparecimento de NPCs;
- gatilhos de mapa;
- batalhas obrigatórias;
- entrega de itens e Pokémon pela história;
- mudanças de cenário provocadas por eventos;
- progressão entre cidades, ginásios e liga;
- encerramento e créditos.

Para esses casos, mantenha saves reais em marcos importantes. O próprio jogador pode salvar antes de uma cena; a cópia desse arquivo vira uma referência reproduzível sem falsificar o estado interno.

## Divisão das verificações

As verificações técnicas automatizadas cobrem compilação, integridade do pacote, checksums, carregamento de recursos, regras determinísticas e ausência de exceções nativas. Não devem pilotar longas sequências narrativas por coordenadas.

A verificação visual fica com o responsável pelo jogo, usando instruções curtas: local, ação, resultado esperado e defeitos conhecidos. Capturas automáticas são reservadas para diagnóstico quando realmente acrescentam evidência.

## Segurança contra cópias esquecidas

O código funcional existe apenas na árvore oficial. `build/dev-save-exploracao` contém executável, pacote, DLL, configuração, log e save; não contém uma segunda cópia do código-fonte.

Antes de criar ou abrir o save de exploração, os três artefatos oficiais atuais são copiados juntos para a pasta isolada. O jogo é iniciado pela cópia, com diretório de trabalho próprio, e não acessa o save ou a configuração da raiz. `dev-session-manifest.txt` registra tamanho e SHA-256 do executável, pacote e DLL usados naquela preparação.

O perfil exige simultaneamente `POKEMON_REGIONALIDADES_DEV_SESSION` e `POKEMON_REGIONALIDADES_APPLY_MOBILITY_PROFILE`. O gerador define as duas somente para seu processo filho e restaura os valores anteriores em seguida. A execução comum, o lançador de exploração e o alvo GBA não aplicam o perfil.

## Regra para protótipos de sistema

Um protótipo avalia uma hipótese e não substitui silenciosamente a base real. Funcionalidades experimentais devem permanecer desativadas na execução normal, usar dados ou sessões identificadas e declarar quais partes da campanha não reproduzem.

Antes de migrar uma solução do protótipo para a base de Hoenn, ela precisa:

1. usar o mesmo carregador, save e recursos da base atual;
2. funcionar em um mapa original representativo, não apenas em uma cena reduzida;
3. ser testada em uma segunda situação de escala ou conteúdo diferente;
4. preservar o comportamento original quando a funcionalidade estiver desativada;
5. passar por compilação e teste de regressão proporcionais ao risco;
6. ter limitações, compatibilidade de save e evidência documentadas.
