#include <cmath>
#include <random>
#include <algorithm>

#include "model.h"
#include "our_gl.h"

extern mat<4,4> Viewport, ModelView, Perspective;
extern std::vector<double> zbuffer; // depth buffer

struct BlankShader : IShader {
    const Model &model;

    BlankShader(const Model &m) : model(m) {}

    virtual vec4 vertex(const int face, const int vert) {
        vec4 gl_Position = ModelView * model.vert(face, vert);
        return Perspective * gl_Position;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        return {false, {255, 255, 255, 255}};
    }
};

int main (int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3    eye{-1, 0, 2}; // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    lookat(eye, center, up);
    init_perspective(norm(eye-center));
    init_viewport(width/16, height/16, width*7/8, height*7/8);
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});

    for (int m=1; m<argc; m++) {
        Model model(argv[m]);
        BlankShader shader{model};
        for (int f=0; f<model.nfaces(); f++) {
            Triangle clip = { shader.vertex(f, 0),
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, shader, framebuffer);
        }
    }

    constexpr double ao_radius = .1; // ssao ball radius in NDC space
    constexpr int nsamples = 128;    // number of samples in the ball
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-ao_radius, ao_radius);
    auto smoothstep = [](double edge0, double edge1, double x) {
        double t = std::clamp((x - edge0) / (edge1 - edge0), 0., 1.);
        return t * t * (3 - 2 * t);
    };

#pragma omp parallel for
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double z = zbuffer[x + y * width];
            if (z < -100) continue;
            vec4 fragment = Viewport.invert() * vec4{x, y, z, 1.};
            double vote = 0;
            double voters = 0;
            for (int i = 0; i < nsamples; i++) {
                vec4 p = Viewport * (fragment + vec4{dist(gen), dist(gen), dist(gen), 0.});
                if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height) continue;
                double d = zbuffer[int(p.x) + int(p.y) * width];
                if (z + 5 * ao_radius < d) continue; // discard if the gap between sample depth and z is too large to remove dark halo
                voters++;
                vote += d > p.z; // if sample depth is greater than fragment depth, vote for shadowing.
            }
            double ssao = smoothstep(0, 1, 1 - vote / voters * .4);
            TGAColor c = framebuffer.get(x, y);
            framebuffer.set(x, y, {c[0] * ssao, c[1] * ssao, c[2] * ssao, c[3]});
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
