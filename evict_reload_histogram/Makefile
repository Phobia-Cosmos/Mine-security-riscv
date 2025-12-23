CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

all: hist-c906 hist-u74

hist-c906: hist.c
	$(CC) $(CFLAGS) -DC906 $^ -o $@

hist-u74: hist.c
	$(CC) $(CFLAGS) -DU74 $^ -o $@

# 运行hist-c906（默认）
run: hist-c906
	$(QEMU) ./$<

# 运行hist-u74
run-u74: hist-u74
	$(QEMU) ./$<

# 运行hist-c906
run-c906: hist-c906
	$(QEMU) ./$<

clean:
	rm -f hist-c906 hist-u74