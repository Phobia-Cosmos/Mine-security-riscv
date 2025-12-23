CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static

all: flush-fault flush-ret

flush-fault: flush-fault.c
	$(CC) $(CFLAGS) $^ -o $@

flush-ret: flush-ret.c
	$(CC) $(CFLAGS) $^ -o $@

run-fault: flush-fault
	$(QEMU) ./$<

run-ret: flush-ret
	$(QEMU) ./$<

clean:
	rm -f flush-ret flush-fault *.csv