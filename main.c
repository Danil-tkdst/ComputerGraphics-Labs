#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "tga.h"
#include "model.h"

#define WIDTH 1000
#define HEIGHT 1000
#define TEXTWIDTH 512
#define TEXTHEIGHT 1024

int zbuffer[WIDTH * HEIGHT];
void line(tgaImage *image, int x0, int y0, int x1, int y1, tgaColor color);
Model *scaleModel(Model *model, double scale);
void grid(tgaImage *image, Model *model, int scale, tgaImage *texture);
void swap(int *a, int *b);
void Triangle(tgaImage *image, tgaImage *texture,
              int x0,  int y0,  int z0,  int x1,  int y1,  int z1,  int x2,  int y2,  int z2,
              int tx0, int ty0,          int tx1, int ty1,          int tx2, int ty2, double intens,  int zbuffer[WIDTH*HEIGHT]);
int main(int argc, char *argv[]){
    int sysExit = 0;
    Model *model = loadFromObj(argv[1]);
    int scale = atoi(argv[3]);
    scaleModel(model, strtod(argv[3], NULL));
    tgaImage *image = tgaNewImage(WIDTH, HEIGHT, RGB);
    tgaImage *texture = tgaLoadFromFile(argv[4]);
    grid(image, model, scale, texture);
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




void Triangle(tgaImage *image, tgaImage *texture,
              int x0,  int y0,  int z0,  int x1,  int y1,  int z1,  int x2,  int y2,  int z2,
              int tx0, int ty0,          int tx1, int ty1,          int tx2, int ty2, double intens,  int zbuffer[WIDTH*HEIGHT]){
    int xa, xb, za, zb, tx, ty, txa, txb;
    if(y0 > y2) {
        swap(&y0, &y2); swap(&ty0, &ty2);
        swap(&x0, &x2); swap(&tx0, &tx2);
    }
    if(y0 > y1) {
        swap(&y0, &y1); swap(&ty0, &ty1);
        swap(&x0, &x1); swap(&tx0, &tx1);
    }
    if(y1 > y2) {
        swap(&y1, &y2); swap(&ty1, &ty2);
        swap(&x1, &x2); swap(&tx1, &tx2);
    }
    for (int y = y0; y <= y1; y++){
        ty = ty0;
        if ((y1 - y0) != 0){
        xa = x0 + (x1 - x0) * (double)(y - y0) / (y1 - y0);
        txa = tx0 + (tx1 - tx0) * (double)(ty - ty0) / (ty1 - y0);
        za = z0 + (z1 - z0) * (double)(y - y0) / (y1 - y0);
        }else{
            xa = x0;
            txa = tx0;
        }
        if ((y2 - y0) != 0){
            xb = x0 + (x2 - x0) * (double)(y - y0) / (y2 - y0);
            txb = tx0 + (tx2 - tx0) * (double)(ty - ty0) / (ty2 - ty0);
            zb = z0 + (z2 - z0) * (double)(y - y0) / (y2 - y0);
        }else{
            xb = x0;
            txb = tx0;
        }
        if (xa > xb){
            swap(&xa, &xb);
            swap(&txa, &txb);
        }
        ty = abs((int)(ty1 - (y1 - y) * (ty1 - ty0) / (y1 - y0)));
        for (int x = xa; x <= xb; x++){
            int idx = x +y*WIDTH;
            int z = za + (zb - za) * (double)(x - xa) / (xb - xa);
            if (zbuffer[idx] < z){
                tx = abs((int)(txb - (xb - x) * (txb - txa) / (xb - xa)));
                tgaColor color = tgaGetPixel(texture, tx, ty);
                zbuffer[idx] = z;
                color = tgaRGB(Red(color) * intens, Green(color) * intens, Blue(color) * intens);
                tgaSetPixel(image, x, y, color);
            }
            
        }
    }

    for(int y = y1; y <= y2; y++){
        if ((y1 - y0) != 0){
            xa = x0 + (x2 - x0) * (double)(y - y0) / (y2 - y0);
            za = z0 + (z1 - z0) * (double)(y - y0) / (y1 - y0);
        }else{
            xa = x0;
        }
        if ((y2 - y0) != 0){
            xb = x1 + (x2 - x1) * (double)(y - y1) / (y2 - y1 + 1);
            zb = z0 + (z2 - z0) * (double)(y - y0) / (y2 - y0);
        }else{
            xb = x0;
        }
        if (xa > xb){
            swap(&xa, &xb);
        }
        ty = abs((int)(ty2 - (y2 - y) * (ty2 - ty1) / (y2 - y1)));
        for (int x = xa; x <= xb; x++){
            int idx = x +y*WIDTH;
            int z = za + (zb - za) * (double)(x - xa) / (xb - xa);
            if (zbuffer[idx] < z){
                tx = abs((int)(txb - (xb - x) * (txb - txa) / (xb - xa)));
                tgaColor color = tgaGetPixel(texture, tx, ty);
                zbuffer[idx] = z;
                color = tgaRGB(Red(color) * intens, Green(color) * intens, Blue(color) * intens);
                tgaSetPixel(image, x, y, color);
            }
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

void grid(tgaImage *image, Model *model, int scale, tgaImage *texture){
    int i, j, k;
    double x, y, z, x_ex, y_ex, z_ex;
    double lightDirection[3] = {1, 1/2, 1};
    //double c1 = -100;
    //double c2 = -5;
    double c3 = -10000;
    double alpha = -20, pi = 3.1415926, sin_a, cos_a;
    alpha = alpha * pi / 180;
    sin_a = sin(alpha);
    cos_a = cos(alpha);
    for (int i = 0; i < WIDTH * HEIGHT; i++){
            zbuffer[i] = INT_MIN;
    }
    for (i = 0; i < model->nface; ++i){
        double a[3], b[3], n[3];
        int screenCoordinats[3][3];
        int textCoordinates[3][3];
        double worldCoordinates[3][3];
        for (int j = 0; j < 3; ++j){
            Vec3 *v = &(model->vertices[model->faces[i][3*j]]);
            x = ((*v)[0] + 1);
            y = (1 - (*v)[1]); 
            z = ((*v)[2] + 1);

            // x rotation:
                y_ex = y;
                y = y_ex * cos_a - z * sin_a;
                z = y_ex * sin_a + z * cos_a;

            // y rotation
                x_ex = x;
                x = x_ex * cos_a + z * sin_a;
                z = -x_ex * sin_a + z * cos_a;

            // z rotation
            /*    x_ex = x;
                x = x_ex * cos_a - y * sin_a;
                y = x_ex * sin_a + y * cos_a;*/

            /*screenCoordinats[j][0] = x / (1 - x/c1 - y/c2 - z/c3);
            screenCoordinats[j][1] = y / (1 - x/c1 - y/c2 - z/c3);
            screenCoordinats[j][2] = z / (1 - x/c1 - y/c2 - z/c3);*/
            screenCoordinats[j][0] = x / (1 - z/c3)  * image->width / 4 + scale * 400;
            screenCoordinats[j][1] = y / (1 - z/c3) * image->height / 4 + scale * 300;
            screenCoordinats[j][2] = z / (1 - z/c3) * image->width / 4;

            worldCoordinates[j][0] = (*v)[0]; 
            worldCoordinates[j][1] = (*v)[1]; 
            worldCoordinates[j][2] = (*v)[2];
            //printf("%d  ",screenCoordinats[j][0]); printf("%d  ",screenCoordinats[j][1]); printf("%d\n",screenCoordinats[j][2]);
            
            Vec3 *t = &(model->textures[model->faces[i][3*j + 1]]);
            textCoordinates[j][0] = (1 - (*t)[0]) * TEXTWIDTH;
            textCoordinates[j][1] = (1 - (*t)[1]) * TEXTHEIGHT; 
        }
        for (k = 0; k < 3; k++){
            a[k] = worldCoordinates[1][k] - worldCoordinates[0][k]; 
            b[k] = worldCoordinates[2][k] - worldCoordinates[0][k];
        }
        n[0] = a[1] * b[2] - a[2]*b[1]; 
        n[1] = -(a[0] * b[2] - a[2] * b[0]);
        n[2] = a[0] * b[1] - a[1] * b[0];

        double norm = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        double intens = ((lightDirection[0] * n[0] + lightDirection[1] * n[1] + lightDirection[2] * n[2])/norm);
        j = 0;
        
        if (intens > 0){
            Triangle(image, texture, screenCoordinats[j][0],screenCoordinats[j][1], screenCoordinats[j][2],
                screenCoordinats[(j+1)%3][0], screenCoordinats[(j+1)%3][1], screenCoordinats[(j+1)%3][2],
                screenCoordinats[(j+2)%3][0], screenCoordinats[(j+2)%3][1], screenCoordinats[(j+2)%3][2], 
                textCoordinates[j][0], textCoordinates[j][1],
                textCoordinates[(j+1)%3][0], textCoordinates[(j+1)%3][1],
                textCoordinates[(j+2)%3][0], textCoordinates[(j+2)%3][1], intens, zbuffer);
        }
    }
}

void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}
