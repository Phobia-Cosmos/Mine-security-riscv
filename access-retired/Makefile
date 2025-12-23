CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

TARGET  = main

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET)
	$(QEMU) ./$<

prepare:
	mkdir sec
	touch sec/foo
	chmod 0000 sec/foo
	chmod 0111 sec

clean:
	rm -f $(TARGET)
	sudo rm -rf sec