# Teste no Switch OLED

## Objetivo da versão 0.2

Confirmar que a base abre em title mode, reconhece controles e estabelece uma
conexão HTTPS/TLS válida com o Microsoft Identity. Nenhuma conta é necessária
nesta fase.

## Checklist

- [ ] O ícone aparece no Homebrew Menu.
- [ ] O aplicativo inicia segurando `R` ao abrir um jogo.
- [ ] O título `ProjectNX` e a versão aparecem.
- [ ] `A` executa o teste de rede e depois percorre as telas simuladas.
- [ ] A tela informa `Rede segura: OK`.
- [ ] O diagnóstico informa `Rede: online (HTTP 200)`.
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
