#pragma once
#include "Physics.h"
#include "Resources.h"
#include <SFML/Graphics.hpp>
#include <vector>

enum class BonusType { Coin, Speed, HighJump };
enum class PlayerAnimation { Idle, Run, Jump, Collect, Hurt, Death };

class Platform : public sf::Drawable
{
private:
    Rect bounds;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    explicit Platform(const Rect& bounds);
    const Rect& getBounds() const;
};

class Player : public sf::Drawable
{
private:
    const sf::Texture& texture;
    Body body;
    int health = 100;
    int facing = 1;
    float speedRemaining = 0.f;
    float jumpRemaining = 0.f;
    float damageRemaining = 0.f;
    float collectRemaining = 0.f;
    float deathTime = 0.f;
    float animationFrame = 0.f;
    PlayerAnimation animation = PlayerAnimation::Idle;
    void setAnimation(PlayerAnimation value);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    explicit Player(const sf::Texture& texture);
    void reset();
    void update(float deltaTime, const std::vector<Rect>& platforms,
                int direction, bool jumpRequested);
    void takeDamage(int amount);
    void handleFall();
    void collect(BonusType type);
    const Body& getBody() const;
    int getHealth() const;
    float getSpeedRemaining() const;
    float getJumpRemaining() const;
    bool isDead() const;
    bool hasDisappeared() const;
};

class Enemy : public sf::Drawable
{
private:
    const sf::Texture* texture;
    Rect bounds;
    float leftLimit;
    float rightLimit;
    int direction;
    float animationTime = 0.f;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    Enemy(const sf::Texture& texture, const Rect& platform, float startFraction, int direction);
    void update(float deltaTime);
    const Rect& getBounds() const;
};

class Bonus : public sf::Drawable
{
private:
    const sf::Texture* texture;
    sf::Vector2f center;
    BonusType type;
    bool active = true;
    float respawnRemaining = 0.f;
    float animationTime;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    Bonus(const sf::Texture& texture, sf::Vector2f center, BonusType type, float phase);
    void update(float deltaTime, bool respawnEnabled);
    void collect();
    bool isActive() const;
    BonusType getType() const;
    Rect getBounds() const;
};

class World : public sf::Drawable
{
private:
    const Resources& resources;
    std::vector<Platform> platforms;
    std::vector<Rect> collisionPlatforms;
    std::vector<Enemy> enemies;
    std::vector<Bonus> bonuses;
    Player player;
    int score = 0;
    int enemyCount = 3;
    float elapsedTime = 0.f;
    void createBonuses();
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    explicit World(const Resources& resources);
    void reset(int requestedEnemyCount);
    void setEnemyCount(int count);
    void update(float deltaTime, int direction, bool jumpRequested, bool respawnEnabled);
    void drawName(sf::RenderTarget& target, const sf::String& name, sf::Color color) const;
    const Player& getPlayer() const;
    int getScore() const;
    int getEnemyCount() const;
    float getElapsedTime() const;
};

// Один и тот же спрайт дракона используется в мире и на экранах меню.
void drawDragon(sf::RenderTarget& target, const sf::Texture& texture,
                sf::Vector2f position, float scale, float time, bool facingLeft = false);
void drawScenery(sf::RenderTarget& target, const Resources& resources,
                 sf::Vector2f size, float time);
