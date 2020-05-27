/* 
Line Sweeping Algorithm
--------------------------------------------------------------------------------
1. Sort vertices of the triangle by their y-coordinates;
2. Rasterize simultaneously the left and the right sides of the triangle;
3. Draw a horizontal line segment between the left and the right boundary points.
*/

#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 2000;
const int height = 2000;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y))
    {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x > p1.x)
    {
        std::swap(p0, p1);
    }

    for (int x = p0.x; x <= p1.x; x++)
    {
        float t = (x - p0.x) / (float)(p1.x - p0.x);
        int y = p0.y * (1. - t) + p1.y * t;
        if (steep)
        {
            image.set(y, x, color);
        }
        else
        {
            image.set(x, y, color);
        }
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
    line(t0, t1, image, color);
    line(t1, t2, image, color);
    line(t2, t0, image, color);
}

void LineSweep(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
    if (t0.y > t1.y)
        std::swap(t0, t1);
    if (t0.y > t2.y)
        std::swap(t0, t2);
    if (t1.y > t2.y)
        std::swap(t1, t2);

    std::cout << t0 << std::endl;
    std::cout << t1 << std::endl;
    std::cout << t2 << std::endl;

    float fullHeight = t2.y - t0.y;
    float halfHeight;
    halfHeight = t1.y - t0.y;

    for (int y = t0.y; y <= t1.y; y++)
    {
        Vec2i interSectionPoint1 = t0 + (t2 - t0) * (float)(y - t0.y) / fullHeight;
        Vec2i interSectionPoint2 = t0 + (t1 - t0) * (float)(y - t0.y) / halfHeight;
        std::cout << "draw bottom part : A " << interSectionPoint1 << " B : " << interSectionPoint2 << std::endl;
    //    line(interSectionPoint1, interSectionPoint2, image, color);

        /*
       直接调用line画线会出现三角形内部有些像素没有填满的问题
       draw bottom part : A 180 150  B : 180 150 
       draw bottom part : A 179 151  B : 174 151 
       draw bottom part : A 177 152  B : 168 152
       */

        if (interSectionPoint1.x > interSectionPoint2.x)
            std::swap(interSectionPoint1, interSectionPoint2);

        for (int x = interSectionPoint1.x; x <= interSectionPoint2.x; x++)
            image.set(x, y, color);
    }

    halfHeight = t2.y - t1.y;
    for (int y = t1.y; y <= t2.y; y++)
    {
        Vec2i interSectionPoint1 = t0 + (t2 - t0) * (y - t0.y) / fullHeight;
        Vec2i interSectionPoint2 = t1 + (t2 - t1) * (y - t1.y) / halfHeight;
         if (interSectionPoint1.x > interSectionPoint2.x)
            std::swap(interSectionPoint1, interSectionPoint2);

        for (int x = interSectionPoint1.x; x <= interSectionPoint2.x; x++)
            image.set(x, y, color);
    }
}

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);

    int scale = 10;

    Vec2i t0[3] = {Vec2i(10, 70) * scale, Vec2i(50, 160) * scale, Vec2i(70, 80) * scale};
    Vec2i t1[3] = {Vec2i(180, 50) * scale, Vec2i(150, 1) * scale , Vec2i(70, 180) * scale};
    Vec2i t2[3] = {Vec2i(180, 150) * scale, Vec2i(120, 160) * scale, Vec2i(130, 180) *scale};

    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);

    LineSweep(t0[0], t0[1], t0[2], image, green);
    LineSweep(t1[0], t1[1], t1[2], image, red);
    LineSweep(t2[0], t2[1], t2[2], image, green);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}