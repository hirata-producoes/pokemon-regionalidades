# Estado atual

## Resumo

O projeto possui uma fundação GBA e um porte nativo para Windows. O porte não emula a ROM: ele recompila o código e substitui serviços de hardware por implementações de PC.

## Validado em execução

- inicialização SDL2 e renderização;
- tela de copyright, introdução, título e novo jogo;
- caminhão inicial, Littleroot e transição de mapa;
- teclado e controle;
- save, carregamento e RTC;
- áudio MP2K não silencioso;
- menu de equipe e resumo;
- batalha selvagem de diagnóstico;
- pacote externo válido, ausente e corrompido;
- recursos externos de Pokémon, cries, tilesets, animações, samples, músicas e voicegroups.

## Implementado, mas ainda em validação

O primeiro contrato de gameplay derivado da planilha foi integrado:

- World Clock durante a execução alterado de 20× para 3×;
- calendário corrigido de 28 para 30 dias internos por estação;
- fases de transição dos dias 29–30 e 1–2 representadas por API própria;
- tempo real transcorrido com o jogo fechado aplicado em 3× ao carregar o save;
- âncora offline armazenada em duas variáveis antes não utilizadas, sem ampliar os SaveBlocks;
- menu inicial e submenus pausam o relógio por estado transitório, sem gravar um flag de pausa no save;
- popup de área apresenta um protótipo compacto com estação, dia sazonal e horário;
- clima lógico determinístico calculado por estação, região, área e blocos de seis horas;
- previsão das próximas 24 horas permanece estável e não consome o RNG de gameplay;
- mapa externo de Littleroot traduz o estado lógico em clima visual e pode atualizar o efeito durante a permanência no mapa;
- popup de área passou a separar corretamente o texto ambiental do horário e agora inclui a condição climática;
- cálculo determinístico da troca de estação coberto por testes de fronteira;
- build nativo para Windows concluído depois da alteração.

O executável recompilado também passou por smoke test isolado: permaneceu estável durante a inicialização, validou o RTC nativo, abriu o pacote de 11.564 recursos e produziu frames da introdução sem usar o save de desenvolvimento.

Os casos de teste da pausa, dos nomes sazonais, da estabilidade climática no bloco de seis horas e da previsão através da meia-noite foram adicionados e compilados no alvo de testes. A execução automatizada continua pendente porque as ferramentas POSIX do test runner não compilam pelo MinGW e o serviço WSL não estava acessível nesta sessão. Ainda faltam o teste manual da pausa, a validação visual completa do popup e de Littleroot, a mudança climática ao vivo e um teste com fechamento e reabertura controlados. Portanto, D-023 e o protótipo de D-029 estão integrados em sua base, mas ainda não estão validados por completo.

Layouts de mapas foram externalizados em 884 recursos. O build foi concluído e `maps.o` diminuiu de 955.473 para 305.313 bytes. Ainda faltam:

- validar o primeiro mapa com o pacote atual;
- testar diferentes tipos de layout e transições;
- testar ausência e corrupção dessa família;
- confirmar o fallback;
- registrar capturas e logs do marco.

## Planejado

- campanhas regionais além da base atual;
- seleção mundial de região no fluxo final;
- pacote externo para eventos, gráficos gerais, fontes e scripts;
- testes automatizados mais amplos;
- empacotamento de release;
- revisão futura da necessidade de 64 bits.

## Artefatos atuais

Os novos builds usam:

- `pokemon_regionalidades-pc.exe`;
- `pokemon_regionalidades.pak`;
- `pokemon_regionalidades.sav`;
- `pokemon_regionalidades.cfg`;
- `pokemon_regionalidades.gba` no alvo GBA.

Ao iniciar pela primeira vez com o nome novo, o PC copia save e configuração antigos quando eles existem e os novos ainda não existem. Os arquivos antigos não são apagados.

## Última validação de build

Em 1º de setembro de 2026, o alvo Windows foi recompilado depois da integração da previsão climática e do perfil ambiental de Littleroot. O resultado foi:

- executável de 16.595.903 bytes;
- pacote de 16.489.008 bytes;
- 11.564 recursos indexados;
- leitura integral do índice do pacote concluída.

O executável permaneceu ativo por 12 segundos em um smoke test isolado. O stderr registrou apenas a ausência opcional das imagens de fundo e borda da janela nativa; não ocorreu encerramento inesperado.

O alvo GBA não foi revalidado nessa execução porque o ambiente do Codex não recebeu acesso ao serviço WSL do Windows. Isso não indica erro no código da ROM; a compilação precisa ser repetida no PowerShell normal do usuário conforme [BUILDING_GBA.md](BUILDING_GBA.md).
