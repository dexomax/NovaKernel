# 🌌 NovaKernel
**A lightweight, 32-bit x86 Protected Mode operating system core.**

NovaKernel is a freestanding OS featuring a custom kernel engine and a structural **100-command paginated shell**. It is designed for speed, simplicity, and a retro "matrix" aesthetic.

---

## 🚀 Quick Start

### 1. Prerequisites
Ensure you have the necessary toolchain installed on your host system:

* **Assembler:** NASM
* **Compiler:** GCC (with 32-bit support)
* **Emulator:** QEMU

### 2. Build and Boot
Run the following command in your terminal to compile the assembly bootloader, the C kernel, and launch the system:

```bash
nasm -f elf32 boot.asm -o boot.o && \
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra && \
ld -m elf_i386 -T linker.ld -o mykernel.bin boot.o kernel.o && \
qemu-system-i386 -kernel mykernel.bin
