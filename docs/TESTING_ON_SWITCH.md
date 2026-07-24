# Teste no Switch OLED

## Objetivo da versão 0.5

Confirmar que o ProjectNX solicita consentimento XboxLive, conclui o login
Microsoft, obtém um Xbox User Token e solicita a autorização XSTS para o
sandbox `RETAIL`, sem gravar tokens no SD.

## Checklist

- [ ] O ícone aparece no Homebrew Menu.
- [ ] O aplicativo inicia segurando `R` ao abrir um jogo.
- [ ] O título `ProjectNX` e a versão aparecem.
- [ ] `A` executa o teste de rede e solicita um código de dispositivo.
- [ ] A tela informa `Rede segura: OK`.
- [ ] O diagnóstico informa `Rede: online (HTTP 200)`.
- [ ] A tela mostra `https://microsoft.com/devicelogin` e um código.
- [ ] O código pode ser confirmado no celular com a conta Microsoft correta.
- [ ] A autorização solicita acesso aos serviços Xbox.
- [ ] O Switch mostra brevemente `Conectando ao Xbox`.
- [ ] A tela passa pelas etapas Xbox User Token e XSTS.
- [ ] A tela informa `Perfil Xbox validado por XSTS`.
- [ ] O Gamertag correto aparece, caso a claim seja retornada.
- [ ] O diagnóstico informa `Xbox: XSTS_AUTHENTICATED | HTTP: 200`.
- [ ] O Switch muda automaticamente para `Catálogo de jogos`.
- [ ] `B` cancela um login pendente.
- [ ] `B` retorna para a tela anterior.
- [ ] `X` exibe o diagnóstico.
- [ ] O diagnóstico identifica corretamente portátil ou dock.
- [ ] `+` encerra e retorna ao Homebrew Menu.
- [ ] Não ocorre tela preta ou fechamento inesperado.

## Informações úteis em caso de falha

- versão do firmware;
- versão do Atmosphère;
- versão do Hekate;
- execução via title mode ou Álbum;
- portátil ou dock;
- etapa em que a falha ocorreu;
- fotografia da mensagem apresentada.
- valor de `XErr`, se o diagnóstico mostrar um.

Se o XSTS recusar o `RelyingParty`, a versão deverá mostrar uma mensagem de erro
e preservar `HTTP` e `XErr` no diagnóstico. Isso indicará a necessidade de
provisionamento no Xbox Partner Center, sem tentar contornar a autorização.

Não compartilhe arquivos de token, códigos de login, identificadores ou outros
dados da conta.
