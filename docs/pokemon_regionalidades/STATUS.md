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

Em 30 de agosto de 2026, o alvo Windows foi recompilado integralmente com os novos nomes. O resultado foi:

- executável de 16.590.947 bytes;
- pacote de 16.489.008 bytes;
- 11.564 recursos indexados;
- leitura integral do índice do pacote concluída.

O alvo GBA não foi revalidado nessa execução porque o ambiente do Codex não recebeu acesso ao serviço WSL do Windows. Isso não indica erro no código da ROM; a compilação precisa ser repetida no PowerShell normal do usuário conforme [BUILDING_GBA.md](BUILDING_GBA.md).
