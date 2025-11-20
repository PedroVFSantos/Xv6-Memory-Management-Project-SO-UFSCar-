# Xv6 Memory Management

**Disciplina:** Sistemas Operacionais (UFSCar)
**Professora:** Kelen Vivaldini
**Aluno/Grupo:** Pedro Santos

Implementação de melhorias no gerenciamento de memória do kernel Xv6. O projeto utiliza **Docker** para garantir a compilação correta do ambiente 32-bits em qualquer sistema operacional.

## 📂 Estrutura

O projeto foi dividido em diretórios independentes para facilitar a correção:

- **`task1/`**: Visualizador de Page Tables (`Ctrl+P` detalhado).
- **`task2/`**: Proteção contra Null Pointer (página 0 inválida).
- **`task3/`**: Segmentos Read-Only (proteção de código).
- **`task4/`**: Implementação de Copy-on-Write (CoW) no `fork()`.

---

## 🛠️ Como Executar (Docker)

Recomendado para evitar erros de versão do GCC ou arquitetura (compatível com Windows, Linux e Mac M1/M2).

### 1. Build da Imagem
Na raiz do projeto:
```bash
docker build -t xv6-env .
