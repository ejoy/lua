# for mingw

LUAINC = -I../src
LUALIB = -L../src/ -llua54
CFLAGS = -Wall -g
SHARED = --shared

all : sharetable.dll multivm.dll

sharetable.dll : sharetable.c
	gcc $(CFLAGS) $(SHARED) -o $@ $^ $(LUAINC) $(LUALIB)

multivm.dll : multivm.c
	gcc $(CFLAGS) $(SHARED) -o $@ $^ $(LUAINC) $(LUALIB)

clean :
	rm sharetable.dll multivm.dll

test :
	../src/lua.exe test.lua

