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
void Triangle(tgaImage *image, int x0, int y0, int x1, int y1, int x2, int y2, tgaColor color);

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




void Triangle(tgaImage *image, int x0, int y0, int x1, int y1, int x2, int y2, tgaColor color) {
    if(((y0 == y1) && (y1 == y2)) || ((x0 == x1) && (x1 == x2))) {
        #ifdef DEBUG
        printf("It's a point! Skip.\n");
        #endif

        return;
    }
    
    if(y0 > y2) {
        swap(&y0, &y2);
        swap(&x0, &x2);
    }
    if(y0 > y1) {
        swap(&y0, &y1);
        swap(&x0, &x1);
    }
    if(y1 > y2) {
        swap(&y1, &y2);
        swap(&x1, &x2);
    }

    if((y0 == y2) || ((x0 == x1) && (x1 == x2))) {

        #ifdef DEBUG
        printf("It's a line! Skip.\n");
        #endif

        return;
    }

    #ifdef DEBUG
    printf("y0 y1 y2 %d %d %d\n", y0, y1, y2);
    printf("DEBUG x0 x1 x2 %d %d %d\n", x0, x1, x2);
    #endif

    int xa, xb, y;
    for(y = y0; y <= y1; y++) {
        

        if((y1 - y0) != 0) {
            xa = x0 + (x1-x0) * (y - y0)/(y1 - y0);
        } else {
            xa = x0;
        }
        if((y2 - y0) != 0) {
            xb = x0 + (x2-x0) * (y - y0)/(y2 - y0);
        } else {
            xb = x0;
        }

        if(xa > xb) {
            swap(&xa, &xb);
        }
        #ifdef DEBUG
        printf("DEBUG xa xb %d %d\n", xa, xb);
        #endif

        for(int x = xa; x <= xb; x++) {
            #ifdef DEBUG
            printf("SET PIXEL X Y %d %d\n", x, y);
            #endif
            tgaSetPixel(image, x, y, color);
        }
    }
   	/*
	* Redefinition upper vertices (x, y)
    */
	y0 = y1 = y;
	x0 = xa;
	x1 = xb;

    #ifdef DEBUG
    printf("ADDITION x0 y0 %d %d; x1 y1 %d %d; x2 y2 %d %d\n", x0, y0, x1, y1, x2, y2);
    #endif

    for(int y = y0; y <= y2; y++) {
        
        if((y2 - y0) != 0) {
            xa = x0 + (x2 - x0)*(y - y0)/(y2 - y0);
        } else {
            xa = x0;
        }
        if((y2 - y1) != 0) {
            xb = x1 + (x2 - x1)*(y - y1)/(y2 - y1);

            #ifdef DEBUG
            printf("xb => y2 != y0 => %d\n", xb);
            #endif

        } else {
            xb = x0;
        }

      	if(xa > xb) {
            swap(&xa, &xb);
        }

        #ifdef DEBUG
        printf("DEBUG ADDITION xa xb %d %d\n", xa, xb);
        #endif

        for(int x = xa; x <= xb; x++) {

            #ifdef DEBUG
            printf("SET PIXEL ADDITION X Y %d %d\n", x, y);
            #endif

            tgaSetPixel(image, x, y, color);
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
    double lightDirection[3] = {0, 0, 1};
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
        for (k = 0; k < 3; k++){
            #ifdef COUNTER_DEBUG
                printf("a[k] & b[k] increment = %d\n", k);
            #endif
            a[k] = worldCoordinates[1][k] - worldCoordinates[0][k]; 
            #ifdef DEBUG
                printf("a[k] = %f\n", a[k]);
            #endif
            b[k] = worldCoordinates[2][k] - worldCoordinates[0][k];
            #ifdef DEBUG
                printf("b[k] = %f\n\n", b[k]);
            #endif
        }
        
        n[0] = a[1] * b[2] - a[2]*b[1]; 
        n[1] = -(a[0] * b[2] - a[2] * b[0]);
        n[2] = a[0] * b[1] - a[1] * b[0];

        #ifdef DEBUG
            printf("n[0,1,2] = %f, %f, %f\n", n[0], n[1], n[2]);
        #endif 

        double norm = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        double intens = ((lightDirection[0] * n[0] + lightDirection[1] * n[1] + lightDirection[2] * n[2])/norm);
        #ifdef DEBUG
            printf("Intensity = %f\n\n\n", intens);
        #endif
        
        j = 0;
        
        if (intens > 0){
            #ifdef DEBUG
                printf(" after intens screenCoordinates[j][0] = %d\n", screenCoordinats[j][0]);
                printf("after intens screenCoordinates[j][1] = %d\n\n", screenCoordinats[j][1]);
            #endif
            
            Triangle(image, screenCoordinats[j][0],screenCoordinats[j][1],
                            screenCoordinats[(j+1)%3][0], screenCoordinats[(j+1)%3][1],
                            screenCoordinats[(j+2)%3][0], screenCoordinats[(j+2)%3][1], tgaRGB(intens*255, intens*255, intens*255));
        }
    }
 
}

void swap(int *a, int *b){
    int t = *a;
    *a = *b;
    *b = t;
}
