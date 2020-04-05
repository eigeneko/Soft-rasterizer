#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const int width  = 3000;
const int height = 3000;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
Model *model = NULL;

void line(int, int, int, int, TGAImage &, TGAColor);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool reverse = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)){
        reverse = true;
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1){
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    int y  = y0;
    int error = std::abs(dy) * 2;
    int delta = 0;
    int stepY = y1 > y0 ? 1 : -1;

    for(int x = x0; x <= x1; x++){
        if (reverse)
            image.set(y, x, color);
        else
            image.set(x, y, color);

        delta += error;  

        if (delta > dx){
            y += stepY;
            delta -= 2 * dx;
        }
    }
}

void traingle(int x0, int y0, int x1, int y1, int x2, int y2, TGAImage &image, TGAColor color)
{
    line(x0, y0, x1, y1, image, color);
    line(x0, y0, x2, y2, image, color);
    line(x1, y1, x2, y2, image, color);
}

void drawTraingles(TGAImage &image)
{
    Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    // traingle(t0[0], t0[1], t0[2], image, red); 
    // traingle(t1[0], t1[1], t1[2], image, white); 
    // traingle(t2[0], t2[1], t2[2], image, green);

    image.flip_vertically();
    image.write_tga_file("2.tga");
}

void drawWire(TGAImage &image, Model *model)
{
    
    for (int i = 0; i < model->nfaces(); i++) {
        // 取模型的每个面（三角形）
        std::vector<int> face = model->face(i);
        // 取构成这个三角形的相邻两个顶点，画一条线
        for (int j = 0; j < 3; j ++) {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j+1)%3]);
            //std::cout << v0.x << ":" << v0.y << std::endl;
            /*
            顶点坐标转换为屏幕坐标（为什么要这么计算）
            +1.是为了把坐标系原点移到屏幕中心，
            -0.458378:0.268123(一个顶点的坐标)
            */
            int x0 = (v0.x + 1) * width  / 2.;
            int y0 = (v0.y + 1.) * height / 2.;
            int x1 = (v1.x + 1.) * width  / 2.;
            int y1 = (v1.y + 1.) * height / 2.;
            /*
            为什么是顶点坐标 *width/2, *height/2 而不是*width, *height
            换成如下，会发现只是影响到了透视
            */
            // int x0 = (v0.x + .5) * width;
            // int y0 = (v0.y + .5) * height;
            // int x1 = (v1.x + .5) * width;
            // int y1 = (v1.y + .5) * height;
            line(x0, y0, x1, y1, image, white);
        }
    }
    image.flip_vertically();
    image.write_tga_file("drawWire.tga");
}

// int main(int argc, char** argv)
// {
//     if (argc == 2)
//         model = new Model(argv[1]);
//     else
//         model = new Model("obj/african_head/african_head.obj");
//     TGAImage image(width, height, TGAImage::RGB);

//     // drawWire(image, model);
//     drawTraingles(image);

//     delete model;
//     return 0;
// }