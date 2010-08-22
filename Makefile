CFLAGS += -pedantic -ansi
CFLAGS += -Wall -Wstrict-prototypes -Wmissing-prototypes
CFLAGS += -Wmissing-declarations -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS += -Wsign-compare -Werror

nanobfi: nanobfi.c

clean:
	rm -f nanobfi nanobfi.o
