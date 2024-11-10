build:
	gcc -o super_jump.exe src\main.c src\input.c -lraylib -lgdi32 -lwinmm -L lib -I include