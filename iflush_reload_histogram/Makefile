CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

all: hist

hist: hist.c
	$(CC) $(CFLAGS) -DC906 $^ -o $@

run: hist
	$(QEMU) ./$<

clean:
	rm -f hist