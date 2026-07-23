# Segurança e privacidade

ProjectNX será um cliente não oficial. A autenticação e os endpoints de
streaming podem mudar sem aviso.

## Regras do projeto

- O aplicativo não pede a senha da Microsoft.
- O login será feito por código de dispositivo nos servidores oficiais.
- Tokens nunca serão escritos em logs.
- Arquivos de autenticação ficam fora do pacote de atualização.
- A opção de logout removerá os tokens locais.
- Dumps e logs destinados a diagnóstico precisam ser revisados antes de serem
  compartilhados.
- `client_secret`, senhas e tokens nunca devem ser colocados no `config.ini`.
- O `client_id` configurável deve pertencer ao aplicativo público do ProjectNX,
  sem reutilizar identificadores de outros clientes.

## Limitação do armazenamento local

O ambiente homebrew não oferece ao ProjectNX um cofre de credenciais equivalente
ao de sistemas móveis modernos. Um token persistente armazenado no cartão SD
deve ser tratado como informação sensível. Quem tiver acesso ao cartão poderá
copiá-lo.

Até que uma estratégia segura seja implementada, a versão preview não grava
credenciais.

## Console modificado

O projeto não pode garantir ausência de bloqueio ou banimento do console. O
usuário é responsável pela configuração do ambiente, pelas regras dos serviços
e pela proteção do próprio cartão SD.
