CC?=gcc
CFLAGS?=-std=c99 -O2 -fPIC
AR?=ar

SRC=src/core/gf_core.c
OBJ=$(SRC:.c=.o)
INCLUDES=-Iinclude

libgf_sdk.a: $(OBJ)
	$(AR) rcs $@ $^

libgf_sdk.so: $(OBJ)
	$(CC) -shared -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	 rm -f $(OBJ) libgf_sdk.a libgf_sdk.so
