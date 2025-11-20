FROM ubuntu:22.04

# Evita perguntas durante a instalação (ex: timezone)
ENV DEBIAN_FRONTEND=noninteractive

# Instala as dependências
# gcc-multilib: para compilar 32-bits em máquina 64-bits
# qemu-system-x86: o emulador
# gdb: para debug
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-multilib \
    qemu-system-x86 \
    gdb \
    git \
    && rm -rf /var/lib/apt/lists/*

# Define o diretório de trabalho dentro do container
WORKDIR /xv6

# Comando padrão ao abrir o container
CMD ["/bin/bash"]
