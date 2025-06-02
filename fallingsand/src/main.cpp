#include "game.h"
#include <memory>
#include <raylib.h>

class Screen : public Entity{
    public:
        PixelTexture screen_texture;
        Game& game_ref;

        Color sand {200,200,0,255};
        Color water {100,100,255,255};
        Color empty {};

        Screen(Game& game) : game_ref(game), screen_texture(game.windowW, game.windowH) {}

        void waterRules(int i, int j){
            Color below {screen_texture.getPixelColor(i, j+1)};
            if(ColorIsEqual(below, empty)){
                screen_texture.setPixel(i, j, empty);
                screen_texture.setPixel(i, j+1, water);
            }else if(i+1 < game_ref.windowW && i > 0 && j+1 < game_ref.windowH && j > 0){
                Color belowleft {screen_texture.getPixelColor(i-1, j+1)};
                Color belowright {screen_texture.getPixelColor(i+1, j+1)};
                Color left {screen_texture.getPixelColor(i-1, j)};
                Color right {screen_texture.getPixelColor(i+1, j)};

                int dir {GetRandomValue(0, 1)};
                if(dir && ColorIsEqual(belowleft, empty)){
                    screen_texture.setPixel(i, j, empty);
                    screen_texture.setPixel(i-1, j+1, water);
                }else if(ColorIsEqual(belowright, empty)){
                    screen_texture.setPixel(i, j, empty);
                    screen_texture.setPixel(i+1, j+1, water);
                }else if(dir && ColorIsEqual(left, empty)){
                    screen_texture.setPixel(i, j, empty);
                    screen_texture.setPixel(i-1, j, water);
                }else if(ColorIsEqual(right, empty)){
                    screen_texture.setPixel(i, j, empty);
                    screen_texture.setPixel(i+1, j, water);
                }
            }
        }

        void sandRules(int i, int j){
            Color below {screen_texture.getPixelColor(i, j+1)};
            if(ColorIsEqual(below, empty)){
                screen_texture.setPixel(i, j, empty);
                screen_texture.setPixel(i, j+1, sand);
            }else if(i+1 < game_ref.windowW && i > 0 && j+1 < game_ref.windowH && j > 0){
                Color left {screen_texture.getPixelColor(i-1, j+1)};
                Color right {screen_texture.getPixelColor(i+1, j+1)};

                int dir {GetRandomValue(0, 1)};
                if(dir && ColorIsEqual(left, empty)){
                    screen_texture.setPixel(i, j, empty);
                    screen_texture.setPixel(i-1, j+1, sand);
                }else if(ColorIsEqual(right, empty)){
                    screen_texture.setPixel(i, j, empty);
                    screen_texture.setPixel(i+1, j+1, sand);
                }
            }
        }

        void update(float dt) override{

            for(int i = 0; i < game_ref.windowW; ++i){
                for(int j = 0; j < game_ref.windowH; ++j){
                    Color curr {screen_texture.getPixelColor(i, j)};

                    if(ColorIsEqual(curr, sand)){
                        sandRules(i, j);
                    }
                    if(ColorIsEqual(curr, water)){
                        waterRules(i, j);
                    }
                }
            }

            if(game_ref.mouse.down.left || game_ref.mouse.down.right){
                int i {static_cast<int>(game_ref.mouse.pos.x)};
                int j {static_cast<int>(game_ref.mouse.pos.y)};

                int brush_w {20};
                int brush_h {20};
                Color col {game_ref.mouse.down.left ? sand : water};
                for(int k = 0; k < brush_w/2; ++k){
                    for(int l = 0; l < brush_h/2; ++l){
                        screen_texture.setPixel(i+k, j+l, col);
                        screen_texture.setPixel(i-k, j-l, col);
                        screen_texture.setPixel(i+k, j-l, col);
                        screen_texture.setPixel(i-k, j+l, col);
                    }
                }
            }

            screen_texture.setTexture();
        }

        void draw() override{
            screen_texture.draw(0,0,game_ref.windowW,game_ref.windowH);
            DrawFPS(10,10);
        }
};

int main(){
    Game game {400, 300, "uwu"}; // w, h, title

    game.addEntity(std::make_unique<Screen>(game));

    game.main();
    return 0;
}
