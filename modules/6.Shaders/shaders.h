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

struct PhongShader : public IShader {
	mat<2,3,float> varying_uv;
	mat<4,4,float> uniform_M;
	mat<4,4,float> uniform_MIT;

	virtual Vec4f vertex(int nthface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(nthface, nthvert));
		Vec4f GL_Vertext = embed<4>(model->vert(nthface, nthvert));
		return Viewport * Projection * ModelView * GL_Vertext;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		Vec2f uv = varying_uv * bar;
		Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
		Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();
		Vec3f r = (n*(n*l*2.f) - l).normalize(); //reflected light
		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
		float diff = std::max(0.f, n*l);
		TGAColor c = model->diffuse(uv);
		color = c;
		for (int i = 0; i < 3; i++)
			color[i] = std::min<float>(5 + c[i]*(diff + .6*spec), 255);
		return false;
	}
};

// If we want use TangentSpaceNormalMap, make sure we are using tangent space normal texture !
// load_texture(filename, "_nm_tangent.tga", normalmap_) in model.cpp
struct TangentSpaceNormalMap : public IShader {
    mat<2,3,float> varying_uv;  // vertex uv coordinates
    mat<4,3,float> varying_tri; // vertex in clip coordinates
    mat<3,3,float> varying_nrm; // normal per vertex in clip coordinates to be interpolated by fragment shader
    mat<3,3,float> ndc_tri;     // vertex in normalized device coordinates

    mat<4,4,float> uniform_M;   // Projection*ModelView
    mat<4,4,float> uniform_MIT; // Inverse transform of Projection*ModelView

    virtual Vec4f vertex(int iface, int nvert) {
        varying_uv.set_col(nvert, model->uv(iface, nvert));
        varying_nrm.set_col(nvert, proj<3>(uniform_MIT * embed<4>(model->normal(iface, nvert), 0.f))); // normal is a vector, embed 0.f at end
        Vec4f GL_Vertex = uniform_M * embed<4>(model->vert(iface, nvert)); // vertex is a point, embed 1.0f by default.
        varying_tri.set_col(nvert, GL_Vertex);
        ndc_tri.set_col(nvert, proj<3>(GL_Vertex / GL_Vertex[3]));
        return Viewport * GL_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec3f bn = (varying_nrm * bar).normalize(); // interpolated normal
        Vec2f uv = varying_uv * bar;                // interpolated uv

        // 1. Why should we use P0, P1, P2 in NDC Space to calculate ?
        // 2. Why one basis of Darboux is the interpolated normal (bn) ?
        mat<3,3,float> A;
        A[0] = ndc_tri.col(1) - ndc_tri.col(0);
        A[1] = ndc_tri.col(2) - ndc_tri.col(0);
        A[2] = bn;

        mat<3,3,float> AI = A.invert();

        Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
        Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

        // The i, j, bn is 3 basis of tangent(Darboux) coordinate.
        // Use these basis to construct transformation matrix.
        mat<3,3,float> B;
        B.set_col(0, i.normalize());
        B.set_col(1, j.normalize());
        B.set_col(2, bn);

        Vec3f n = (B * model->normal(uv)).normalize();
        Vec3f l = proj<3>(uniform_M * embed<4>(light_dir, 0.f)).normalize(); // Don't forget to transform light direction !

        float diff = std::max(0.f, n * l);
        color = model->diffuse(uv) * diff;
        return false;
    }
};
  