#include "raylib.h"
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

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
    bool operator<(const Vec2& other) const;
    bool operator==(const Vec2& other) const;
    float dot(const Vec2& other) const;

    float length() const;
    Vec2 normalize() const;
    bool isZero() const;
    Vec2 lerp(Vec2 otherVec, float t);

    void print() const;
    void drawPoint();
    void drawLineTo(Vec2 otherVec, Color col);
};

// Rect definition, def with either (pos, size) or (x,y,w,h)
// scal mult and div for pos, scale to scalar mult size
// get center etc.
struct Rect{
    Vec2 pos;
    Vec2 size;

    Rect();
    Rect(float x, float y, float w, float h);
    Rect(Vec2 pos, Vec2 size);

    Rect operator*(float scalar) const;
    Rect operator/(float scalar) const;
    Rect operator+(Vec2 vec) const;
    Rect operator-(Vec2 vec) const;

    Vec2 min();
    Vec2 max();
    Vec2 center();
    void setCenter(Vec2 center);

    void print();
    void draw(Color col, float rot=0.0f, Vec2 pivot=Vec2(), bool showTrueLoc=false);

    Rect scale(float xScale, float yScale, bool keepCentered=true);
    Vec2 AABBCollision(Rect* otherRect);
};


class Entity{
    public:
        virtual void update(float dt) = 0;
        virtual void draw() = 0;
};

class Mouse{
    public:
        Vec2 pos;
        struct{
            bool left {};
            bool middle {};
            bool right {};
        } pressed;

        struct{
            bool left {};
            bool middle {};
            bool right {};
        } down;

        void update(float dt);
        Rect* rect();
};

class Game{
    public:
        int windowW {};
        int windowH {};
        std::string title {};
        bool running {true};

        Mouse mouse {};

        std::vector<Entity*> entities;

        Game(int screenW, int screenH, std::string title);
        void add_entity(Entity* entity);

        void update(float dt);
        void draw();
        void main();
};
