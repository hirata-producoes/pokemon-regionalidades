# Regiões e jogos de referência

Ter uma espécie, um inicial ou uma constante de região no código não significa que uma campanha inteira esteja pronta. Esta página usa três estados:

- **jogável/base:** conteúdo integrado ao fluxo atual;
- **dados presentes:** mapas ou mecânicas existem, mas ainda não formam uma campanha validada;
- **planejado:** há design, constantes ou iniciais, mas faltam mapas, eventos e progressão.

| Região | Jogos de referência | Estado |
|---|---|---|
| Hoenn | Ruby, Sapphire, Emerald e referências de ORAS | Base atual. O início de Emerald e o primeiro mapa foram validados no PC. |
| Kanto | Red, Green, Blue, Yellow, FireRed, LeafGreen e referências de Let's Go | Dados de FRLG presentes. Pallet Town possui uma integração inicial; a campanha completa ainda não foi validada. |
| Ilhas Sevii | FireRed e LeafGreen | Mapas registrados com os dados FRLG; progressão mundial não implementada. |
| Johto | Gold, Silver, Crystal e HGSS | Planejada; iniciais e identificador preparados. |
| Sinnoh | Diamond, Pearl, Platinum e BDSP | Planejada; iniciais e identificador preparados. |
| Hisui | Legends: Arceus | Planejada; formas, evoluções e iniciais disponíveis na base. |
| Unova | Black, White, Black 2 e White 2 | Planejada; iniciais e identificador preparados. |
| Kalos | X e Y | Planejada; iniciais e identificador preparados. |
| Alola | Sun, Moon, Ultra Sun e Ultra Moon | Planejada; formas regionais e iniciais disponíveis. |
| Galar | Sword e Shield | Planejada; formas regionais e iniciais disponíveis. |
| Paldea | Scarlet e Violet | Planejada; espécies, formas e iniciais disponíveis. |
| Ilhas Laranja | anime | Região especial planejada; trio provisório Pikachu, Eevee e Lapras. |

## Inventário observado

`data/maps/map_groups.json` registra 936 entradas de mapas:

- 518 da organização Emerald/Hoenn;
- 417 identificadas como FireRed/LeafGreen;
- 1 mapa próprio no grupo Pokémon Regionalidades: `PalletTown_Pgw`.

Vários mapas compartilham layouts, por isso quantidade de mapas e quantidade de layouts não são iguais.

## Gerações

`include/config/species_enabled.h` habilita Pokémon das gerações I a IX. Isso amplia espécies e mecânicas, mas não importa automaticamente histórias, cidades ou campanhas dos jogos modernos.

## Critério para considerar uma região pronta

Uma região só será marcada como jogável quando tiver entrada e saída coerentes, mapas essenciais, encontros, eventos, progressão, tradução, save testado e uma sequência reproduzível de começo ao marco definido para aquela entrega.
