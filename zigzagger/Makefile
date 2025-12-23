CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

TARGET  = main

all: $(TARGET)

$(TARGET): main.c asm.S
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET)
	$(QEMU) ./$<

clean:
	rm -f $(TARGET)
