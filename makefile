build:
	gcc -o fn_super_jumper.exe src\main.c src\input.c -lraylib -lgdi32 -lwinmm -L lib -I include