#include "LCD_1in44.h"
#include <random>
#include <typeinfo>
#include <algorithm>
#include <vector>
#include "GUI_Paint.h"
#include "DEV_Config.h"
#include "Debug.h"
#include <cmath>
#include <stdlib.h>
#include <string>
#include "Infrared.h"
#include <cstring>
#include "hardware/dma.h"

#define DEBUG 1
#define key0 15
#define key1 17
#define key2 2
#define key3 3

int get_random_int(int a, int b){
        static std::random_device rd;
        static std::mt19937 gen(rd());

        std::uniform_int_distribution<> distrib(a, b);

        return distrib(gen);
}

// api reference gathered from digging about
// DEV_Module_Init() not 100% sure what this does lmao
//      return code != 0 is bad
// DEV_Module_Exit()
//
// DEV_Delay_ms()
//      sleepy times
// LCD_1IN44_Init(HORIZONTAL/VERTICAL)
//      init screen in horiz or vert display ( i think this just changes the coord system ) 
// LCD_1IN44_Clear(COLOR)
//      clear screen with provided color
// Paint_DrawString_EN(x, y, string, &Font[1-20], FONTCOLOR, BGCOLOR);
//
// Colors are in RGB565 ie 2bytes split between 5 bits R 6 bits G and 5 bits B
// eg: 0xFFFF is white
//
// Input stuff
/*
    int key0 = 15; 
    int key1 = 17; 
    int key2 = 2; 
    int key3 = 3; 
    
    SET_Infrared_PIN(key0);    
    SET_Infrared_PIN(key1);
    SET_Infrared_PIN(key2);
    SET_Infrared_PIN(key3);

    DEV_Digital_Read(key#)
        returns true or false if currently held... i think

*/
// Drawing shit
// Paint_DrawRectangle(88, 98, 123, 128, YELLOW, DOT_PIXEL_1X1,DRAW_FILL_FULL)
//      (topleftx, toplefty, bottomrightx, bottomrighty, COL, PIXEL_SIZE???, FILLSTYLE?);
// Paint_DrawCircle(95, 25, 15, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
//      (x, y, r, COL, PIXEL_SIZE?, FILLSTYLE)
// Paint_DrawPoint(122,5, BLACK, DOT_PIXEL_5X5, DOT_FILL_RIGHTUP);
//      (x, y, COL, PIXEL_SIZE?, FILLSTYLE)
// Paint_DrawLine( 10,  10, 40, 40, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
//      (x1, y1, x2, y2, COL, PIXEL_SIZE, LINESTYLE)

// I think this sets up the image.. cache..? im not sure but looking thru everything has 
//      LCD_1IN44_Display(BlackImage);
// after all draw calls so maybe everything is drawn onto BlackImage for some reason then
// doing this displays it
/*
    UDOUBLE Imagesize = LCD_1IN44_HEIGHT*LCD_1IN44_WIDTH*2;
    UWORD *BlackImage;
    if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        exit(0);
    }
    // 1.Create a new image cache named IMAGE_RGB and fill it with white
    Paint_NewImage((UBYTE *)BlackImage,LCD_1IN44.WIDTH,LCD_1IN44.HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_Clear(WHITE);
    Paint_SetRotate(ROTATE_0);
    Paint_Clear(WHITE);

    dont forget to free(BlackImage)
*/
struct Vec2{
        float x;
        float y;

        Vec2(float x, float y): x(x), y(y){}
        Vec2(): x(0), y(0){}

        Vec2 operator*(float t){
                return Vec2(x*t, y*t);
        }
        Vec2 operator+(const Vec2& other){
                return Vec2(x+other.x, y+other.y);
        }
        bool operator==(const Vec2& other){
                return other.x == x && other.y == y;
        }
        Vec2 operator-(const Vec2& other){
                return Vec2(x-other.x, y-other.y);
        }
};

bool isKeyDown(int keycode){
        return DEV_Digital_Read(keycode) == 0;
}

void drawRect(Vec2 pos, Vec2 size, UWORD col){
        Paint_DrawRectangle(std::floor(pos.x), std::floor(pos.y), std::floor(pos.x)+size.x, std::floor(pos.y)+size.y, col, DOT_PIXEL_1X1, DRAW_FILL_FULL);
}

enum class EntityType{
        PLAYER,
        WALL
};

struct Game;

struct Entity{
        std::vector<EntityType> tags {};
        virtual void update(float dt, Game* gamePtr) = 0; // ik this is terrible but whatever
        virtual void draw() = 0;
        virtual ~Entity() = default;
        void addTag(EntityType tag){
                tags.push_back(tag);
        }
        bool hasTag(EntityType tag){
                return std::find(tags.begin(), tags.end(), tag) != tags.end();
        }
};

