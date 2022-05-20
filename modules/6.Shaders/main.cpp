#include <iostream>
#include <vector>

#include "geometry.h"
#include "model.h"
#include "shaders.h"
#include "tgaimage.h"
#include "our_gl.h"

int main(int argc, char **argv) {
    if (argc == 2)
        model = new Model(argv[1]);
    else
        model = new Model("obj/african_head.obj");

    lookat(eye, center, up);
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(-1.f / (eye - center).norm());
    light_dir.normalize();

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    TangentSpaceNormalMap shader;
    shader.uniform_M = Projection * ModelView;
    shader.uniform_MIT = (Projection * ModelView).invert_transpose();

    for (int i = 0; i < model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.write_tga_file("image.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}