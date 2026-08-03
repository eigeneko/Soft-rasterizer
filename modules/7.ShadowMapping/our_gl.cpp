#include <cmath>
#include <limits>
#include <cstdlib>
#include "our_gl.h"

Model* model     = NULL;
float* shadowbuffer = NULL;
const int width  = 800;
const int height = 800;

Vec3f light_dir(1,1,1);
Vec3f eye(0,-1,3);
Vec3f center(0,0,0);
Vec3f up(0,1,0);

Matrix ModelView;
Matrix Projection;
Matrix Viewport;

IShader::~IShader() {}

void viewport(int x, int y, int w, int h)
{
    Viewport = Matrix::identity();
    Viewport[0][3] = x+w/2.f;
    Viewport[1][3] = y+h/2.f;
    Viewport[2][3] = 255.f/2.f;
    Viewport[0][0] = w/2.f;
    Viewport[1][1] = h/2.f;
    Viewport[2][2] = 255.f/2.f;
}

void projection(float coeff)
{
    Projection = Matrix::identity();
    Projection[3][2] = coeff;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross(z, up).normalize();
    Vec3f y = cross(z, x).normalize();
    ModelView = Matrix::identity();
    for (int i = 0; i < 3; i++) {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -center[i];
    }
}

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P)
{
    Vec3f vec1(C[0]-A[0], B[0]-A[0], A[0]-P[0]);
    Vec3f vec2(C[1]-A[1], B[1]-A[1], A[1]-P[1]);
    Vec3f w = cross(vec1, vec2);
    if (std::abs(w.z)>1e-2)
        return Vec3f(1.f-(w.x+w.y)/w.z, w.y/w.z, w.x/w.z);
    return Vec3f(-1, -1, -1);
}

void triangle(Vec4f* points, IShader& shader, TGAImage& image, float* zbuffer)
{
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::min(bboxmin[j], points[i][j]/points[i][3]);
            bboxmax[j] = std::max(bboxmax[j], points[i][j]/points[i][3]);
        }
    }
    Vec2i P;
    TGAColor color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
        Vec3f c = barycentric(proj<2>(points[0]/points[0][3]), proj<2>(points[1]/points[1][3]), proj<2>(points[2]/points[2][3]), proj<2>(P));
        float z = points[0][2]*c.x + points[1][2]*c.y + points[2][2]*c.z;
        float w = points[0][3]*c.x + points[1][3]*c.y + points[1][3]*c.z;
        int frag_depth = std::max(0, std::min(255, int(z/w+.5)));
        if (c.x<0 || c.y<0 || c.z<0 || zbuffer[P.x + P.y * image.get_width()]>frag_depth) continue;
        bool discard = shader.fragment(c, color);
        if (!discard) {
            zbuffer[P.x + P.y * image.get_width()] = frag_depth;
            image.set(P.x, P.y, color);
        }
        }
    }
}