#include "World.h"
#include "Config.h"
#include <algorithm>
#include <cmath>

Platform::Platform(const Rect& bounds) : bounds(bounds)
{
}

const Rect& Platform::getBounds() const
{
    return bounds;
}

void Platform::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    sf::RectangleShape rock(sf::Vector2f(bounds.width, bounds.height));
    rock.setPosition(bounds.x, bounds.y);
    rock.setFillColor(sf::Color(41, 59, 72));
    rock.setOutlineColor(sf::Color(90, 122, 137));
    rock.setOutlineThickness(-1.f);
    target.draw(rock, states);
    for (float x = 18.f; x < bounds.width; x += 48.f)
    {
        sf::RectangleShape seam(sf::Vector2f(2.f, bounds.height - 8.f));
        seam.setPosition(bounds.x + x, bounds.y + 7.f);
        seam.setFillColor(sf::Color(26, 41, 54));
        target.draw(seam, states);
    }
    sf::RectangleShape snow(sf::Vector2f(bounds.width + 4.f, 6.f));
    snow.setPosition(bounds.x - 2.f, bounds.y);
    snow.setFillColor(sf::Color(195, 219, 226));
    target.draw(snow, states);
    sf::RectangleShape highlight(sf::Vector2f(bounds.width, 2.f));
    highlight.setPosition(bounds.x, bounds.y + 6.f);
    highlight.setFillColor(sf::Color(110, 161, 179));
    target.draw(highlight, states);
}

Player::Player(const sf::Texture& texture) : texture(texture)
{
    reset();
}

void Player::reset()
{
    body = Body();
    body.bounds = Rect{105.f, 588.f, 34.f, 62.f};
    body.grounded = true;
    health = Config::MaximumHealth;
    facing = 1;
    speedRemaining = 0.f;
    jumpRemaining = 0.f;
    damageRemaining = 0.f;
    collectRemaining = 0.f;
    deathTime = 0.f;
    animationFrame = 0.f;
    animation = PlayerAnimation::Idle;
}

void Player::setAnimation(PlayerAnimation value)
{
    if (animation != value)
    {
        animation = value;
        animationFrame = 0.f;
    }
}

void Player::update(float deltaTime, const std::vector<Rect>& platforms,
                     int direction, bool jumpRequested)
{
    if (isDead())
    {
        deathTime += deltaTime;
        setAnimation(PlayerAnimation::Death);
        animationFrame = std::min(5.f, deathTime * 5.f);
        return;
    }
    speedRemaining = std::max(0.f, speedRemaining - deltaTime);
    jumpRemaining = std::max(0.f, jumpRemaining - deltaTime);
    damageRemaining = std::max(0.f, damageRemaining - deltaTime);
    collectRemaining = std::max(0.f, collectRemaining - deltaTime);
    float maximumSpeed = Config::WalkSpeed;
    float jumpSpeed = Config::JumpSpeed;
    if (speedRemaining > 0.f)
    {
        maximumSpeed *= 1.6f;
    }
    if (jumpRemaining > 0.f)
    {
        jumpSpeed *= 1.3f;
    }
    Physics::update(body, platforms, direction, jumpRequested,
                    maximumSpeed, jumpSpeed, deltaTime);
    if (direction != 0)
    {
        facing = direction;
    }
    if (damageRemaining > 0.8f)
    {
        setAnimation(PlayerAnimation::Hurt);
    }
    else if (collectRemaining > 0.f)
    {
        setAnimation(PlayerAnimation::Collect);
    }
    else if (!body.grounded)
    {
        setAnimation(PlayerAnimation::Jump);
    }
    else if (std::abs(body.velocityX) > 15.f)
    {
        setAnimation(PlayerAnimation::Run);
    }
    else
    {
        setAnimation(PlayerAnimation::Idle);
    }
    float framesPerSecond = 6.f;
    if (animation == PlayerAnimation::Run)
    {
        // Быстрее движемся — быстрее переставляем ноги, а не просто скользим.
        framesPerSecond = std::abs(body.velocityX) / 25.f;
    }
    animationFrame = std::fmod(animationFrame + framesPerSecond * deltaTime, 6.f);
}

void Player::takeDamage(int amount)
{
    if (isDead() || damageRemaining > 0.f)
    {
        return;
    }
    health = std::max(0, health - amount);
    damageRemaining = Config::DamageCooldown;
    if (health == 0)
    {
        deathTime = 0.f;
        body.velocityX = 0.f;
        body.velocityY = 0.f;
        setAnimation(PlayerAnimation::Death);
    }
}

