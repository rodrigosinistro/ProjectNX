# Teste no Switch OLED

## Objetivo da versão 0.3.1

Confirmar que o ProjectNX solicita um código de dispositivo, mostra a URL
oficial da Microsoft e detecta a conclusão do login sem gravar tokens no SD.

## Checklist

- [ ] O ícone aparece no Homebrew Menu.
- [ ] O aplicativo inicia segurando `R` ao abrir um jogo.
- [ ] O título `ProjectNX` e a versão aparecem.
- [ ] `A` executa o teste de rede e solicita um código de dispositivo.
- [ ] A tela informa `Rede segura: OK`.
- [ ] O diagnóstico informa `Rede: online (HTTP 200)`.
- [ ] A tela mostra `https://microsoft.com/devicelogin` e um código.
- [ ] O código pode ser confirmado no celular com a conta Microsoft correta.
- [ ] O Switch muda automaticamente para `Catálogo de jogos` após o login.
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
