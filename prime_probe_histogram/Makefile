CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

all: hist-c906 hist-u74

hist-c906: hist.c
	$(CC) $(CFLAGS) -DC906 $^ -o $@

hist-u74: hist.c
	$(CC) $(CFLAGS) -DU74 $^ -o $@

run-c906: hist-c906
	$(QEMU) ./$<

run-u74: hist-u74
	$(QEMU) ./$<

run: hist-c906
	$(QEMU) ./$<

clean:
	rm -f hist-c906 hist-u74