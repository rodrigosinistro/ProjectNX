# Teste no Switch OLED

## Objetivo da versão 0.4

Confirmar que o ProjectNX solicita consentimento XboxLive, conclui o login
Microsoft e troca o acesso por um Xbox User Token sem gravar tokens no SD.

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
- [ ] A tela informa `Conta Xbox conectada`.
- [ ] O diagnóstico informa `Xbox: USER_AUTHENTICATED | HTTP: 200`.
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

Não compartilhe arquivos de token ou dados da conta.