void Player::handleFall()
{
    if (isDead())
    {
        return;
    }
    // Падение всегда снимает здоровье: защита от врага не отменяет этот штраф.
    damageRemaining = 0.f;
    takeDamage(Config::FallDamage);
    body.bounds.x = 105.f;
    body.bounds.y = 588.f;
    body.velocityX = 0.f;
    body.velocityY = 0.f;
    body.grounded = true;
}

void Player::collect(BonusType type)
{
    collectRemaining = 0.25f;
    if (type == BonusType::Speed)
    {
        speedRemaining = Config::EffectDuration;
    }
    else if (type == BonusType::HighJump)
    {
        jumpRemaining = Config::EffectDuration;
    }
}

const Body& Player::getBody() const { return body; }
int Player::getHealth() const { return health; }
float Player::getSpeedRemaining() const { return speedRemaining; }
float Player::getJumpRemaining() const { return jumpRemaining; }
bool Player::isDead() const { return health == 0; }
bool Player::hasDisappeared() const { return isDead() && deathTime >= 1.4f; }

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (hasDisappeared())
    {
        return;
    }
    int frame = static_cast<int>(animationFrame);
    int row = static_cast<int>(animation);
    sf::Sprite sprite(texture, sf::IntRect(frame * 96, row * 96, 96, 96));
    sprite.setOrigin(48.f, 82.f);
    sprite.setPosition(body.bounds.x + body.bounds.width / 2.f,
                        body.bounds.y + body.bounds.height);
    sprite.setScale(static_cast<float>(facing), 1.f);
    if (!body.grounded && !isDead())
    {
        sprite.setRotation(facing * 5.f);
    }
    if (damageRemaining > 0.f && !isDead())
    {
        if (static_cast<int>(damageRemaining * 16.f) % 2 == 0)
        {
            sprite.setColor(sf::Color(255, 160, 160, 150));
        }
    }
    if (isDead())
    {
        float progress = std::min(1.f, deathTime / 1.4f);
        sprite.setColor(sf::Color(220, 230, 240,
                         static_cast<sf::Uint8>(255.f * (1.f - progress))));
        sprite.move(0.f, -progress * 20.f);
        sprite.setRotation(facing * progress * 16.f);
        // Плащ распадается на летящие тёмные перья: простой эффект из треугольников.
        for (int i = 0; i < 16; i++)
        {
            float phase = i * 2.39996f;
            float distance = progress * (45.f + i * 4.f);
            sf::CircleShape feather(4.f, 3);
            feather.setPosition(sprite.getPosition().x + std::cos(phase) * distance,
                                 sprite.getPosition().y - 40.f - progress * 65.f +
                                 std::sin(phase) * distance * 0.5f);
            feather.setScale(0.6f, 1.8f);
            feather.setRotation(i * 31.f + progress * 170.f);
            feather.setFillColor(sf::Color(119, 158, 178,
                                  static_cast<sf::Uint8>(255.f * (1.f - progress))));
            target.draw(feather, states);
        }
    }
    target.draw(sprite, states);
    if (collectRemaining > 0.f)
    {
        float progress = 1.f - collectRemaining / 0.25f;
        sf::CircleShape ring(22.f + progress * 24.f);
        ring.setOrigin(ring.getRadius(), ring.getRadius());
        ring.setPosition(body.bounds.x + 17.f, body.bounds.y + 30.f);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(2.f);
        ring.setOutlineColor(sf::Color(239, 201, 119,
                             static_cast<sf::Uint8>(220.f * (1.f - progress))));
        target.draw(ring, states);
    }
}

Enemy::Enemy(const sf::Texture& texture, const Rect& platform,
               float startFraction, int direction)
    : texture(&texture), leftLimit(platform.x + 8.f),
      rightLimit(platform.x + platform.width - 40.f), direction(direction)
{
    bounds = Rect{leftLimit + (rightLimit - leftLimit) * startFraction,
                  platform.y - 50.f, 32.f, 50.f};
}

void Enemy::update(float deltaTime)
{
    bounds.x += direction * 78.f * deltaTime;
    if (bounds.x <= leftLimit)
    {
        bounds.x = leftLimit;
        direction = 1;
    }
    else if (bounds.x >= rightLimit)
    {
        bounds.x = rightLimit;
        direction = -1;
    }
    animationTime = std::fmod(animationTime + deltaTime, 100.f);
}

