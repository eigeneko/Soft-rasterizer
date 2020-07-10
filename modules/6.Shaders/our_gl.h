#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

extern Model* model;
extern const int height;
extern const int width;
extern Vec3f light_dir;
extern Vec3f eye;
extern Vec3f up;
extern Vec3f center;

extern Matrix ModelView;
extern Matrix Projection;
extern Matrix Viewport;

void viewport(int x, int y, int w, int h);
void projection(float coeff=0.f);
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader
{
    virtual ~IShader();
    virtual Vec4f vertex(int nthface, int nthvert) = 0;
    virtual bool fragment(Vec3f barCoordinate, TGAColor& color) = 0;
};

void triangle(Vec4f* points, IShader& shader, TGAImage& image, TGAImage& zbuffer);

#endif