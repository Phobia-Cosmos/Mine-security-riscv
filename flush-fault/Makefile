all: flush-fault flush-ret

flush-fault: flush-fault.c
	/bin/x86_64-linux-gnu-gcc-13 flush-fault.c -o flush-fault

flush-ret: flush-ret.c
	/bin/x86_64-linux-gnu-gcc-13 flush-ret.c -o flush-ret


clean:
	rm -f flush-ret flush-fault *.csv