const Rect& Enemy::getBounds() const { return bounds; }

void Enemy::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    int frame = static_cast<int>(animationTime * 7.f) % 6;
    sf::Sprite sprite(*texture, sf::IntRect(frame * 96, 0, 96, 96));
    sprite.setOrigin(48.f, 82.f);
    sprite.setScale(direction * 0.8f, 0.8f);
    sprite.setPosition(bounds.x + 16.f, bounds.y + bounds.height);
    target.draw(sprite, states);
}

Bonus::Bonus(const sf::Texture& texture, sf::Vector2f center,
               BonusType type, float phase)
    : texture(&texture), center(center), type(type), animationTime(phase)
{
}

void Bonus::update(float deltaTime, bool respawnEnabled)
{
    animationTime = std::fmod(animationTime + deltaTime, 100.f);
    if (!active && respawnEnabled)
    {
        respawnRemaining -= deltaTime;
        if (respawnRemaining <= 0.f)
        {
            active = true;
        }
    }
}

void Bonus::collect()
{
    active = false;
    respawnRemaining = Config::BonusRespawnDelay;
}

bool Bonus::isActive() const { return active; }
BonusType Bonus::getType() const { return type; }
Rect Bonus::getBounds() const { return Rect{center.x - 14.f, center.y - 18.f, 28.f, 36.f}; }

void Bonus::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (!active)
    {
        return;
    }
    int frame = static_cast<int>(animationTime * 8.f) % 6;
    sf::Sprite sprite(*texture, sf::IntRect(frame * 64, static_cast<int>(type) * 64, 64, 64));
    sprite.setOrigin(32.f, 32.f);
    sprite.setScale(0.8f, 0.8f);
    sprite.setPosition(center.x, center.y + std::sin(animationTime * 3.f) * 4.f);
    target.draw(sprite, states);
}

World::World(const Resources& resources) : resources(resources), player(resources.player)
{
    collisionPlatforms.push_back(Rect{60.f, 650.f, 1160.f, 70.f});
    collisionPlatforms.push_back(Rect{150.f, 535.f, 240.f, 28.f});
    collisionPlatforms.push_back(Rect{430.f, 420.f, 250.f, 28.f});
    collisionPlatforms.push_back(Rect{800.f, 535.f, 260.f, 28.f});
    collisionPlatforms.push_back(Rect{800.f, 305.f, 260.f, 28.f});
    for (std::size_t i = 0; i < collisionPlatforms.size(); i++)
    {
        platforms.push_back(Platform(collisionPlatforms[i]));
    }
    reset(3);
}

void World::createBonuses()
{
    bonuses.clear();
    bonuses.push_back(Bonus(resources.bonuses, sf::Vector2f(280.f, 615.f), BonusType::Speed, 0.f));
    bonuses.push_back(Bonus(resources.bonuses, sf::Vector2f(690.f, 615.f), BonusType::Coin, 0.2f));
    bonuses.push_back(Bonus(resources.bonuses, sf::Vector2f(1110.f, 615.f), BonusType::Coin, 0.4f));
    for (std::size_t i = 1; i < collisionPlatforms.size(); i++)
    {
        const Rect& platform = collisionPlatforms[i];
        BonusType type = BonusType::Coin;
        if (i == 1)
        {
            type = BonusType::HighJump;
        }
        bonuses.push_back(Bonus(resources.bonuses,
            sf::Vector2f(platform.x + platform.width * 0.30f, platform.y - 35.f),
            type, static_cast<float>(i) * 0.3f));
        bonuses.push_back(Bonus(resources.bonuses,
            sf::Vector2f(platform.x + platform.width * 0.73f, platform.y - 35.f),
            BonusType::Coin, static_cast<float>(i) * 0.6f));
    }
}

void World::reset(int requestedEnemyCount)
{
    player.reset();
    score = 0;
    elapsedTime = 0.f;
    createBonuses();
    enemies.clear();
    setEnemyCount(requestedEnemyCount);
}

void World::setEnemyCount(int count)
{
    count = std::max(0, std::min(count, 5));
    // Удаляем/добавляем только нужное количество, не сбрасывая остальных врагов.
    while (static_cast<int>(enemies.size()) > count)
    {
        enemies.pop_back();
    }
    while (static_cast<int>(enemies.size()) < count)
    {
        std::size_t index = enemies.size();
        std::size_t platformIndex = index;
        float fraction = 0.68f;
        int direction = -1;
        if (index % 2 == 1)
        {
            fraction = 0.8f;
            direction = 1;
        }
        enemies.push_back(Enemy(resources.enemy, collisionPlatforms[platformIndex], fraction, direction));
    }
    enemyCount = count;
}

