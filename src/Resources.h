#pragma once
#include <SFML/Graphics.hpp>

// Конкретные ресурсы маленькой игры: без шаблонного кеша и одиночки Singleton.
class Resources
{
public:
    Resources();
    sf::Font font;
    sf::Texture background;
    sf::Texture player;
    sf::Texture enemy;
    sf::Texture dragon;
    sf::Texture bonuses;

    void load();
};
