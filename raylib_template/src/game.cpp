#include "game.h"
#include <raylib.h>

// math funcs
float lerp(float start, float end, float t){
    return start+end*t;
}

Vec2::Vec2() : x(0), y(0) {}
Vec2::Vec2(float x, float y) : x(x), y(y) {}

Vec2 Vec2::operator*(float scalar) const{
    return Vec2(x * scalar, y * scalar);
}

Vec2 Vec2::operator/(float scalar) const{
    return Vec2(x / scalar, y / scalar);
}

Vec2 Vec2::operator+(const Vec2& other) const{
    return Vec2(x + other.x, y + other.y);
}

Vec2 Vec2::operator-(const Vec2& other) const{
    return Vec2(x - other.x, y - other.y);
}

bool Vec2::operator<(const Vec2& other) const{
    return (x < other.x) || (x == other.x && y < other.y);
}

bool Vec2::operator==(const Vec2& other) const{
    return (x == other.x && y == other.y);
}

void Vec2::print() const{
    std::cout << "Vec2(" << x << ", " << y << ")\n";
}

float Vec2::dot(const Vec2& other) const{
    return x * other.x + y * other.y;
}

float Vec2::length() const{
    return std::sqrt(x * x + y * y);
}

Vec2 Vec2::normalize() const{
    float len = length();
    return Vec2(x / len, y / len);
}

bool Vec2::isZero() const{
    return (x==0 && y==0);
}

Vec2 Vec2::lerp(Vec2 otherVec, float t){
    // idk if this is weird but idk how to do this + or this - otherVec
    // so just call their operator funcs directly
    return Vec2(this->operator-((this->operator-(otherVec)) * t));
}

void Vec2::drawPoint(){
    DrawCircleLines(this->x, this->y, 10, RED);
}

void Vec2::drawLineTo(Vec2 otherVec, Color col){
    DrawLine(this->x, this->y, otherVec.x, otherVec.y, col);
}

Rect::Rect(): pos(Vec2()), size(Vec2()){}
Rect::Rect(float x, float y, float w, float h)
    : pos(Vec2(x, y)), size(Vec2(w,h)){}
Rect::Rect(Vec2 pos, Vec2 size)
    : pos(pos), size(size){}

Rect Rect::operator*(float scalar) const{
    return Rect(pos*scalar, size);
}
Rect Rect::operator/(float scalar) const{
    return Rect(pos/scalar, size);
}
Rect Rect::operator+(Vec2 vec) const{
    return Rect(pos+vec, size);
}
Rect Rect::operator-(Vec2 vec) const{
    return Rect(pos-vec, size);
}
Vec2 Rect::min(){
    return pos;
}
Vec2 Rect::max(){
    return pos+size;
}
Vec2 Rect::center(){
    return pos + size/2;
}
void Rect::setCenter(Vec2 center){
    pos = center - size/2;
}

void Rect::print(){
    std::cout << "Rect[ (" << pos.x << ", " << pos.y << "), (" << size.x << ", " << size.y << ") ]" << "\n";
}

void Rect::draw(Color col, float rot, Vec2 pivot, bool showTrueLoc){
    if(showTrueLoc){
        DrawRectangle(pos.x, pos.y, size.x, size.y, MAGENTA);
    }
    // pivot is in world space ie if the center of the screen is passed in
    // obj will rotate around center of the screen
    pivot = pivot - pos;
    DrawRectanglePro(Rectangle{pos.x+pivot.x,pos.y+pivot.y,size.x,size.y}, Vector2{pivot.x, pivot.y}, rot, col);
}

Rect Rect::scale(float xScale, float yScale, bool keepCentered){
    if(keepCentered){
        float xDiff {(xScale - 1) * size.x};
        float yDiff {(yScale - 1) * size.y};

        return Rect(pos.x - xDiff/2, pos.y - yDiff/2, size.x + xDiff, size.y + yDiff);
    }

    return Rect(pos, Vec2(size.x*xScale, size.y*yScale));
}

Vec2 Rect::AABBCollision(Rect* otherRect){
    Vec2 rectAMin {this->min()};
    Vec2 rectAMax {this->max()};
    Vec2 rectBMin {otherRect->min()};
    Vec2 rectBMax {otherRect->max()};

    // check collision
    if(rectAMax.x <= rectBMin.x || rectAMin.x >= rectBMax.x || rectAMax.y <= rectBMin.y || rectAMin.y >= rectBMax.y){
    }

    float overlapX {0.0f};
    if(rectAMax.x > rectBMin.x && rectAMin.x < rectBMax.x){
        overlapX = std::min(rectAMax.x - rectBMin.x, rectBMax.x - rectAMin.x);
    }

    float overlapY {0.0f};
    if(rectAMax.y > rectBMin.y && rectAMin.y < rectBMax.y){
        overlapY = std::min(rectAMax.y - rectBMin.y, rectBMax.y - rectAMin.y);
    }

    if(overlapX < overlapY){
        return Vec2(overlapX, 0);
    }else{
        return Vec2(0, overlapY);
    }
}

/*/
 * Mouse definition
/*/
void Mouse::update(float dt){
    pos = Vec2(GetMousePosition().x, GetMousePosition().y);

    pressed.left = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    pressed.middle = IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);
    pressed.right = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);

    down.left = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    down.middle = IsMouseButtonDown(MOUSE_BUTTON_MIDDLE);
    down.right = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
}
Rect* Mouse::rect(){
    return new Rect(pos, Vec2());
}

/*/
 * Game definition
/*/
Game::Game(int init_windowW, int init_windowH, std::string init_title)
    : windowW(init_windowW), windowH(init_windowH), title(init_title)
{
    InitWindow(windowW, windowH, title.c_str());
    SetTargetFPS(360);

    std::cout << windowW << "x" << windowH << "\n";
}

void Game::add_entity(Entity* entity){
    entities.push_back(entity);
}

void Game::update(float dt){
    for(Entity* e : entities){
        e->update(dt);
    }

    mouse.update(dt);
}

void Game::draw(){
    BeginDrawing();
    ClearBackground(WHITE);

    for(Entity* e : entities){
        e->draw();
    }

    EndDrawing();
}

void Game::main(){
    while(running){
        float dt {GetFrameTime()};
        update(dt);
        draw();

        running = running && !WindowShouldClose(); // can exit with raylib or by setting running=false
    }

    CloseWindow();
}
