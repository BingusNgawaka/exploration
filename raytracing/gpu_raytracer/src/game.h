#include "raylib.h"
#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

// only really makes sense with 0 <= t <= 1
float lerp(float start, float end, float t);

// these are kinda just my own versions of raylibs shit bcus using them makes me angry

// Vec2 definition with addition scal mult
// normalize, getLength, isZero etc.
struct Vec2 {
    float x, y;

    Vec2();
    Vec2(float x, float y);

    Vec2 operator*(float scalar) const;
    Vec2 operator/(float scalar) const;
    Vec2 operator+(const Vec2& other) const;
    Vec2 operator-(const Vec2& other) const;

    // used for sorting, less than comparison operator
    // returns bool(x < other.x) or (y < other.y) if x == other.x
    bool operator<(const Vec2& other) const;
    bool operator==(const Vec2& other) const;

    float dot(const Vec2& other) const;
    float length() const;
    Vec2 normalize() const;
    bool isZero() const;
    Vec2 lerp(const Vec2& otherVec, float t);

    void print() const;
    void drawPoint() const;
    void drawLineTo(const Vec2& otherVec, Color col) const;
};
inline Vec2 operator*(double scalar, const Vec2& v); // commutative scalar mult

// Rect definition, def with either (pos, size) or (x,y,w,h)
// scal mult and div for pos, scale to scalar mult size
// get center etc.
struct Rect{
    Vec2 pos;
    Vec2 size;

    Rect();
    Rect(float x, float y, float w, float h);
    Rect(const Vec2& pos, const Vec2& size);

    Rect operator*(float scalar) const;
    Rect operator/(float scalar) const;
    Rect operator+(const Vec2& vec) const;
    Rect operator-(const Vec2& vec) const;

    Vec2 min() const;
    Vec2 max() const;
    Vec2 center() const;
    void setCenter(const Vec2& center);

    void print() const;
    void draw(Color col, float rot=0.0f, Vec2 pivot=Vec2(), bool showTrueLoc=false) const;

    Rect scale(float xScale, float yScale, bool keepCentered=true) const;

    // returns MTV if collision else returns zero vector
    Vec2 AABBCollision(const Rect& otherRect) const;
};

struct PixelTexture{
    Color* pixelArray;
    Image pixelImage;
    Texture2D pixelTexture;

    int width;
    int height;

    PixelTexture() = delete;
    PixelTexture(int width, int height);

    // rule of 5 type shi
    ~PixelTexture();

    // disable copy
    PixelTexture(const PixelTexture&) = delete;
    PixelTexture& operator=(const PixelTexture&) = delete;

    // allow move
    PixelTexture(PixelTexture&& other) noexcept;
    PixelTexture& operator=(PixelTexture&& other) noexcept;

    void setPixel(int x, int y, const Color& c);
    void setTexture();
    void draw(float x, float y, float w, float h);
};


class Entity{
    public:
        virtual void update(float dt) = 0;
        virtual void draw() = 0;
        virtual ~Entity() = default;
};

struct Mouse{
    Vec2 pos;
    struct ButtonState{
        bool left {};
        bool middle {};
        bool right {};
    };

    ButtonState pressed {};
    ButtonState down {};

    void update(float dt);
    Rect rect() const;
};

class Game{
    public:
        int windowW {};
        int windowH {};
        std::string title {};
        bool running {true};

        Mouse mouse {};

        std::vector<std::unique_ptr<Entity>> entities;

        ~Game();
        Game(int screenW, int screenH, std::string title);

        void addEntity(std::unique_ptr<Entity> entity);

        void update(float dt);
        void draw();
        void main();
};
