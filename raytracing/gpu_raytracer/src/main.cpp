#include "game.h"
#include "vec3.h"
#include <raylib.h>
#include <string>
#include <unordered_map>

#define SCREEN_W 640*2
#define SCREEN_H 360*2
#define MAX_SPHERES 128

struct myCamera {
    Vec3 pos;
    Vec3 u, v, w; // basis vecs :
                  //    u - right
                  //    v - up
                  //    w - forward
    
    Vec3 dir {0,0,-1}; // used for mouse look

    float focalLength, viewportHeight, viewportWidth;
    float fov, aspectRatio;

    Vec3 target, up;

    std::unordered_map<std::string, int> shaderLocs {
        {"cameraPos", 0},{"cameraU", 0},{"cameraV", 0},{"cameraW", 0},
        {"focalLength", 0},{"viewportWidth", 0},{"viewportHeight", 0},
    };

    myCamera(const Vec3& pos, const Vec3& target, const Vec3& up, float fov, float aspectRatio, const Shader& shader)
        : pos(pos), fov(fov), aspectRatio(aspectRatio), target(target), up(up)
    {
        setFovAndBasisVectors();

        // init shader locs from loc map
        for(auto& [name, loc] : shaderLocs){
            loc = GetShaderLocation(shader, name.c_str());
        }
    }

    void setFovAndBasisVectors(){
        focalLength = (pos - target).mag();

        float theta {fov*DEG2RAD};
        double h {tan(theta/2)};

        viewportHeight = 2*h*focalLength;
        viewportWidth = viewportHeight*aspectRatio;

        w = (pos-target).normalized();
        u = cross(up, w).normalized();
        v = cross(w, u);
    }

    void updateTarget(const Vec3& newTarget, const Vec3& newUp){
        target = newTarget;
        up = newUp;
        setFovAndBasisVectors();
    }

    void updateFov(float newFov){
        fov = newFov;
        setFovAndBasisVectors();
    }

    void lookAt(const Vec3& newTarget){
        updateTarget(newTarget, up);
    }

    void updatePos(const Vec3& newPos){
        pos = newPos;
        setFovAndBasisVectors();
    }

    void setUniformValues(const Shader& shader){
        SetShaderValue(shader, shaderLocs["cameraPos"], &pos, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, shaderLocs["cameraU"], &u, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, shaderLocs["cameraV"], &v, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, shaderLocs["cameraW"], &w, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, shaderLocs["focalLength"], &focalLength, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, shaderLocs["viewportWidth"], &viewportWidth, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, shaderLocs["viewportHeight"], &viewportHeight, SHADER_UNIFORM_FLOAT);
    }
};

struct Sphere{
    Vec3 center;
    float r;

    Sphere(const Vec3& center, float r): center(center), r(r){}
};

class Screen : public Entity{
    private:
        Shader shader {LoadShader(0, "../resources/shaders/test.fs")};
        RenderTexture2D screenTexture {LoadRenderTexture(SCREEN_W, SCREEN_H)}; // have to use a render texture for correct fragtexcoords

        myCamera camera;

        Game& game;

        std::vector<Sphere> spheres;

    public:
        Screen(Game& game)
            : game(game), camera(Vec3(), Vec3(0,0,-1), Vec3(0,1,0), 90, 16.0/9.0, shader)
        {
            // set screen res uniforms
            int wLoc {GetShaderLocation(shader, "screenW")};
            int screenW {SCREEN_W};
            SetShaderValue(shader, wLoc, &screenW, SHADER_UNIFORM_INT);

            int hLoc {GetShaderLocation(shader, "screenH")};
            int screenH {SCREEN_H};
            SetShaderValue(shader, hLoc, &screenH, SHADER_UNIFORM_INT);

            // -------------------------------------- //
            /*
            float range = 0.5;
            for(int i = 0; i < 10; ++i){
                spheres.push_back(Sphere({range*GetRandomValue(-10, 10),range*GetRandomValue(-10, 10), -1},GetRandomValue(1, 10)/10.0));
            }
            */
            spheres.push_back(Sphere({1,0,-1},0.5));
            spheres.push_back(Sphere({-1,0,-1},0.5));

            spheres.push_back(Sphere({0,-100.5,-1},100.0));

            // -------------------------------------- //
        }
        ~Screen(){
            UnloadShader(shader);
        }

        void update(float dt) override{
            Vec3 move = Vec3();
            const float cameraMoveSpeed = 0.01;
            const float cameraRotSpeed = 1;

            if(IsKeyDown(KEY_W))
                move -= camera.w;
            if(IsKeyDown(KEY_A))
                move -= camera.u;
            if(IsKeyDown(KEY_S))
                move += camera.w;
            if(IsKeyDown(KEY_D))
                move += camera.u;

            if(IsKeyDown(KEY_LEFT_SHIFT))
                move -= camera.v;
            if(IsKeyDown(KEY_SPACE))
                move += camera.v;

            if(IsKeyDown(KEY_LEFT))
                camera.dir = camera.dir.rotate(camera.up, cameraRotSpeed*DEG2RAD);

            if(IsKeyDown(KEY_RIGHT))
                camera.dir = camera.dir.rotate(camera.up, -cameraRotSpeed*DEG2RAD);

            if(IsKeyDown(KEY_UP))
                camera.dir = camera.dir.rotate(camera.u, cameraRotSpeed*DEG2RAD);

            if(IsKeyDown(KEY_DOWN))
                camera.dir = camera.dir.rotate(camera.u, -cameraRotSpeed*DEG2RAD);

            camera.updatePos(camera.pos + cameraMoveSpeed*move);
            camera.lookAt(camera.pos + camera.dir);
        }

        void draw() override{
            camera.setUniformValues(shader);

            Vec3 centers[MAX_SPHERES];
            float radii[MAX_SPHERES];

            int centersLoc {GetShaderLocation(shader, "sphereCenters")};
            int radiiLoc {GetShaderLocation(shader, "sphereRadii")};
            int countLoc {GetShaderLocation(shader, "sphereCount")};
            int count {static_cast<int>(spheres.size())};
            for(int i = 0; i < count; ++i){
                centers[i] = spheres.at(i).center;
                radii[i] = spheres.at(i).r;
            }
            SetShaderValueV(shader, centersLoc, centers, SHADER_UNIFORM_VEC3, count);
            SetShaderValueV(shader, radiiLoc, radii, SHADER_UNIFORM_FLOAT, count);
            SetShaderValue(shader, countLoc, &count, SHADER_UNIFORM_INT);

            BeginShaderMode(shader);
            Rectangle src {0,0,SCREEN_W,-SCREEN_H};
            Rectangle dest {0,0,SCREEN_W,SCREEN_H};
            DrawTexturePro(screenTexture.texture, src, dest, {0,0}, 0.0f, WHITE);
            EndShaderMode();
            DrawFPS(10, 10);
        }
};

int main(){
    Game game {SCREEN_W, SCREEN_H, "uwu"}; // w, h, title

    game.addEntity(std::make_unique<Screen>(game));

    game.main();
    return 0;
}
