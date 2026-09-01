# Arquitetura

## Ideia central

O código de gameplay foi escrito para o GBA. Em vez de reescrever o jogo inteiro, o porte mantém esse código e fornece equivalentes para os serviços do console.

```text
mapas, scripts, batalhas e menus
                 |
          código compartilhado
           /             \
      alvo GBA          alvo PC
   hardware real     camada SDL2
```

## Componentes

| Componente | Responsabilidade |
|---|---|
| `src/platform/sdl2.c` | janela, eventos, áudio, RTC, save e ciclo de execução |
| `src/platform/gba_easy_draw.c` | interpreta o estado gráfico e compõe frames |
| `src/platform/bios.c` | substitui rotinas de BIOS necessárias |
| `src/platform/dma.c` | modela transferências DMA |
| `src/platform/cgb_audio.c` | reproduz canais de áudio no estilo CGB |
| `src/platform/resource_pack.c` | abre, valida e lê o pacote externo |
| `src/pokemon_regionalidades_clock.c` | cálculos puros de relógio, calendário, seed e previsão climática |
| `src/pokemon_go_world.c` | estado mundial persistido e consultas ambientais de gameplay |
| `Makefile_pc` | gera recursos e vincula o executável Windows |

## Por que ainda é 32 bits

O processo PC usa x86 de 32 bits porque partes da base armazenam ponteiros em estruturas e scripts com layout de 32 bits. Isso não é o limite de 32 MiB do GBA. O pacote possui offsets de 64 bits e é lido sob demanda.

Uma futura migração para 64 bits exigirá auditar tamanhos de estruturas, casts, formatos serializados e interfaces de assembly. Não é apenas trocar uma opção do compilador.

## Pacote externo

`pokemon_regionalidades.pak` guarda recursos grandes fora do executável. O índice usa nomes, FNV-1a de 64 bits e CRC32. Apenas índice e nomes entram na memória ao abrir; os blobs são carregados quando necessários.

Músicas e voicegroups contêm referências internas. Seus geradores registram relocações para que os ponteiros sejam reconstruídos com segurança no endereço onde cada blob foi carregado.

## Nomes legados

`PGW_*`, `tools/pokemon_go_world` e algumas variáveis de ambiente continuam presentes. Eles formam um namespace interno estável. A interface pública já usa Pokémon Regionalidades; a migração interna será incremental para facilitar revisão e regressão.

Para detalhes de formatos, métricas e histórico, consulte o [relatório técnico](../pokemon_go_world/RELATORIO_TECNICO_PORTE_PC.md).

O fluxo ambiental e seus limites de protótipo estão documentados em [Sistema ambiental](ENVIRONMENT_SYSTEM.md).
