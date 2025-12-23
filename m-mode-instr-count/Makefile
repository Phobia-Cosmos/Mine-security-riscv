CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static

all: example

example: example.c
	$(CC) $(CFLAGS) $^ -o $@

run: example
	$(QEMU) ./$<

clean:
	rm -f example