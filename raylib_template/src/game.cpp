#include "game.h"
#include <memory>
#include <raylib.h>

// math funcs
float lerp(float start, float end, float t){
    return start+(end-start)*t;
}

Vec2::Vec2() : x(0), y(0) {}
Vec2::Vec2(float x, float y) : x(x), y(y) {}

Vec2 Vec2::operator*(float scalar) const{
    return Vec2(x * scalar, y * scalar);
}
inline Vec2 operator*(double scalar, const Vec2& v){
    return v*scalar;
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

// used for sorting, less than comparison operator
// returns bool(x < other.x) or (y < other.y) if x == other.x
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
    if(isZero()) return Vec2();
    float len = length();
    return Vec2(x / len, y / len);
}

bool Vec2::isZero() const{
    return (x==0 && y==0);
}

Vec2 Vec2::lerp(const Vec2& otherVec, float t){
    return (*this) + (otherVec - *this)*t;
}

void Vec2::drawPoint() const{
    DrawCircleLines(this->x, this->y, 10, RED);
}

void Vec2::drawLineTo(const Vec2& otherVec, Color col) const{
    DrawLine(this->x, this->y, otherVec.x, otherVec.y, col);
}

Rect::Rect(): pos(Vec2()), size(Vec2()){}

Rect::Rect(float x, float y, float w, float h)
    : pos(Vec2(x, y)), size(Vec2(w,h)){}

Rect::Rect(const Vec2& pos, const Vec2& size)
    : pos(pos), size(size){}

Rect Rect::operator*(float scalar) const{
    return Rect(pos*scalar, size);
}
Rect Rect::operator/(float scalar) const{
    return Rect(pos/scalar, size);
}
Rect Rect::operator+(const Vec2& vec) const{
    return Rect(pos+vec, size);
}
Rect Rect::operator-(const Vec2& vec) const{
    return Rect(pos-vec, size);
}
Vec2 Rect::min() const{
    return pos;
}
Vec2 Rect::max() const{
    return pos+size;
}
Vec2 Rect::center() const{
    return pos + size/2;
}
void Rect::setCenter(const Vec2& center){
    pos = center - size/2;
}

void Rect::print() const{
    std::cout << "Rect[ (" << pos.x << ", " << pos.y << "), (" << size.x << ", " << size.y << ") ]" << "\n";
}

void Rect::draw(Color col, float rot, Vec2 pivot, bool showTrueLoc) const{
    if(showTrueLoc){
        DrawRectangle(pos.x, pos.y, size.x, size.y, MAGENTA);
    }
    // pivot is in world space ie if the center of the screen is passed in
    // obj will rotate around center of the screen
    pivot = pivot - pos;
    DrawRectanglePro(Rectangle{pos.x+pivot.x,pos.y+pivot.y,size.x,size.y}, Vector2{pivot.x, pivot.y}, rot, col);
}

Rect Rect::scale(float xScale, float yScale, bool keepCentered) const{
    if(keepCentered){
        float xDiff {(xScale - 1) * size.x};
        float yDiff {(yScale - 1) * size.y};

        return Rect(pos.x - xDiff/2, pos.y - yDiff/2, size.x + xDiff, size.y + yDiff);
    }

    return Rect(pos, Vec2(size.x*xScale, size.y*yScale));
}

Vec2 Rect::AABBCollision(const Rect& otherRect) const{
    Vec2 rectAMin {this->min()};
    Vec2 rectAMax {this->max()};
    Vec2 rectBMin {otherRect.min()};
    Vec2 rectBMax {otherRect.max()};

    // check collision
    if(rectAMax.x <= rectBMin.x || rectAMin.x >= rectBMax.x || rectAMax.y <= rectBMin.y || rectAMin.y >= rectBMax.y){
        return Vec2();
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
 * PixelTexture definition
/*/
PixelTexture::PixelTexture(int width, int height){
    this->width = width;
    this->height = height;

    pixelArray = static_cast<Color*>(malloc(width*height * sizeof(Color))); // allocate w*h * 4 bytes

    pixelImage.data = pixelArray;
    pixelImage.width = width;
    pixelImage.height = height;
    pixelImage.mipmaps = 1;
    pixelImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    pixelTexture = LoadTextureFromImage(pixelImage);
}

PixelTexture::~PixelTexture(){
    UnloadTexture(pixelTexture);
    free(pixelArray);
}

PixelTexture::PixelTexture(PixelTexture&& other) noexcept{
    this->pixelArray = other.pixelArray;
    this->pixelImage = other.pixelImage;
    this->pixelTexture = other.pixelTexture;
    this->width = other.width;
    this->height = other.height;

    // zero out other
    other.pixelArray = nullptr;
    other.pixelImage = {};
    other.pixelTexture = {};
    other.width = 0;
    other.height = 0;
}

PixelTexture& PixelTexture::operator=(PixelTexture&& other) noexcept{
    if(this != &other){
        UnloadTexture(this->pixelTexture);
        UnloadImage(this->pixelImage);
        free(this->pixelArray);

        this->pixelArray = other.pixelArray;
        this->pixelImage = other.pixelImage;
        this->pixelTexture = other.pixelTexture;
        this->width = other.width;
        this->height = other.height;

        // zero out other
        other.pixelArray = nullptr;
        other.pixelImage = {};
        other.pixelTexture = {};
        other.width = 0;
        other.height = 0;
    }

    return *this;
}

void PixelTexture::setPixel(int x, int y, const Color& c){
    // TODO: maybe add err handling for out of bounds index assignment
    int convertedIndex {y*width + x};
    pixelArray[convertedIndex].r = c.r;
    pixelArray[convertedIndex].g = c.g;
    pixelArray[convertedIndex].b = c.b;
    pixelArray[convertedIndex].a = c.a;
}

// used this way for "batch" calls
// ie shd use setPixel as much as needed in given frame
// then use setTexture before drawing
void PixelTexture::setTexture(){
    UpdateTexture(pixelTexture, pixelArray);
}

// setTexture must be called before this at least once
void PixelTexture::draw(float x, float y, float w, float h){
    Rectangle src {0,0,static_cast<float>(width),static_cast<float>(height)};
    Rectangle dest {x,y,w,h};
    Vector2 orig {0,0};
    float rot {};
    DrawTexturePro(pixelTexture, src, dest, orig, rot, WHITE);
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
Rect Mouse::rect() const{
    return Rect(pos, Vec2());
}

/*/
 * Game definition
/*/
Game::Game(int init_windowW, int init_windowH, std::string init_title)
    : windowW(init_windowW), windowH(init_windowH), title(init_title)
{
    InitWindow(windowW, windowH, title.c_str());
    SetTargetFPS(60);

    std::cout << "Window created! (" << windowW << "x" << windowH << ")\n";
}

Game::~Game(){
    entities.clear();
    CloseWindow();
}

void Game::addEntity(std::unique_ptr<Entity> entity){
    entities.push_back(std::move(entity));
}

void Game::update(float dt){
    for(std::unique_ptr<Entity>& e : entities){
        e->update(dt);
    }

    mouse.update(dt);
}

void Game::draw(){
    BeginDrawing();
    ClearBackground(WHITE);

    for(std::unique_ptr<Entity>& e : entities){
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
}
