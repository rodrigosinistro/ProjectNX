# Roadmap

## 0.1 — Preview nativo

- [x] Estrutura do projeto libnx
- [x] Máquina de estados
- [x] Leitura básica de controles
- [x] Detecção portátil/dock
- [x] Testes locais do núcleo
- [x] Workflow de compilação automática
- [x] Compilação com devkitA64
- [x] Teste físico no Switch OLED

## 0.2 — Base de rede e interface

- [x] Inicialização de sockets
- [x] Cliente HTTPS/TLS
- [x] JSON
- [x] Configuração persistente
- [ ] Logger com remoção de dados sensíveis
- [ ] Interface gráfica inicial

## 0.3 — Autenticação

- [x] OAuth por código de dispositivo
- [x] Tela de código e URL
- [x] Polling cancelável
- [ ] Renovação de sessão
- [ ] Logout e exclusão local dos tokens

## 0.4 — Identidade Xbox e catálogo

- [x] Consentimento `XboxLive.signin`
- [x] Xbox User Token
- [ ] XSTS para serviços Xbox
- [ ] Identificação segura do perfil
- [ ] Descoberta de região
- [ ] Catálogo autorizado
- [ ] Pesquisa e filtros
- [ ] Capas em cache
- [ ] Favoritos
- [ ] Estados de fila e indisponibilidade

## 0.5 — Transporte

- [ ] Criação da sessão de streaming
- [ ] Oferta/resposta SDP
- [ ] ICE e DTLS-SRTP
- [ ] Canais de áudio, vídeo, entrada e mensagens
- [ ] Keepalive e reconexão

## 0.6 — Reprodução

- [ ] Decodificação H.264 por NVDEC
- [ ] Renderização por GPU
- [ ] Áudio Opus
- [ ] Sincronização A/V
- [ ] Escalonamento 720p/1080p

## 0.7 — Controles e estabilidade

- [ ] Joy-Con e Pro Controller
- [ ] Mapeamento configurável
- [ ] Vibração
- [ ] Telemetria local de latência e perda
- [ ] Recuperação de keyframe
- [ ] Sessões longas e suspensão

## 1.0 — Primeira versão estável

- [ ] Fluxo completo dentro do ProjectNX
- [ ] Instalador simples
- [ ] Atualização sem apagar configurações
- [ ] Documentação em português
- [ ] Matriz de compatibilidade

Remote Play de um console próprio será avaliado depois do fluxo xCloud estável.
