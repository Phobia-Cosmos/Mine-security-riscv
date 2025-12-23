CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

all: main

main: main.c
	$(CC) $(CFLAGS) $^ -o $@

run: main
	$(QEMU) ./$<

clean:
	rm -f main cycles.csv timings.csv