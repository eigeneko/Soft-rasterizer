#include <vector>
#include <iostream>
#include <algorithm>
#include "geometry.h"
#include "TGAImage.h"

int width = 200;
int height = 200;

Vec3f barycentric(Vec2i* points, Vec2i P)
{
	/*
	 * [u, v, 1] * [ABx, ACx, PAx]^T = 0
	 * [u, v, 1] * [ABy, ACy, PAy]^T = 0
	 * 用叉乘得到与[ABx, ACx, PAx] [ABy, ACy, PAy]垂直的向量[u, v, 1]
	 */
    Vec3f w = cross(Vec3f(points[1][0]-points[0][0], points[2][0]-points[0][0], points[0][0] - P[0]), Vec3f(points[1][1]-points[0][1], points[2][1]-points[0][1], points[0][1]-P[1]));

	/*
     * w.z = ABx * ACy - ACx * ABy
     * because points and P have integer coordinates so abs(w.z) < 1 means w.z is 0. which indicates the triangle is degenerate.
     * 但是仔细观察可以发现, w.z的分量是固定的, 因为AB AC两个向量已知，所以w.z < 1的判断可以优化，放到遍历boundingBox的点调用barycentric的循环外部
     * 因为整数对乘法成一个环, 这里abs(w.z) = 0 or 1是离散的, 但因为w.z是Vec3f,由于浮点数存储的精度问题, 比如1存成0.9999, 所以这里用<1判断
     */
    // std::cout << "points: " << w.x << " " << w.y << " " << w.z << std::endl;
    // std::cout << "v1x: " << points[1][0]-points[0][0] << " v2y: " << points[2][1]-points[0][1] << " v1y: " << points[2][0]-points[0][0] << " v2x: " << points[1][1]-points[0][1] << std::endl;
	if (std::abs(w.z < 1)) {
        return Vec3f(-1, -1, -1);
    }
    return Vec3f(1.f - (w.x + w.y) / w.z, w.x / w.z, w.y / w.z);
}

void traingle(Vec2i* points, TGAImage& image, TGAColor color)
{
	// BoundingBox 初始化
    Vec2i bboxmin(image.get_width()-1, image.get_height()-1);
    Vec2i bboxmax(0, 0);
	// 限制BoundingBox的最大值，对屏幕外的像素进行裁剪
	Vec2i clamp(image.get_width()-1, image.get_height()-1);

    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0, std::min(bboxmin[j], points[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], points[i][j]));
        }
    }

    Vec2i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f BC_Screen = barycentric(points, P);
            // std::cout << "BC_X: " << BC_Screen.x << "BC_Y: " << BC_Screen.y << "BC_Z: " << BC_Screen.z << std::endl;
            if (BC_Screen.x<0 || BC_Screen.y<0 || BC_Screen.z<0) continue;
            image.set(P.x, P.y, color);
        }
    }
}

int main(int argc, char** argv)
{
    TGAImage frame(width, height, TGAImage::RGB);
    Vec2i points[3] = { Vec2i(10, 10), Vec2i(100, 30), Vec2i(190, 160) };
    traingle(points, frame, TGAColor(255, 0, 0));
    frame.flip_vertically();
    frame.write_tga_file("framebuffer.tga");
    return 0;
}