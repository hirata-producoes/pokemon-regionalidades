# Roteiro do projeto

Este roteiro organiza a evolução técnica e de conteúdo. Ele não promete datas: cada marco só é considerado concluído quando pode ser compilado, testado e documentado.

## Princípios

- manter uma versão executável durante o desenvolvimento;
- implementar uma região por etapas jogáveis, não por quantidade de arquivos;
- compartilhar regras de jogo entre PC e GBA sempre que isso for viável;
- retirar do executável os recursos que podem viver no pacote externo;
- registrar decisões para que iniciantes entendam por que elas foram tomadas;
- preservar autoria, licenças e referências técnicas.

## Marco 1 — Fundação do projeto

Estado: concluído como base inicial.

- base efetiva na linha `pokeemerald-expansion 1.16.3`, embora o planejamento inicial tenha citado a 1.13;
- histórico publicado no GitHub;
- alvo GBA preservado;
- documentação técnica inicial;
- identidade Pokémon Regionalidades definida.

## Marco 2 — Porte nativo para PC

Estado: funcional, em validação contínua.

- janela e renderização via SDL2;
- entrada por teclado e controle;
- áudio, RTC e save nativos;
- executável Windows;
- primeiros testes de mapas, menus e batalhas;
- migração de nomes antigos de save e configuração.

## Marco 3 — Vertical slice de Hoenn

Estado: em andamento.

Fundação ambiental iniciada a partir das decisões D-023, D-029 e D-052:

- [x] ritmo 3× durante a execução;
- [x] calendário de 30 dias e fronteiras sazonais;
- [x] contrato das fases de transição;
- [x] avanço offline com âncora persistida no save;
- [x] pausa seletiva no menu inicial e seus submenus;
- [x] protótipo de estação, dia e horário no popup de área;
- [x] previsão climática determinística em blocos de seis horas;
- [x] mapa externo de Littleroot ligado ao clima dinâmico como protótipo;
- [ ] validar visualmente Littleroot em diferentes horários, estações e climas.

Uma *vertical slice* é uma pequena parte do jogo funcionando de ponta a ponta. O objetivo é validar:

- início de uma nova partida;
- exploração de uma sequência curta de mapas;
- diálogos, eventos e transições;
- captura e batalha;
- centro Pokémon, loja e save;
- retorno ao jogo depois de fechar o programa.

O detalhamento das decisões importadas da planilha está em [DESIGN_PLAN.md](DESIGN_PLAN.md).

## Marco 4 — Estrutura multirregional

Estado: planejado.

- definir como regiões são desbloqueadas;
- padronizar identificadores de mapas e pontos de viagem;
- separar progresso global e progresso regional;
- testar Pokédex, equipes, flags e variáveis entre regiões;
- integrar Kanto como segunda região jogável validada.

## Marco 5 — Pacote externo de recursos

Estado: protótipo existente, integração progressiva.

- versionamento do manifesto;
- validação de integridade;
- leitura de imagens, áudio e dados externos;
- mensagens claras quando um recurso estiver ausente;
- política de atualização e compatibilidade do pacote.

## Marco 6 — Regiões adicionais

Estado: planejado em longo prazo.

Johto, Sinnoh, Unova, Kalos, Alola, Galar, Paldea e regiões derivadas entram uma por vez. “Dados presentes no repositório” não será tratado como sinônimo de “região jogável”. Consulte [REGIONS.md](REGIONS.md).

## Marco 7 — Distribuição para PC

Estado: planejado.

- build reproduzível;
- pacote portátil para Windows;
- testes em instalação limpa;
- controles configuráveis;
- logs de diagnóstico;
- atualização de save entre versões;
- licenças e créditos incluídos no pacote.

## Definição de pronto

Uma funcionalidade é considerada pronta quando:

1. compila no alvo afetado;
2. tem um teste manual ou automatizado descrito;
3. não quebra saves suportados sem aviso e migração;
4. possui documentação proporcional à complexidade;
5. registra a origem de código ou recurso reutilizado.