struct Game{
        std::vector<Entity*> entities {};
        std::vector<Entity*> entitiesToRm {};
        std::vector<Entity*> entitiesToAdd {};

        void addEntity(Entity* e){
                entitiesToAdd.push_back(e);
        }
        void rmEntity(Entity* e){
                entitiesToRm.push_back(e);
        }
        void addEntities(){
                for(const auto& e : entitiesToAdd){
                        entities.push_back(e);
                }
                entitiesToAdd.clear();
        }
        void rmEntities(){
                for(const auto& e : entitiesToRm){
                        entities.erase(std::find(entities.begin(), entities.end(), e));
                }
                entitiesToRm.clear();
        }

        void update(float dt){
                for(auto& e: entities) e->update(dt, this);
        }

        void draw(){
                for(auto& e: entities) e->draw();
        }
};

// returns min trans vec of collision, 0,0 if none
Vec2 AABBCollision(Vec2& pos1, Vec2& size1, Vec2& pos2, Vec2& size2) {
    if(pos1.x + size1.x <= pos2.x || pos1.x >= pos2.x + size2.x || 
       pos1.y + size1.y <= pos2.y || pos1.y >= pos2.y + size2.y) {
        return Vec2(0, 0);
    }

    float overlapX1 = (pos1.x + size1.x) - pos2.x;
    float overlapX2 = (pos2.x + size2.x) - pos1.x;
    float overlapY1 = (pos1.y + size1.y) - pos2.y;
    float overlapY2 = (pos2.y + size2.y) - pos1.y;

    float minX = (overlapX1 < overlapX2) ? -overlapX1 : overlapX2;
    float minY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;

    if (std::abs(minX) < std::abs(minY)) {
        return Vec2(minX, 0);
    } else {
        return Vec2(0, minY);
    }
}

namespace Snake{
        class Player : public Entity{
                private:
                        std::vector<Vec2> body {};
                        Vec2 dir {1, 0};
                        float size {8};
                        float movementTimer {};
                        float movementTimerMax {0.3};

                        Vec2 apple {};
                        bool ateApple {false};

                        bool key0Down {};
                        bool key3Down {};

                public:
                        Player(){
                                for(int i{}; i < 1; ++i){
                                        body.push_back({10-i,5});
                                }

                                eat_apple();
                        }

                        void check_collision(Game* gamePtr){
                                for(int i{}; i < body.size()-1; ++i){
                                        if(body.at(i) == body.back()){
                                                gamePtr->rmEntity(this);
                                        }
                                }
                        }

                        void eat_apple(){
                                if(movementTimerMax > 0.1)
                                        movementTimerMax *= 0.9;
                                apple.x = get_random_int(0,LCD_1IN44.WIDTH/size - 1);
                                apple.y = get_random_int(0,LCD_1IN44.HEIGHT/size - 1);
                                ateApple = true;
                        }

                        void move(){
                                body.push_back(body.back()+dir);
                                if(body.back() == apple){
                                        eat_apple();
                                }
                                if(ateApple)
                                        ateApple = false;
                                else
                                        body.erase(body.begin());
                        }

                        void wrap(){
                                if(body.back().x < 0)
                                        body.back().x = LCD_1IN44.WIDTH/size - 1;
                                if(body.back().x >= LCD_1IN44.WIDTH/size)
                                        body.back().x = 0;

                                if(body.back().y >= LCD_1IN44.HEIGHT/size)
                                        body.back().y = 0;
                                if(body.back().y < 0)
                                        body.back().y = LCD_1IN44.HEIGHT/size - 1;
                        }

                        void input(){
                                if(isKeyDown(key0) && !key0Down){
                                        Vec2 newDir {dir.y, -dir.x};
                                        if(std::find(body.begin(), body.end(), body.back()+newDir) == body.end()){
                                                dir = newDir;
                                        }
                                }
                                if(isKeyDown(key3) && !key3Down){
                                        Vec2 newDir {-dir.y, dir.x};
                                        if(std::find(body.begin(), body.end(), body.back()+newDir) == body.end()){
                                                dir = newDir;
                                        }
                                }

                                key0Down = isKeyDown(key0);
                                key3Down = isKeyDown(key3);
                        }

                        void update(float dt, Game* gamePtr) override{
                                input();
                                if(movementTimer <= 0){
                                        move();
                                        movementTimer = movementTimerMax;
                                }else{
                                        movementTimer -= dt;
                                }
                                wrap();

                                if(body.back() == apple){
                                        eat_apple();
                                }
                                check_collision(gamePtr);
                        }
                        void draw() override{
                                for(int i{}; i < body.size(); ++i){
                                        drawRect(body.at(i)*size, {size, size}, GREEN);
                                }
                                drawRect(apple*size, {size,size}, RED);
                        }
        };
}

