# Arquitetura do ProjectNX

## Objetivo

Entregar um cliente nativo de baixa latência sem depender do navegador interno
do Switch. O executável principal será um `.nro` para Horizon OS/libnx.

## Camadas

```text
Interface e navegação
        |
Estado da aplicação
        |
Autenticação | Catálogo | Sessão de streaming
        |          |              |
 HTTP/TLS     Cache local     WebRTC e canais de dados
                                   |
                         Vídeo | Áudio | Controles
                                   |
                         NVDEC | Opus | HID/Vibração
```

### Núcleo

Máquina de estados independente da plataforma. Pode ser compilada e testada em
um computador comum.

### Plataforma Switch

Responsável por ciclo do applet, entrada HID, modo dock/portátil, rede, áudio,
renderização e acesso ao cartão SD.

### Autenticação

Usará OAuth 2.0 Device Authorization Grant. O usuário digitará no celular ou
computador o código mostrado no Switch. O ProjectNX nunca solicitará nem
armazenará a senha da conta Microsoft.

O `client_id` é o identificador público do aplicativo registrado pelo projeto.
Identificadores privados ou extraídos de aplicativos Xbox não são reutilizados.
A base 0.4 solicita o código com consentimento XboxLive, respeita o intervalo de
consulta informado pela Microsoft e troca o acesso por um Xbox User Token
oficial. Os tokens permanecem somente em memória.

### Catálogo

Carregará títulos autorizados para a conta e região, com cache de metadados e
capas no cartão SD. Pesquisa e favoritos serão locais.

### Streaming

A sessão será negociada pelo backend de streaming e transportada por WebRTC.
O pipeline planejado é:

- vídeo H.264;
- decodificação NVDEC no Tegra X1;
- renderização GPU;
- áudio Opus estéreo;
- canal de dados para controles, vibração e mensagens da sessão.

## Diretórios planejados no SD

```text
sdmc:/switch/projectnx/
  projectnx.nro
  config.ini
  cache/
  logs/
  auth/
```

Tokens e logs nunca serão incluídos em pacotes de atualização.

## Princípios

- nenhuma senha dentro do aplicativo;
- nenhum identificador secreto embutido no repositório;
- funções de rede isoladas da interface;
- erros visíveis e recuperáveis;
- bitrate e resolução adaptáveis;
- suporte prioritário a title mode;
- testes do núcleo executáveis sem um Switch.
