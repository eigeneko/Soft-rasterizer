#include <iostream>
#include <vector>

#include "geometry.h"
#include "model.h"
#include "our_gl.h"
#include "shaders.h"
#include "tgaimage.h"

int main(int argc, char **argv) {
  if (argc == 2)
    model = new Model(argv[1]);
  else
    model = new Model("obj/african_head.obj");

  float *zbuffer = new float[width * height];
  shadowbuffer = new float[width * height];
  for (int i = width * height; --i;) {
    zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
  }

  light_dir.normalize();

  // Rendering the shadow buffer
  {
    TGAImage depth(width, height, TGAImage::RGB);
    lookat(light_dir, center, up);
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(0);

    DepthShader shader;
    Vec4f screen_coords[3];
    for (int i = 0; i < model->nfaces(); i++) {
      for (int j = 0; j < 3; j++) {
        screen_coords[j] = shader.vertex(i, j);
      }
      triangle(screen_coords, shader, depth, zbuffer);
    }

    depth.flip_vertically();
    depth.write_tga_file("depth.tga");
  }

  delete model;
  return 0;
}