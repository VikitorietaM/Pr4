#pragma once
#include <vector>

// Физика не зависит от SFML: её можно проверить отдельно от окна и графики.
struct Rect
{
    float x;
    float y;
    float width;
    float height;

    Rect()
        : x(0.f), y(0.f), width(0.f), height(0.f)
    {
    }

    Rect(float x, float y, float width, float height)
        : x(x), y(y), width(width), height(height)
    {
    }
};

struct Body
{
    Rect bounds;
    float velocityX = 0.f;
    float velocityY = 0.f;
    bool grounded = false;
};

namespace Physics
{
    bool intersects(const Rect& first, const Rect& second);
    float approach(float value, float target, float maximumChange);
    void update(Body& body, const std::vector<Rect>& platforms,
                int direction, bool jumpRequested, float maximumSpeed,
                float jumpSpeed, float deltaTime);
}
