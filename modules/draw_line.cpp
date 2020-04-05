#include "tgaimage.h"
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void TGAImageTest(TGAImage &);
void DrawLine1(int, int, int, int, TGAImage &, TGAColor);
void DrawLine2(int, int, int, int, TGAImage &, TGAColor);
void DrawLine3(int, int, int, int, TGAImage &, TGAColor);
void DrawLine4(int, int, int, int, TGAImage &, TGAColor);

void DrawLine1Test();
void DrawLine2Test();
void DrawLine3Test();
void DrawLine4Test();


int main(int argc, char** argv)
{
    // TGAImageTest();
    // DrawLine1Test();
    // DrawLine2Test();
    // DrawLine3Test();
    DrawLine4Test();
}

// ----------------------------------------------------------------------------
//    Lesson 0
// ----------------------------------------------------------------------------

void TGAImageTest(TGAImage &image)
{
    image.set(10, 90, red);
    image.write_tga_file("翻转前.tga");
    image.flip_vertically();
    image.write_tga_file("翻转后.tga");
}

// ----------------------------------------------------------------------------
//    Lesson 1 
// ---------------------------------------------------------------------------

/* 所画的直线中会出现相邻两个像素x相同，y不同，造成两个像素竖直相连的情况 */
void DrawLine1(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    for (float t = .0; t <= 1.; t +=.01){
        int x = x0 + (x1 - x0) * t;
        int y = y0 + (y1 - y0) * t;
        image.set(x, y, color);
    }
}

void DrawLine1Test()
{
    TGAImage image(100, 100, TGAImage::RGB);
    DrawLine1(10, 10, 70, 90, image, white);
    image.flip_vertically();
    image.write_tga_file("DrawLine1.tga");
}

/* 如果x0比x1大，这个函数根本画不出线 */
/* 如果斜率比较大，y会变得非常稀疏 */
void DrawLine2(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    for (int x = x0; x <= x1; x++){
        float t = (x - x0) / float(x1 - x0);
        int y = y0 * (1-t) + y1 * t;
        image.set(x, y, color);
    }
}

void DrawLine2Test()
{
    TGAImage image(100, 100, TGAImage::RGB);
    DrawLine2(13, 20, 80, 40, image, white); 
    DrawLine2(20, 13, 40, 80, image, red); 
    DrawLine2(80, 40, 13, 20, image, red);
    image.flip_vertically();
    image.write_tga_file("DrawLine2.tga");
}

/* 修复2中出现的问题，取dx，dy中较大的方向进行迭代 */
/* 选取的迭代方向决定了像素点的个数（dx或者dy）*/

void DrawLine3(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool reverse = false;

    if (std::abs(x0 - x1) < std::abs(y0 - y1)){ // along the steep axis
        std::swap(x0, y0);
        std::swap(x1, y1);
        reverse = true;
    }

    if (x0 > x1){ // make sure the increment from x0 to x1 is positive
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    
    for (int x = x0; x <= x1; x++){
        float t = (x - x0) / (float)(x1 - x0);
        int   y =  y0 * (1. - t) + y1 * t;
        if (reverse)
            image.set(y, x, color); // de-transpose because now x is actually y.
        else
            image.set(x, y, color);
    }
}

void DrawLine3Test()
{
    TGAImage image(100, 100, TGAImage::RGB);
    DrawLine3(13, 20, 80, 40, image, white); 
    DrawLine3(20, 13, 40, 80, image, red); 
    DrawLine3(80, 40, 13, 20, image, red);
    image.flip_vertically();
    image.write_tga_file("DrawLine3.tga");    
}

/* 优化，提前计算好斜率，只计算y的增量 */
void DrawLine4(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool reverse = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)){
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
    float error = std::abs(dy/float(dx));
    float delta = 0;
    for(int x = x0; x <= x1; x++){
        if (reverse)
            image.set(y, x, color);
        else
            image.set(x, y, color);

        delta += error;    

        if (delta > .5){
            y += y1 > y0 ? 1 : -1;
            delta -= 1.;
        }
    }
}

void DrawLine4Test()
{
   TGAImage image(100, 100, TGAImage::RGB);
   DrawLine3(13, 20, 80, 40, image, white); 
   DrawLine3(20, 13, 40, 80, image, red); 
   DrawLine3(80, 40, 13, 20, image, red);
   image.flip_vertically();
   image.write_tga_file("DrawLine4.tga");   
}

/* 
画线函数的最终版
error 代表x每增加1，y方向的增量
优化1 ：把error和delta从float变为int类型，同时扩大2*dx两倍，delta > .5 变为 2*dx*delta > dx, 这要求组成delta的error也扩大2*dx倍
优化2 ：提前计算好循环内y的增量为1还是-1，不用每次都去判断y0和y1的大小
优化3 ：reverse的判断移到循环外（编译器可以帮我们进行这个优化，所以为了保持代码的简短，这里在写法上还是维持原状，但需要知道实际上有这个优化）
compilers are smart enough today, they (at any optimization level) will do what (supposedly is called) loop unhoisting
to figure out that, if the variable is not a global one (therefore thread safe), it will move it out of the for loop. 
*/

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool reverse = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)){
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