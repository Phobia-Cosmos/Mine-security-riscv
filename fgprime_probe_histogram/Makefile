CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

TARGET  = hist

all: $(TARGET)

$(TARGET): hist.c
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET)
	$(QEMU) ./$<

clean:
	rm -f $(TARGET)