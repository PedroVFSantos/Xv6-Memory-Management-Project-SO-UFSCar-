# Xv6 Memory Management

**Disciplina:** Sistemas Operacionais UFSCar
**Professora:** Kelen Vivaldini
**Aluno/Grupo:** Pedro Santos, Rennan Lopes, Lucas Martinez, Gustavo Bragaia, Henrique Pique, Felipe Yacobian e Daniel Gidrão

Implementação de melhorias no gerenciamento de memória do kernel Xv6. O projeto utiliza **Docker** para garantir a compilação correta do ambiente 32-bits em qualquer sistema operacional.

## Estrutura

O projeto foi dividido em diretórios independentes para facilitar a correção:

- **`task1/`**: Visualizador de Page Tables (`Ctrl+P` detalhado).
- **`task2/`**: Proteção contra Null Pointer (página 0 inválida).
- **`task3/`**: Segmentos Read-Only (proteção de código).
- **`task4/`**: Implementação de Copy-on-Write (CoW) no `fork()`.
---

## Como Executar (Docker)
Recomendado para evitar erros de versão do GCC ou arquitetura

### 1. Build da Imagem
Na raiz do projeto:

```bash
docker build -t xv6-env .

```

### 2. Rodar o Container
**Linux/Mac:**
```bash
docker run --platform linux/amd64 -it --rm -v $(pwd):/xv6 xv6-env
```
** Windows (Powershell)
```bash
docker run --platform linux/amd64 -it --rm -v ${PWD}:/xv6 xv6-env
```

## Verificação das Tasks sugeridas

Dentro do terminal do container, navegue até a pasta da tarefa desejada (`cd taskX`) e execute `make qemu-nox` para iniciar o sistema.

### Task 1: Visualização
- **Teste:** Pressione `Ctrl+P` dentro do Xv6.
- **Resultado:** Exibe o mapeamento detalhado das Page Tables e Page Directories.

### Task 2: Null Pointer
- **Teste:** Execute o comando `nulltest` ou tente acessar o endereço 0 via código.
- **Resultado:** O processo deve ser encerrado com um **trap** ao tentar acessar o endereço 0.

### Task 3: Read-Only Segments
- **Teste:** Execute o comando `rotest` ou tente escrever na função `main`.
- **Resultado:** O processo deve ser encerrado com um **trap** ao tentar escrever no segmento de código (`.text`).

### Task 4: Copy-on-Write
- **Teste:** Utilize o sistema normalmente (ex: `ls`, `sh`, `usertests`).
- **Resultado:** O sistema funciona sem travamentos. Logs de depuração (ex: "COW: Copiando...") podem aparecer no console indicando a alocação tardia de páginas.

---

## Comandos Úteis

- **Sair do Xv6 (QEMU):** Pressione `Ctrl` + `A`, solte as teclas e pressione `x`.
- **Sair do Container:** Digite `exit`.
- **Limpar compilação:** `make clean`.
