all:
	gcc -Wall main.c tga.c model.c -o main
	./main obj/cat.obj ./res.tga 1.9 ./obj/cat_diff.tga