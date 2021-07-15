all: main

run: 
	./main

emu: pat8.c pat8.h pat8emu.c pat8emu.h
	gcc pat8.c pat8emu.c -lSDL2 -lm -o pat8emu

asm: pat8.c pat8.h pat8asm.c
	gcc pat8.c pat8asm.c -lm -o pat8asm

main: pat8.c pat8.h pat8emu.c pat8emu.h
	gcc pat8.c pat8emu.c -lSDL2 -lm -o pat8emu

clean:
	rm pat8emu