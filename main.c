#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "tga.h"
#include "model.h"

#define WIDTH 1000
#define HEIGHT 1000
/*
#define DEBUG
#define COUNTER_DEBUG
#define DEBUG_COORDINATES
*/

void line(tgaImage *image, int x0, int y0, int x1, int y1, tgaColor color);
Model *scaleModel(Model *model, double scale);
void grid(tgaImage *image, Model *model, int scale);
void swap(int *a, int *b);

int main(int argc, char *argv[]){
    int sysExit = 0;
    Model *model = loadFromObj(argv[1]);
    int scale = atoi(argv[3]);
    scaleModel(model, strtod(argv[3], NULL));
    tgaImage *image = tgaNewImage(WIDTH, HEIGHT, RGB);
    grid(image, model, scale);
    if (-1 == tgaSaveToFile(image, argv[2])) {
        perror("tgaSaveToFile");
        sysExit = -1;
    }
    if (argc < 4){
        perror("Need more parameters! ./... *.obj *.tga scale");
        sysExit = -1;
    }
    
    tgaFreeImage(image);
    freeModel(model);
    return sysExit;
}

void line(tgaImage *image, int x0, int y0, int x1, int y1, tgaColor color){
    char steep = 0;
    if (abs(x0 - x1) < abs(y0 - y1)){
        swap(&x0, &y0);
        swap(&x1, &y1);
        steep = 1;
    }
    if (x0 > x1){
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int errorAccumulation = 0;
    int deltaError = 2 * abs(y1 - y0);
    int y = y0;
    for (int x = x0; x <= x1; x++){
        if (steep == 1){
            tgaSetPixel(image, (unsigned int)y, (unsigned int)x, color);
        } else{
            tgaSetPixel(image, (unsigned int)x, (unsigned int)y, color);
        }
        errorAccumulation += deltaError;

        if (errorAccumulation > (x1 - x0)){
            y += (y1 > y0?1:-1);
            errorAccumulation -= 2 * (x1-x0);
        }
    }
} 

Model *scaleModel(Model *model, double scale){
    for (unsigned i = 0; i < model->nvert; i++){
        for (unsigned j = 0; j < 3; j++){
            (model->vertices[i])[j] = (model->vertices[i])[j] * scale;
        }
    }
    return model;
}

void grid(tgaImage *image, Model *model, int scale){
    int i, j, k;
    tgaColor white = tgaRGB(255, 255, 255);
    for (i = 0; i < model->nface; ++i){
        #ifdef COUNTER_DEBUG
            printf("Nfaces increment = %d\n", i);
        #endif

        double a[3], b[3], n[3];
        int screenCoordinats[3][3];
        double worldCoordinates[3][3];
        for (int j = 0; j < 3; ++j){
            Vec3 *v = &(model->vertices[model->faces[i][3*j]]);
            #ifdef COUNTER_DEBUG
                printf("Vertices increment = %d\n", j);
            #endif
            screenCoordinats[j][0] = ((*v)[0] + 1) * image->width / 2;
            screenCoordinats[j][1] = (1 - (*v)[1]) * image->height / 2 + scale * 500; 
            screenCoordinats[j][2] = ((*v)[2] + 1) * image->width / 2;
            worldCoordinates[j][0] = (*v)[0]; 
            worldCoordinates[j][1] = (*v)[1]; 
            worldCoordinates[j][2] = (*v)[2]; 
            #ifdef DEBUG
                printf("screenCoordinates[j][0] = %d\n", screenCoordinats[j][0]);
                printf("screenCoordinates[j][1] = %d\n\n", screenCoordinats[j][1]);
                printf("worldCoordinates[j][0] = %f\n", worldCoordinates[j][0]);
                printf("worldCoordinates[j][1] = %f\n", worldCoordinates[j][1]);
                printf("worldCoordinates[j][2] = %f\n\n", worldCoordinates[j][2]);
            #endif
        }

        //Brezenham realization of mesh grid 
        for (j = 0; j < 3; ++j){
            line(image, screenCoordinats[j][0], screenCoordinats[j][1],
                 screenCoordinats[(j + 1) % 3][0], screenCoordinats[(j + 1) % 3][1], white);
        }
    }
 
}

void swap(int *a, int *b){
    int t = *a;
    *a = *b;
    *b = t;
}
