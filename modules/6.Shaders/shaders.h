#include "geometry.h"
#include "our_gl.h"

struct GouraudShader : public IShader {
  Vec3f vertex_lightIntensity;

  // virtual ~GouraudShader() {}

  virtual Vec4f vertex(int nthface, int nthvertex) {
    Vec4f GL_Vertex = embed<4>(model->vert(
        nthface, nthvertex)); // embed vec3f to vec4f by adding a component
    Vec4f coordinates = Viewport * Projection * ModelView * GL_Vertex;
    vertex_lightIntensity[nthvertex] =
        std::max(0.f, model->normal(nthface, nthvertex) * light_dir);
    return coordinates;
  }

  virtual bool fragment(Vec3f bar_coordinates, TGAColor &color) {
    float intensity = vertex_lightIntensity *bar_coordinates; // interpolate the intensity for current pixel within the traingle.
    color = TGAColor(255, 255, 255) * intensity;
    return false;
  }
};

struct ToonShader : public IShader {
  Vec3f vertex_lightIntensity;

  // virtual ~ToonShader() {}

  virtual Vec4f vertex(int nthface, int nthvertex) {
    Vec4f GL_Vertex = embed<4>(model->vert(
        nthface, nthvertex)); // embed vec3f to vec4f by adding a component
    Vec4f coordinates = Viewport * Projection * ModelView * GL_Vertex;
    vertex_lightIntensity[nthvertex] =
        std::max(0.f, model->normal(nthface, nthvertex) * light_dir);
    return coordinates;
  }

  virtual bool fragment(Vec3f bar_coordinates, TGAColor &color) {
    float intensity = vertex_lightIntensity * bar_coordinates;
    if (intensity > .85)
      intensity = 1;
    else if (intensity > .60)
      intensity = .80;
    else if (intensity > .45)
      intensity = .60;
    else if (intensity > .30)
      intensity = .45;
    else if (intensity > .15)
      intensity = .30;
    color = TGAColor(255, 155, 0) * intensity;
    return false;
  }
};

// 使用Texture
struct DiffuseShader : public IShader {
  Vec3f vertex_lightIntensity;
  mat<2, 3, float> varying_uv;

  // virtual ~DiffuseShader() {}

  virtual Vec4f vertex(int nthface, int nthvert) {
    varying_uv.set_col(nthvert, model->uv(nthface, nthvert));
    Vec4f GL_Vertex = embed<4>(model->vert(nthface, nthvert));
    Vec4f coordinates = Viewport * Projection * ModelView * GL_Vertex;
    vertex_lightIntensity[nthvert] =
        std::max(0.f, model->normal(nthface, nthvert) * light_dir);
    return coordinates;
  }

  virtual bool fragment(Vec3f bar_coordinates, TGAColor &color) {
    float intensity = vertex_lightIntensity * bar_coordinates; // interpolate the intensity for current pixel
    Vec2f uv = varying_uv * bar_coordinates;                   // interpolate uv for current pixel
    color = model->diffuse(uv) * intensity;
    return false;
  }
};

// 使用Normal Mapping Texture获取信息而不是使用顶点的normal做插值
struct NormalMappingShader : public IShader {
    mat<2,3,float> varying_uv;
    mat<4,4,float> uniform_M;   // Projection*ModelView
    mat<4,4,float> uniform_MIT; // Inverse transform of Projection*ModelView
    
    // virtual ~NormalMapping() {}

    Vec4f virtual vertex(int nthface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(nthface, nthvert));
        Vec4f GL_Vertex = embed<4>(model->vert(nthface, nthvert));
        return Viewport * Projection * ModelView * GL_Vertex;
    }

    bool virtual fragment(Vec3f bar_coordinates, TGAColor& color) {
        Vec2f uv = varying_uv * bar_coordinates;
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
        Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();
        float intensity = std::max(0.f, n * l);
        color = model->diffuse(uv) * intensity;
        return false;
    }
};