namespace Platformer{
        class Wall : public Entity{
                public:
                        Vec2 pos;
                        Vec2 size;
                        Wall(Vec2 pos, Vec2 size): pos(pos), size(size) {
                                addTag(EntityType::WALL);
                        }
                        void update(float dt, Game* gamePtr) override{
                        }
                        void draw() override{
                                drawRect(pos, size, BLACK);
                        }

        };

        class Player : public Entity{
                private:
                        Vec2 spawnPoint;
                        Vec2 pos;
                        Vec2 size {8, 8};
                        Vec2 vel {};

                        float speed {500};
                        float jump {20000};
                        float gravity {1200};

                        bool grounded {true};

                        float drag {0.94};

                        UWORD col {BLUE};

                public:
                        Player(Vec2 startPos): pos(startPos), spawnPoint(startPos){}
                        void die(){
                                pos = spawnPoint;
                                vel = Vec2();
                        }
                        void update(float dt, Game* gamePtr) override{
                                if(isKeyDown(key0))
                                        vel.x -= speed*dt;
                                if(isKeyDown(key1))
                                        vel.x += speed*dt;

                                if(isKeyDown(key3) && grounded){
                                        vel.y = -jump*dt;
                                }

                                pos = pos + vel*dt;

                                vel.y += gravity*dt;

                                vel.x = vel.x * drag;
                                vel.y = vel.y * (0.98);

                                // wall collisions
                                grounded = false;
                                for(const auto& e : gamePtr->entities){
                                        if(e->hasTag(EntityType::WALL)){
                                                Wall* wall {static_cast<Wall*>(e)};
                                                Vec2 collis {AABBCollision(pos, size, wall->pos, wall->size)};
                                                if(collis.x != 0 || collis.y != 0){
                                                        pos = pos + collis;
                                                        Paint_DrawString_EN(32, 32, ("collis: "+std::to_string(std::floor(collis.x))+", "+std::to_string(std::floor(collis.y))).c_str(), &Font8, BLACK, WHITE);
                                                        if(collis.x != 0){
                                                                vel.x = 0;
                                                        }
                                                        if(collis.y < 0)
                                                                grounded = true;
                                                        if(collis.y != 0){
                                                                vel.y = 0;
                                                        }
                                                }
                                        }
                                }
                                //
                                if(pos.y+size.y > LCD_1IN44.HEIGHT){
                                        die();
                                }
                                if(pos.x+size.x > LCD_1IN44.WIDTH){
                                        pos.x = LCD_1IN44.WIDTH-size.x;
                                        vel.x = 0;
                                }
                                if(pos.y < 0){
                                        pos.y = 0;
                                        vel.y = 0;
                                }
                                if(pos.x < 0){
                                        pos.x = 0;
                                        vel.x = 0;
                                }
                        }
                        void draw() override{
                                drawRect(pos, size, BLUE);
                                //if(DEBUG) Paint_DrawString_EN(70, 32, ("vel: "+std::to_string(std::floor(vel.x))+", "+std::to_string(std::floor(vel.y))).c_str(), &Font8, BLACK, WHITE);
                                //if(DEBUG) Paint_DrawString_EN(70, 64, ("pos: "+std::to_string(std::floor(pos.x))+", "+std::to_string(std::floor(pos.y))).c_str(), &Font8, BLACK, WHITE);
                        }
        };
}

namespace Home{
        struct Button{
                Vec2 pos;
                Vec2 size;

                Button(Vec2 p, Vec2 s): pos(p), size(s){}
        };

        class Menu : public Entity{
                private:
                        std::vector<Button> btns;
                        int selectedBtn {0};

                        bool key0Down {false};
                        bool key1Down {false};
                        bool key2Down {false};
                        bool key3Down {false};

