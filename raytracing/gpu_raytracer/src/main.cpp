#include "game.h"
#include "vec3.h"
#include <raylib.h>
#include <string>
#include <unordered_map>

#define SCREEN_W 640
#define SCREEN_H 360

struct myCamera {
    Vec3 pos;
    Vec3 u, v, w; // basis vecs :
                  //    u - right
                  //    v - up
                  //    w - forward

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

class Screen : public Entity{
    private:
        Shader shader {LoadShader(0, "../resources/shaders/test.fs")};
        RenderTexture2D screenTexture {LoadRenderTexture(SCREEN_W, SCREEN_H)}; // have to use a render texture for correct fragtexcoords

        myCamera camera;

    public:
        Screen()
            : camera(Vec3(), Vec3(0,0,-1), Vec3(0,1,0), 90, 16.0/9.0, shader)
        {
            // set screen res uniforms
            int wLoc {GetShaderLocation(shader, "screenW")};
            int screenW {SCREEN_W};
            SetShaderValue(shader, wLoc, &screenW, SHADER_UNIFORM_INT);

            int hLoc {GetShaderLocation(shader, "screenH")};
            int screenH {SCREEN_H};
            SetShaderValue(shader, hLoc, &screenH, SHADER_UNIFORM_INT);
        }
        ~Screen(){
            UnloadShader(shader);
        }

        void update(float dt) override{
        }
        void draw() override{
            camera.setUniformValues(shader);
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

    game.addEntity(std::make_unique<Screen>());

    game.main();
    return 0;
}
