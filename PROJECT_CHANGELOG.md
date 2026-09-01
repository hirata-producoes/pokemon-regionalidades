# Histórico do Pokémon Regionalidades

Este arquivo acompanha apenas mudanças específicas do projeto. O `CHANGELOG.md` original continua reservado ao histórico herdado do `pokeemerald-expansion`.

## Em desenvolvimento

- planejamento atualizado convertido em registro Markdown rastreável;
- World Clock configurado em 3× conforme D-023;
- estações alteradas para 30 dias internos, com transição nos dias 29–30 e 1–2;
- avanço offline calculado a partir de uma âncora de RTC persistida sem ampliar os SaveBlocks;
- cálculo sazonal separado em API determinística e testes de fronteira adicionados;
- pausa transitória do World Clock integrada ao menu inicial e aos seus submenus, sem persistir um flag de pausa no save;
- protótipo de interface ambiental adicionado ao popup de área, mostrando estação, dia sazonal e horário;
- identidade pública alterada de Pokémon GO World para Pokémon Regionalidades;
- executáveis, ROM, pacote de recursos, save e configuração receberam nomes públicos próprios;
- leitura compatível dos nomes antigos de recursos;
- cópia não destrutiva de save e configuração legados;
- documentação reorganizada para estudo, referência e portfólio;
- regiões e estados de implementação descritos separadamente;
- guias de compilação para Windows e GBA adicionados;
- roteiro, contribuição, créditos e avisos legais documentados.

## 30 de agosto de 2026 — Primeira publicação

- histórico completo enviado para `hirata-producoes/pokemon-regionalidades`;
- branch principal configurada como `main`;
- commit de referência da publicação inicial: `27251a5d0e`.

## Fase anterior — Porte PC experimental

- integração inicial de SDL2;
- execução nativa no Windows;
- testes de renderização, áudio, RTC, controles, save, mapas e batalhas;
- protótipo de pacote externo de recursos;
- relatório técnico preservado em `docs/pokemon_go_world/RELATORIO_TECNICO_PORTE_PC.md`.
