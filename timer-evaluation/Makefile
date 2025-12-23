CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

all: main timer-resolution

main: main.c
	$(CC) $(CFLAGS) $^ -o $@

timer-resolution: timer-resolution.c
	$(CC) $(CFLAGS) $^ -o $@

run-main: main
	$(QEMU) ./$<

run-timer: timer-resolution
	$(QEMU) ./$<

run: main
	$(QEMU) ./$<

clean:
	rm -f main timer-resolution