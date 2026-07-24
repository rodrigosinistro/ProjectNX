# ProjectNX

[![Build ProjectNX](https://github.com/rodrigosinistro/ProjectNX/actions/workflows/build.yml/badge.svg)](https://github.com/rodrigosinistro/ProjectNX/actions/workflows/build.yml)

Cliente homebrew nativo e não oficial para acessar jogos por nuvem no Nintendo
Switch. O projeto começa pelo Xbox Cloud Gaming e mantém a arquitetura aberta
para outros backends no futuro.

> ProjectNX não é afiliado, patrocinado nem aprovado pela Nintendo ou pela
> Microsoft. Nintendo Switch, Xbox e seus nomes relacionados pertencem aos
> respectivos titulares.

## Estado atual

Versão `0.3.1-preview`.

Esta versão realiza o primeiro login Microsoft real:

- inicialização como homebrew nativo;
- leitura de Joy-Cons e Pro Controller;
- navegação entre os estados principais;
- detecção de modo portátil ou dock;
- inicialização dos sockets da libnx;
- conexão HTTPS/TLS com o endpoint oficial do Microsoft Identity;
- validação do certificado do servidor;
- leitura segura de `config.ini`;
- solicitação OAuth 2.0 por código de dispositivo;
- exibição do endereço e do código no Switch;
- consulta automática e cancelável da autorização;
- tokens mantidos somente em memória durante este preview;
- tratamento básico de erros;
- núcleo de estados testável fora do Switch.

Ela ainda **não carrega o catálogo nem inicia uma transmissão real**. O login
confirma a conta Microsoft pelo fluxo oficial, mas a integração específica com
os serviços Xbox será pesquisada e implementada separadamente.

## Controles do preview

| Botão | Ação |
| --- | --- |
| `A` | Avançar, testar a rede e iniciar o login |
| `B` | Voltar ou cancelar o login pendente |
| `X` | Mostrar ou ocultar informações técnicas |
| `+` | Encerrar |

## Compilação

Requisitos:

- devkitPro;
- devkitA64;
- libnx;
- libcurl para Switch;
- `DEVKITPRO` configurado no ambiente.

Com a toolchain instalada:

```sh
make
```

O resultado será `ProjectNX.nro`. Para criar o pacote de instalação:

```sh
make package
```

O pacote será criado em `dist/projectnx/`, já com a estrutura correta para a raiz
do cartão SD.

### Build automático

Cada envio para a branch `main` dispara uma compilação no GitHub Actions usando
o ambiente devkitA64. Para baixar:

1. Abra a aba **Actions** do repositório.
2. Selecione a execução mais recente de **Build ProjectNX**.
3. Baixe o artefato `ProjectNX-v0.3.1-preview`.
4. Extraia o ZIP e copie a pasta `switch` para a raiz do cartão SD.

## Instalação no Switch

1. Copie a pasta `switch/projectnx` do pacote para a raiz do cartão SD.
2. Inicie o Homebrew Menu em **title mode**, segurando `R` enquanto abre um jogo.
3. Abra o ProjectNX.

O pacote já contém `config.ini` com o identificador público oficial do
ProjectNX. Ele não contém senha, segredo ou token.

O modo Álbum oferece memória limitada e não será suportado para streaming.

## Testes locais

O núcleo de estados não depende da libnx:

```sh
make test
```

Para verificar a estrutura do projeto:

```sh
make validate
```

## Documentação

- [Arquitetura](docs/ARCHITECTURE.md)
- [Roadmap](docs/ROADMAP.md)
- [Segurança e privacidade](docs/SECURITY.md)
- [Teste no Switch](docs/TESTING_ON_SWITCH.md)
