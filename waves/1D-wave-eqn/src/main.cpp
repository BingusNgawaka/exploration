#include "game.h"
#include <cmath>
#include <functional>
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

class WaveSim : public Entity{
    private:
        float dt;
        float dx;
        float L;
        float c;
        std::vector<Vec2> points;
        std::vector<Vec2> oldPoints;
        std::function<float(int)> initDefl;
        std::function<float(int)> initVel;

        // this class as it is isnt super modular but the idea would be to change these
        // y_0 and x_0 values to move your wave sim around as everything is calculated at (0,0) so its easy to just shift it around
        float y_0 {300};
        float x_0 {};
        float damping {0};
        float fps {};
        float accumulatedTime {};
        Game* game;

        Vec2 oldMouse {};

    public:
        // game ref, L, dt, dx, c, initPos, initOldPos
        WaveSim(Game* game, float length, float time_resolution, float spatial_resolution, float wave_speed, std::function<float(int)> init_cond_func, std::function<float(int)> init_vel_func):
            dx(spatial_resolution), dt(time_resolution), L(length), c(wave_speed), points(), oldPoints(), game(game), initVel(init_vel_func), initDefl(init_cond_func)
        {
            reset(game, L, dt, dx, c, initDefl, initVel);
        }
        void reset(Game* game, float length, float time_resolution, float spatial_resolution, float wave_speed, std::function<float(int)> init_cond_func, std::function<float(int)> init_vel_func){
            accumulatedTime = 0;
            points.clear();
            oldPoints.clear();
            int num_of_points {static_cast<int>(L/dx)};
            for(int i = 0; i <= num_of_points; ++i){
               // hard coded init condition for now
               float initDefl {init_cond_func(i*dx)};
               points.push_back(Vec2(i*dx, initDefl));
            }

            // setting oldpoints sets our init vel in a way
            for(int i = 0; i <= num_of_points; ++i){
               // hard coded init condition for now
               float initDefl {init_vel_func(i*dx)};
               oldPoints.push_back(Vec2(i*dx, initDefl));
            }

        }
        void draw(){
            Vec2 offSetVec {x_0, y_0};
            for(int i = 0; i < points.size(); ++i){
                // debug draw for real point positions
                //(points.at(i)+Vec2(x_0, y_0/2)).drawPoint();
                if(i == 0){
                    (points[i] + offSetVec).drawLineTo(points[i+1]+offSetVec, BLACK);
                }else if(i > 1){
                    (points[i] + offSetVec).drawLineTo(points[i-1]+offSetVec, BLACK);
                }
            }

            // RAYGUI shit //
            // 
            float xPos {130};
            float yPos {10};
            float w {200};
            float h {30};
            GuiSlider ( Rectangle{ xPos, yPos, w, h },
            "Wave Speed (c)", NULL, &c, 0, 1000 );

            GuiSlider ( Rectangle{ xPos, yPos+h*1.5f, w, h },
            "Time Resolution (dt)", NULL, &dt, 0, 0.3 );

            GuiSlider ( Rectangle{ xPos, yPos+h*3, w, h },
            "Spatial Resolution (dx)", NULL, &dx, 0, 15 );

            if(GuiButton(Rectangle{xPos + w*1.2f, yPos, w/2, h}, "RESET WAVE")){
                reset(game, L, dt, dx, c, initDefl, initVel);
            }

            if(GuiButton(Rectangle{xPos + w*1.2f, yPos+h*1.5f, w/2, h}, "RESET CONSTS")){
                c = 40;
                dt = 0.05;
                dx = 2.5;
            }

            float K = pow(c*(this->dt)/dx, 2); // courant cond
            DrawText(("Courant Constant: " + std::to_string(K)).c_str(), static_cast<int>(xPos + w*1.1), static_cast<int>(yPos+h*3), 32, BLACK);

            DrawText(("FPS: " + std::to_string(fps)).c_str(), static_cast<int>(xPos + w*2), static_cast<int>(yPos), 32, BLACK);
            // ----------- //

            // draw mouse pos
            DrawCircle(game->mouse.pos.x, game->mouse.pos.y, 5, RED);
        }
        bool checkMouseCollision(Vec2 pos){
            return ((game->mouse.pos - (pos+Vec2(x_0, y_0))).length() < 30);
        }
        void updateSim(){
            std::vector<Vec2> newPoints {points};
            for(int i = 0; i < points.size(); ++i){
                if(i == 0 || i == points.size() - 1){
                    // boundary condition
                    newPoints[i].y = 0;
                }else{
                    if(game->mouse.down.left) {
                        // apply a gaussian dist of vel impulse to all points to make interaction smooth
                        // this is just a simple example for now
                        float impactX = game->mouse.pos.x - x_0;
                        float impulse = (game->mouse.pos.y - oldMouse.y);
                        float A = impulse * 0.001f;
                        float s = 30.0f;

                        for (int i = 0; i < points.size(); ++i) {
                            float dx = points[i].x - impactX;
                            float gaussian = A * std::exp(-(dx * dx) / (2 * s * s));

                            points[i].y    -= gaussian;
                            oldPoints[i].y -= gaussian;
                        }
                    }

                    float K = pow(c*(this->dt)/dx, 2); // courant cond
                    float vel {points[i].y - oldPoints[i].y};
                    vel *= (1 - damping); // dont rlly need daming cause of restoring force

                    // u(x, t + dt) wave eqn
                    newPoints[i].y = points[i].y + vel + K*(points[i+1].y - 2*points[i].y + points[i-1].y);

                    float restoringForce = -newPoints[i].y * 0.01;
                    newPoints[i].y += restoringForce;
                }
            }
            
            // slide window fwd
            oldPoints = points;
            points = newPoints;
            oldMouse = game->mouse.pos;
        }

        void update(float dt){
            fps = 1/dt;
            accumulatedTime += dt;
            std::cout << "deltaTime: " << dt << "\n";
            std::cout << "time res: " << this->dt << "\n";
            int i {};
            while(accumulatedTime >= this->dt){ // force fixed time step
                updateSim();
                accumulatedTime -= this->dt;
                i++;
            }
            std::cout << "iters this frame: " << i << "\n";
        }
};

float initPos(int x){
    //return 100*sin(PI*5*x/800);

    /* Gaussian */
    float A {-200};
    float x_0 {400};
    float num {static_cast<float>(pow(x-x_0, 2))};
    float s {60};
    float denom {2*s*s};
    return A*exp(-(num)/(denom));
}

int main(){
    Game game {800, 600, "1D Wave Equation"};

    game.add_entity(new WaveSim(&game, 800, 0.016, 5, 300, initPos, initPos));

    game.main();
    return 0;
}
