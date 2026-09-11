# Controles do porte para PC

Os nomes A, B, L, R, Start e Select representam os botões do Game Boy Advance.
O botão `Configurações` da tela de perfis e o menu `Configurações` da janela do
jogo permitem remapear esses comandos sem alterar o save da campanha. Quando o
jogo está aberto, a nova configuração é recarregada em até um segundo.

`Aplicar` envia uma prévia ao jogo e mantém a janela aberta para comparar
tamanhos, volumes e controles. `Cancelar` ou fechar a janela restaura a
configuração que estava salva antes dos testes. `Salvar` confirma a seleção,
cria a recuperação normal da configuração e fecha a janela.

## Menu da janela

A primeira barra no estilo de emulador contém:

- `Jogo`: pausar/continuar, reiniciar a sessão, voltar à escolha de perfis e sair;
- `Configurações`: abrir controles e aceleração sem fechar a campanha;
- `Ajuda`: consultar os atalhos básicos.

`Voltar aos perfis` encerra primeiro a sessão atual e então reabre a tela de
perfis. Ele não grava automaticamente: use o save normal do jogo quando quiser
preservar o progresso atual.

## Teclado

| Função no jogo | Tecla atual |
|---|---|
| Movimentar e escolher opções | Setas direcionais |
| A: confirmar, conversar e interagir | Z |
| B: voltar, cancelar e correr quando permitido | X |
| Start: abrir o menu principal do jogo | Enter |
| Select | Backspace |
| L | A |
| R | S |
| Acelerar o jogo enquanto estiver pressionado | Espaço |
| Pausar ou continuar o programa | Ctrl + P |
| Reiniciar o jogo | Ctrl + R |

A aceleração é momentânea: ao soltar sua tecla, o jogo volta à velocidade normal.
O multiplicador pode ser escolhido entre 2× e 10×. Durante a aceleração, o áudio
é pausado para não produzir ruído ou tentar reproduzir vários segundos de som em
um segundo.

## Controle compatível com XInput

| Função no jogo | Botão atual |
|---|---|
| Movimentar | Direcional digital ou analógico esquerdo |
| A do GBA | A |
| B do GBA | X |
| Start | Start/Menu |
| Select | Back/View |
| L e R do GBA | Botões superiores esquerdo e direito |
| Acelerar | Gatilho direito |

O GBA possui apenas L e R. Ele não possui L2 e R2 como controles modernos. No
porte atual, A, B, Start, Select, L, R e aceleração podem ser atribuídos aos
botões A, B, X, Y, Start, Back, LB, RB, cliques dos analógicos ou gatilhos. O
direcional digital e o analógico esquerdo permanecem fixos para movimento nesta
primeira versão.

## Vídeo

A terceira aba da tela externa oferece dois comportamentos de janela:

- `Redimensionável`: permite ajustar livremente largura e altura;
- `Tamanho fixo`: bloqueia o redimensionamento no tamanho predefinido.

Os tamanhos disponíveis vão de 2× (`640 × 360`) a 5× (`1600 × 900`). Também é
possível alternar tela cheia, escala inteira, VSync e a moldura do jogo. Essas
opções entram em vigor durante a sessão; salvar somente controles não redefine
uma janela que já esteja redimensionada.

## Áudio

A aba `Áudio` possui controles de 0 a 10 para:

- volume geral;
- música;
- efeitos e gritos de Pokémon.

O volume geral é aplicado depois das duas categorias. Música e efeitos são
separados tanto nas amostras digitais quanto nos canais sonoros clássicos do GBA.
As mudanças entram em vigor durante a sessão.

## Limitações desta etapa

Esta é a primeira versão do menu externo. O jogo ainda não oferece:

- escolha entre aceleração momentânea e alternável;
- perfis diferentes de controle.

Esses recursos fazem parte da transformação do porte técnico em um produto de PC. Eles serão implementados depois que a campanha original estiver estável o suficiente para servir como referência de regressão.

O menu externo já dá acesso à seleção de perfil. Recuperação de saves,
save/carregamento rápidos e o Soft Reset inteiramente interno ainda serão
integrados. Configurações de dispositivo e vídeo
ficarão fora do save da campanha, para que um problema de configuração não
danifique o progresso. Consulte [Perfis, saves e recuperação no PC](SAVE_PROFILES.md).
