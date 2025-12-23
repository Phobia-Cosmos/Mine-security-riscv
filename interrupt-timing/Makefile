CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

all: attacker

attacker: attacker.c
	$(CC) $(CFLAGS) $^ -o $@

run: attacker
	$(QEMU) ./$<

clean:
	rm -f attacker