                public:
                        Menu(){
                                float w {16};
                                float h {8};
                                Vec2 s {w, h};
                                int yPad {16};
                                for(int j{}; j < 2; ++j){
                                        for(int i{}; i < 5; ++i){
                                                Vec2 p {(i+1)*((LCD_1IN44.WIDTH - (5*w))/6)+i*w, (LCD_1IN44.HEIGHT*0.65 + (h+yPad)*j)};
                                                btns.push_back(Button(p, s));
                                        }
                                }
                        }
                        void update(float dt, Game* gamePtr) override{
                                if(isKeyDown(key0) && !key0Down){
                                        --selectedBtn;
                                        if(selectedBtn == 4){
                                                selectedBtn = 9;
                                        }
                                        if(selectedBtn == -1){
                                                selectedBtn = 4;
                                        }
                                }
                                if(isKeyDown(key1) && !key1Down){
                                        ++selectedBtn;
                                        if(selectedBtn == 5){
                                                selectedBtn = 0;
                                        }
                                        if(selectedBtn == 10){
                                                selectedBtn = 5;
                                        }
                                }
                                if(isKeyDown(key2) && !key2Down){
                                        selectedBtn += 5;
                                }
                                key0Down = isKeyDown(key0);
                                key1Down = isKeyDown(key1);
                                key2Down = isKeyDown(key2);
                                key3Down = isKeyDown(key3);

                                selectedBtn = selectedBtn % 10;

                                if(selectedBtn == 0 && isKeyDown(key3)){
                                        gamePtr->rmEntity(this);
                                        gamePtr->addEntity(new Platformer::Player({10,10}));
                                        gamePtr->addEntity(new Platformer::Wall({0,100},{LCD_1IN44.WIDTH, 8}));
                                        gamePtr->addEntity(new Platformer::Wall({64,80},{16, 8}));
                                        gamePtr->addEntity(new Platformer::Wall({80,62},{16, 8}));
                                        gamePtr->addEntity(new Platformer::Wall({42,48},{16, 8}));
                                }
                                if(selectedBtn == 1 && isKeyDown(key3)){
                                        gamePtr->rmEntity(this);
                                        gamePtr->addEntity(new Snake::Player());
                                }
                        }
                        void draw() override{
                                //drawRect(Vec2(0, LCD_1IN44.HEIGHT*0.55), Vec2(LCD_1IN44.WIDTH, LCD_1IN44.HEIGHT*0.45), BLUE);
                                for(int i{}; i < 10; ++i){
                                        if(selectedBtn == i)
                                                drawRect(btns[i].pos-Vec2(2,2), btns[i].size+Vec2(4,4), RED);
                                        else
                                                drawRect(btns[i].pos-Vec2(1,1), btns[i].size+Vec2(2,2), BLACK);
                                        drawRect(btns[i].pos, btns[i].size, WHITE);
                                }
                                if(selectedBtn == 0){
                                        Paint_DrawString_EN(20, 32, "Platformer Test", &Font8, BLACK, WHITE);
                                }else if(selectedBtn == 1){
                                        Paint_DrawString_EN(20, 32, "Snake", &Font8, BLACK, WHITE);
                                }else{
                                        Paint_DrawString_EN(20, 32, ("Game " + std::to_string(selectedBtn+1)).c_str(), &Font8, BLACK, WHITE);
                                }
                        }
        };
};

int main(int argc, char** argv){
        // initialize
        stdio_init_all();
        DEV_Module_Init();

        LCD_1IN44_Init(VERTICAL);

        // setup screen buffer and clear screen
        UDOUBLE Imagesize = LCD_1IN44_HEIGHT*LCD_1IN44_WIDTH*2;
        UWORD *Screen = (UWORD*)malloc(Imagesize);

        Paint_NewImage((UBYTE *)Screen,LCD_1IN44.WIDTH,LCD_1IN44.HEIGHT, 0, WHITE);
        Paint_SetScale(65);
        Paint_Clear(WHITE);

        UWORD *WhiteScreen = (UWORD*)malloc(Imagesize);
        memset(WhiteScreen, 0xFF, Imagesize);

        // init input shit
        SET_Infrared_PIN(key0);    
        SET_Infrared_PIN(key1);
        SET_Infrared_PIN(key2);
        SET_Infrared_PIN(key3);

        absolute_time_t time_at_prev_frame {get_absolute_time()};
        Game game {};

        //game.addEntity(new Wall({0,0}, {8,LCD_1IN44.HEIGHT}));
        // main loop
        game.addEntity(new Home::Menu());
        while(true){
                // calculate dt
                absolute_time_t time_at_curr_frame {get_absolute_time()};
                int64_t time_diff_us {absolute_time_diff_us(time_at_prev_frame, time_at_curr_frame)};
                float dt {static_cast<float>(time_diff_us) / 1000000}; // dt in terms of seconds cause thats how my brain likes it
                int fps {static_cast<int>(std::floor(1/dt))};
                time_at_prev_frame = time_at_curr_frame;

                game.update(dt);

                game.rmEntities();
                game.addEntities();

                // clear screen
                memcpy(Screen, WhiteScreen, Imagesize);

                // draw
                if(DEBUG) Paint_DrawString_EN(90, 0, ("fps: "+std::to_string(fps)).c_str(), &Font8, BLACK, WHITE);

                game.draw();

                // send buffer to screen
                LCD_1IN44_Display(Screen);
        }
        
        return 0;
}
