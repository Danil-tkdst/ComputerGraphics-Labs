all:
	gcc main.c tga.c model.c -o main
	.\main .\obj\Yoshi2.obj .\res.tga 1