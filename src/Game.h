#pragma once
#include "Resources.h"
#include "World.h"
#include "PauseMenu.h"

enum class GameState { Start, Playing, Paused, GameOver };

class Game
{
private:
    // Порядок важен: сначала окно, затем ресурсы, затем использующие их объекты.
    sf::RenderWindow window;
    Resources resources;
    World world;
    PauseMenu menu;
    Button primaryButton;
    Button secondaryButton;
    Button exitButton;
    sf::View worldView;
    sf::View uiView;
    sf::Vector2f uiSize;
    GameState state = GameState::Start;
    bool pauseFromStart = false;
    bool jumpWasDown = false;
    bool jumpQueued = false;
    float accumulator = 0.f;
    float menuTime = 0.f;
    void updateViews(unsigned int width, unsigned int height);
    void processEvents();
    sf::Vector2f getMousePosition(const sf::Event& event) const;
    void startGame();
    void openPause(bool fromStart);
    void closePause();
    void handleMenuAction(MenuAction action);
    void update(float deltaTime);
    void draw();
    void drawHud();
    void drawStart();
    void drawGameOver();
public:
    Game();
    void run();
};
