CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

all: spectre

spectre: spectre.c
	$(CC) $(CFLAGS) $^ -o $@

run: spectre
	$(QEMU) ./$<

clean:
	rm -f spectre