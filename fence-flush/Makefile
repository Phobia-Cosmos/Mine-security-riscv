CC      = /opt/riscv/bin/riscv64-unknown-linux-gnu-gcc
QEMU    = qemu-riscv64
CFLAGS  = -static -Os

TARGET  = hist

all: $(TARGET)

$(TARGET): hist.c
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET)
	$(QEMU) ./$<

<<<<<<< HEAD
=======
hist: hist.c
	riscv64-unknown-elf-gcc hist.c -Os -o hist
	
>>>>>>> 45d35b0fb90fd08f29abf4736ef5add032dc08e3
clean:
	rm -f $(TARGET)