void World::update(float deltaTime, int direction, bool jumpRequested, bool respawnEnabled)
{
    elapsedTime += deltaTime;
    player.update(deltaTime, collisionPlatforms, direction, jumpRequested);
    if (player.isDead())
    {
        return;
    }
    if (player.getBody().bounds.y > Config::WorldHeight + 140.f)
    {
        player.handleFall();
    }
    if (player.isDead())
    {
        return;
    }
    for (std::size_t i = 0; i < enemies.size(); i++)
    {
        enemies[i].update(deltaTime);
        if (Physics::intersects(player.getBody().bounds, enemies[i].getBounds()))
        {
            player.takeDamage(Config::ContactDamage);
        }
    }
    if (player.isDead())
    {
        return;
    }
    for (std::size_t i = 0; i < bonuses.size(); i++)
    {
        bonuses[i].update(deltaTime, respawnEnabled);
        if (bonuses[i].isActive() && Physics::intersects(player.getBody().bounds, bonuses[i].getBounds()))
        {
            player.collect(bonuses[i].getType());
            if (bonuses[i].getType() == BonusType::Coin)
            {
                score += 10;
            }
            bonuses[i].collect();
        }
    }
}

const Player& World::getPlayer() const { return player; }
int World::getScore() const { return score; }
int World::getEnemyCount() const { return enemyCount; }
float World::getElapsedTime() const { return elapsedTime; }

void World::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for (std::size_t i = 0; i < platforms.size(); i++) { target.draw(platforms[i], states); }
    for (std::size_t i = 0; i < bonuses.size(); i++) { target.draw(bonuses[i], states); }
    for (std::size_t i = 0; i < enemies.size(); i++) { target.draw(enemies[i], states); }
    target.draw(player, states);
}

void World::drawName(sf::RenderTarget& target, const sf::String& name, sf::Color color) const
{
    if (player.isDead() || name.isEmpty())
    {
        return;
    }
    sf::Text label(name, resources.font, 17);
    label.setFillColor(color);
    label.setOutlineColor(sf::Color(8, 18, 29));
    label.setOutlineThickness(1.5f);
    sf::FloatRect local = label.getLocalBounds();
    label.setOrigin(local.left + local.width / 2.f, local.top + local.height);
    const Rect& bounds = player.getBody().bounds;
    label.setPosition(bounds.x + bounds.width / 2.f, bounds.y - 16.f);
    target.draw(label);
}

void drawDragon(sf::RenderTarget& target, const sf::Texture& texture,
                 sf::Vector2f position, float scale, float time, bool facingLeft)
{
    int frame = static_cast<int>(time * 7.f) % 6;
    sf::Sprite dragon(texture, sf::IntRect(frame * 192, 0, 192, 128));
    dragon.setOrigin(96.f, 64.f);
    float horizontalScale = scale;
    if (facingLeft)
    {
        horizontalScale = -scale;
    }
    dragon.setScale(horizontalScale, scale);
    dragon.setPosition(position);
    dragon.setRotation(std::sin(time * 1.2f) * 4.f);
    target.draw(dragon);
}

void drawScenery(sf::RenderTarget& target, const Resources& resources,
                  sf::Vector2f size, float time)
{
    sf::Sprite background(resources.background);
    background.setScale(size.x / resources.background.getSize().x,
                         size.y / resources.background.getSize().y);
    target.draw(background);
    float phase = time * 0.065f;
    float x = size.x * (0.53f + std::sin(phase) * 0.33f);
    float y = size.y * (0.22f + std::sin(time * 0.35f) * 0.018f);
    drawDragon(target, resources.dragon, sf::Vector2f(x, y), size.x / 1650.f,
                time, std::cos(phase) < 0.f);
    // Снег детерминированный, без случайного генератора и отдельной системы частиц.
    for (int i = 0; i < 65; i++)
    {
        float radius = 1.f + static_cast<float>(i % 3) * 0.55f;
        sf::CircleShape flake(radius);
        float snowX = std::fmod(i * 97.f + time * (8.f + i % 4), size.x);
        float snowY = std::fmod(i * 61.f + time * (18.f + i % 13), size.y);
        flake.setPosition(snowX, snowY);
        flake.setFillColor(sf::Color(213, 231, 239, 80));
        target.draw(flake);
    }
}
