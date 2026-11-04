#include "Physics.h"
#include "Config.h"
#include <algorithm>
#include <cmath>

bool Physics::intersects(const Rect& first, const Rect& second)
{
    return first.x < second.x + second.width &&
           first.x + first.width > second.x &&
           first.y < second.y + second.height &&
           first.y + first.height > second.y;
}

float Physics::approach(float value, float target, float maximumChange)
{
    if (value < target)
    {
        return std::min(value + maximumChange, target);
    }
    return std::max(value - maximumChange, target);
}

void Physics::update(Body& body, const std::vector<Rect>& platforms,
                     int direction, bool jumpRequested, float maximumSpeed,
                     float jumpSpeed, float deltaTime)
{
    if (deltaTime <= 0.f)
    {
        return;
    }
    float acceleration = Config::Acceleration;
    if (direction == 0)
    {
        acceleration = Config::Braking;
    }
    body.velocityX = approach(body.velocityX, direction * maximumSpeed,
                              acceleration * deltaTime);
    if (jumpRequested && body.grounded)
    {
        body.velocityY = -jumpSpeed;
        body.grounded = false;
    }
    body.velocityY += Config::Gravity * deltaTime;
    body.velocityY = std::min(body.velocityY, 1000.f);

    // Ограничиваем путь за подшаг, чтобы не пролетать сквозь тонкую платформу.
    float maximumDistance = std::max(std::abs(body.velocityX * deltaTime),
                                     std::abs(body.velocityY * deltaTime));
    int steps = std::max(1, static_cast<int>(std::ceil(maximumDistance / 4.f)));
    float stepTime = deltaTime / static_cast<float>(steps);
    body.grounded = false;

    for (int step = 0; step < steps; step++)
    {
        // Сначала разрешаем столкновения по горизонтали.
        float movementX = body.velocityX * stepTime;
        body.bounds.x += movementX;
        for (std::size_t i = 0; i < platforms.size(); i++)
        {
            if (!intersects(body.bounds, platforms[i]))
            {
                continue;
            }
            if (movementX > 0.f)
            {
                body.bounds.x = platforms[i].x - body.bounds.width;
                body.velocityX = 0.f;
            }
            else if (movementX < 0.f)
            {
                body.bounds.x = platforms[i].x + platforms[i].width;
                body.velocityX = 0.f;
            }
        }
        // Затем по вертикали: и посадка сверху, и удар головой снизу.
        float movementY = body.velocityY * stepTime;
        body.bounds.y += movementY;
        for (std::size_t i = 0; i < platforms.size(); i++)
        {
            if (!intersects(body.bounds, platforms[i]))
            {
                continue;
            }
            if (movementY > 0.f)
            {
                body.bounds.y = platforms[i].y - body.bounds.height;
                body.velocityY = 0.f;
                body.grounded = true;
            }
            else if (movementY < 0.f)
            {
                body.bounds.y = platforms[i].y + platforms[i].height;
                body.velocityY = 0.f;
            }
        }
    }

    // При нулевой вертикальной скорости соприкосновение не даёт пересечения.
    // Проверяем опору небольшим пробником под ногами, но только не в полёте вверх.
    if (body.velocityY >= 0.f)
    {
        Rect feet = body.bounds;
        feet.y += 0.5f;
        body.grounded = false;
        for (std::size_t i = 0; i < platforms.size(); i++)
        {
            if (intersects(feet, platforms[i]) &&
                body.bounds.y + body.bounds.height <= platforms[i].y + 0.6f)
            {
                body.grounded = true;
                break;
            }
        }
    }
}
