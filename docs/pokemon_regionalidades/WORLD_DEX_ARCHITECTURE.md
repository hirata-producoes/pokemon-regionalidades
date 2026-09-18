# Arquitetura do RotomDex mundial

## Decisão

Pokémon Regionalidades terá um único RotomDex, um registro mundial de espécies e
catálogos regionais progressivos. Não haverá uma Pokédex física para cada região
nem quatro conjuntos independentes de marcações de visto e capturado.

Esse modelo preserva a exploração de cada região sem criar uma trava técnica quando Kanto, Johto e Sinnoh forem acrescentadas.

## Três informações diferentes

| Informação | Escopo | Exemplo |
|---|---|---|
| observação | mundial | espécie não observada, vista ou capturada |
| catálogo | regional | posição e presença na Pokédex de Hoenn, Kanto, Johto ou Sinnoh |
| pesquisa liberada | por região | o professor daquela região instalou o módulo técnico correspondente |

As marcações de visto e capturado já são armazenadas pela base do Expansion usando a numeração nacional. Elas continuam válidas mesmo quando a interface regional atual não mostra aquela espécie.

## Funcionamento planejado

1. O professor da região inicial entrega o RotomDex e libera o catálogo técnico daquela região.
2. Ao encontrar ou capturar uma espécie de outro catálogo, o registro mundial conserva a observação imediatamente.
3. Enquanto o módulo regional correspondente não tiver sido liberado, a espécie aparece futuramente em **Notas de campo**, com dados limitados.
4. Ao falar com o professor de outra região, esse módulo é acrescentado ao mesmo RotomDex. Nada é apagado, sobrescrito ou recalculado.
5. A interface poderá alternar entre **Mundial**, **Kanto**, **Johto**, **Hoenn**, **Sinnoh** e **Notas de campo**, mostrando apenas as abas já disponíveis.

## Pesquisa e filtros planejados

A interface do RotomDex deverá permitir combinar filtros sem alterar os dados
registrados. A primeira versão funcional terá pesquisa pelo nome da espécie,
filtro por um ou dois tipos e estado de registro: todas, capturadas ou ainda
não capturadas. Os filtros deverão funcionar tanto no catálogo mundial quanto
nas abas regionais e em Notas de campo.

## Contrato técnico criado

No PC, o estado persistente mínimo de pesquisa fica no bloco obrigatório e
versionado `ROTOMDEX`. Ao abrir um save anterior, o programa une a máscara de
módulos ainda presente no final de `SaveBlock3` e grava o resultado nativo no
próximo save confirmado. O campo antigo permanece apenas como espelho de
compatibilidade e não é um limite permanente imposto pelo alvo GBA.

As operações centrais ficam isoladas em `pokemon_regionalidades_dex`:

- zerar o progresso de pesquisa em um jogo novo;
- liberar uma região sem afetar as anteriores;
- consultar quais módulos regionais estão liberados;
- consultar a observação mundial de uma espécie.

Ao receber o RotomDex do primeiro professor, a região escolhida para iniciar é
liberada. Professores futuros entregam atualização, módulo de pesquisa ou
aprimoramento para esse mesmo aparelho. Seus scripts chamam a operação regional
comum e nunca criam outro dispositivo ou outro banco de visto/capturado.

O menu abre uma única interface. As abas consultam a máscara de módulos
liberados antes de montar suas listas; obter Kanto depois de Hoenn apenas soma
uma aba. Essa regra evita tanto a sobrescrita pela última região quanto a
tentativa de abrir duas Pokédex simultaneamente.

## O que não será antecipado durante a base de Hoenn

A interface multirregional e as listas exatas de Johto e Sinnoh dependem dos conteúdos regionais que ainda serão integrados. Elas não serão simuladas agora. A base de Hoenn continuará sendo estabilizada com a interface atual, enquanto o formato persistente e o contrato de progressão já ficam definidos.

Também não será ativada imediatamente a tela tradicional de National Dex como solução provisória. Ela é uma lista única e não representa o modelo de pesquisa regional descrito acima.

## Compatibilidade

- O mesmo registro mundial evita perder Pokémon capturados antes de obter um novo catálogo.
- Catálogos são formas de ordenar e apresentar espécies, não bancos de captura separados.
- O formato possui versão para receber novas regiões e regras sem invalidar saves.
- O alvo GBA preserva a representação mínima compartilhada; o PC pode ampliar o
  estado no bloco `ROTOMDEX` sem perder ou sobrescrever módulos regionais.

O esquema 1 foi integrado em 9 de setembro de 2026. Codificação, validação,
migração e dois ciclos reais de gravação foram exercitados em uma sessão técnica
isolada. As observações de visto e capturado continuam no registro mundial já
fornecido pela base; a futura interface em abas e as Notas de campo ainda não
foram implementadas.
