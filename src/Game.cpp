#include "Game.h"
#include "Config.h"
#include <algorithm>
#include <cmath>
#include <string>

static void drawCenteredLabel(sf::RenderTarget& target, const sf::Font& font,
                               const sf::String& value, float centerX, float top,
                               unsigned int fontSize, sf::Color color)
{
    sf::Text text(value, font, fontSize);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top);
    text.setPosition(centerX, top);
    text.setFillColor(color);
    target.draw(text);
}

Game::Game()
    : window(sf::VideoMode(1280, 720), "The Watch - Wings over the Wall", sf::Style::Default),
      resources(), world(resources), menu(resources.font),
      primaryButton(resources.font, sf::FloatRect(0.f, 0.f, 280.f, 48.f), L""),
      secondaryButton(resources.font, sf::FloatRect(0.f, 0.f, 280.f, 48.f), L""),
      exitButton(resources.font, sf::FloatRect(0.f, 0.f, 280.f, 48.f), L"")
{
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(true);
    updateViews(window.getSize().x, window.getSize().y);
}

void Game::updateViews(unsigned int width, unsigned int height)
{
    if (width == 0 || height == 0)
    {
        return;
    }
    // Игровой мир сохраняет размер и пропорции. Меняется только его окно просмотра.
    worldView.reset(sf::FloatRect(0.f, 0.f, Config::WorldWidth, Config::WorldHeight));
    float windowRatio = static_cast<float>(width) / static_cast<float>(height);
    float worldRatio = Config::WorldWidth / Config::WorldHeight;
    sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);
    if (windowRatio > worldRatio)
    {
        viewport.width = worldRatio / windowRatio;
        viewport.left = (1.f - viewport.width) / 2.f;
    }
    else
    {
        viewport.height = windowRatio / worldRatio;
        viewport.top = (1.f - viewport.height) / 2.f;
    }
    worldView.setViewport(viewport);

    // Минимальное логическое место под меню: 560 x 760.
    // На маленьком окне UI уменьшается целиком, на узком — перестраивается в одну колонку.
    float uiScale = std::min(static_cast<float>(width) / 560.f,
                             static_cast<float>(height) / 760.f);
    uiScale = std::min(1.25f, uiScale);
    uiSize = sf::Vector2f(width / uiScale, height / uiScale);
    uiView.reset(sf::FloatRect(0.f, 0.f, uiSize.x, uiSize.y));
    menu.layout(uiSize);
    float buttonWidth = std::min(320.f, uiSize.x * 0.60f);
    primaryButton.setSize(sf::Vector2f(buttonWidth, 48.f));
    secondaryButton.setSize(sf::Vector2f(buttonWidth, 48.f));
    exitButton.setSize(sf::Vector2f(buttonWidth, 48.f));
    float x = (uiSize.x - buttonWidth) / 2.f;
    float y = uiSize.y * 0.57f;
    primaryButton.setPosition(x, y);
    secondaryButton.setPosition(x, y + 62.f);
    exitButton.setPosition(x, y + 124.f);
    primaryButton.cancelInteraction();
    secondaryButton.cancelInteraction();
    exitButton.cancelInteraction();
}

sf::Vector2f Game::getMousePosition(const sf::Event& event) const
{
    sf::Vector2i pixel = sf::Mouse::getPosition(window);
    // Берём координаты события, а не более позднее положение курсора.
    if (event.type == sf::Event::MouseMoved)
    {
        pixel = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
    }
    else if (event.type == sf::Event::MouseButtonPressed ||
             event.type == sf::Event::MouseButtonReleased)
    {
        pixel = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
    }
    else if (event.type == sf::Event::MouseWheelScrolled)
    {
        pixel = sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y);
    }
    return window.mapPixelToCoords(pixel, uiView);
}

void Game::startGame()
{
    world.reset(menu.getEnemyCount());
    state = GameState::Playing;
    accumulator = 0.f;
    jumpQueued = false;
    jumpWasDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
                  sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
    menu.cancelInteraction();
    primaryButton.cancelInteraction();
    secondaryButton.cancelInteraction();
    exitButton.cancelInteraction();
}

void Game::openPause(bool fromStart)
{
    pauseFromStart = fromStart;
    state = GameState::Paused;
    accumulator = 0.f;
    jumpQueued = false;
    menu.open(fromStart);
    primaryButton.cancelInteraction();
    secondaryButton.cancelInteraction();
    exitButton.cancelInteraction();
}

void Game::closePause()
{
    state = GameState::Playing;
    if (pauseFromStart)
    {
        state = GameState::Start;
    }
    accumulator = 0.f;
    jumpQueued = false;
    jumpWasDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
                  sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
    menu.cancelInteraction();
}

void Game::handleMenuAction(MenuAction action)
{
    if (action == MenuAction::Resume) { closePause(); }
    else if (action == MenuAction::Restart) { startGame(); }
    else if (action == MenuAction::Exit) { window.close(); }
}

void Game::processEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
            break;
        }
        if (event.type == sf::Event::Resized)
        {
            updateViews(event.size.width, event.size.height);
        }
        if (event.type == sf::Event::LostFocus)
        {
            if (state == GameState::Playing) { openPause(false); }
            menu.cancelInteraction();
            primaryButton.cancelInteraction();
            secondaryButton.cancelInteraction();
            exitButton.cancelInteraction();
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
        {
            if (state == GameState::Playing) { openPause(false); }
            else if (state == GameState::Paused) { closePause(); }
            else if (state == GameState::GameOver) { state = GameState::Start; }
            else { window.close(); }
            continue;
        }
        if (event.type == sf::Event::KeyPressed &&
            (event.key.code == sf::Keyboard::R || event.key.code == sf::Keyboard::Enter))
        {
            if (state == GameState::GameOver ||
                (state == GameState::Playing && event.key.code == sf::Keyboard::R) ||
                (state == GameState::Start && event.key.code == sf::Keyboard::Enter))
            {
                startGame();
                continue;
            }
        }
        sf::Vector2f mouse = getMousePosition(event);
        if (state == GameState::Paused)
        {
            MenuAction action = menu.handleEvent(event, mouse);
            world.setEnemyCount(menu.getEnemyCount());
            handleMenuAction(action);
        }
        else if (state == GameState::Start || state == GameState::GameOver)
        {
            primaryButton.handleEvent(event, mouse);
            secondaryButton.handleEvent(event, mouse);
            exitButton.handleEvent(event, mouse);
            if (primaryButton.consumeClick()) { startGame(); }
            else if (secondaryButton.consumeClick())
            {
                if (state == GameState::Start) { openPause(true); }
                else { state = GameState::Start; }
            }
            else if (exitButton.consumeClick()) { window.close(); }
        }
    }
}

void Game::update(float deltaTime)
{
    menuTime = std::fmod(menuTime + deltaTime, 10000.f);
    if (state != GameState::Playing || !window.hasFocus())
    {
        accumulator = 0.f;
        return;
    }
    int direction = 0;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) { direction--; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) { direction++; }
    bool jumpDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
    if (jumpDown && !jumpWasDown) { jumpQueued = true; }
    jumpWasDown = jumpDown;
    accumulator += deltaTime;
    while (accumulator >= Config::FixedStep)
    {
        world.update(Config::FixedStep, direction, jumpQueued, menu.isBonusRespawnEnabled());
        jumpQueued = false;
        accumulator -= Config::FixedStep;
        if (world.getPlayer().hasDisappeared())
        {
            state = GameState::GameOver;
            accumulator = 0.f;
            break;
        }
    }
}

void Game::drawHud()
{
    sf::RectangleShape background(sf::Vector2f(uiSize.x - 32.f, 88.f));
    background.setPosition(16.f, 16.f);
    background.setFillColor(sf::Color(10, 22, 35, 220));
    window.draw(background);
    sf::String health = L"Здоровье  ";
    sf::String score = L"Очки  ";
    if (!menu.isRussian()) { health = L"Health  "; score = L"Score  "; }
    health += sf::String(std::to_string(world.getPlayer().getHealth())) + L" / ";
    health += sf::String(std::to_string(Config::MaximumHealth));
    score += sf::String(std::to_string(world.getScore()));
    UI::drawLabel(window, resources.font, health, sf::Vector2f(30.f, 25.f), 18);
    UI::drawLabel(window, resources.font, score, sf::Vector2f(uiSize.x - 190.f, 25.f), 22, UI::Accent);
    float barWidth = std::min(320.f, uiSize.x * 0.4f);
    sf::RectangleShape rail(sf::Vector2f(barWidth, 12.f));
    rail.setPosition(30.f, 65.f);
    rail.setFillColor(sf::Color(50, 48, 57));
    window.draw(rail);
    sf::RectangleShape fill(sf::Vector2f(barWidth * world.getPlayer().getHealth() / static_cast<float>(Config::MaximumHealth), 12.f));
    fill.setPosition(rail.getPosition());
    fill.setFillColor(sf::Color(188, 76, 83));
    window.draw(fill);
    sf::String control = L"A/D или ← →   •   ↑ / Пробел: прыжок   •   Esc: меню   •   R: заново";
    if (!menu.isRussian())
    {
        control = L"A/D or ← →   •   ↑ / Space: jump   •   Esc: menu   •   R: restart";
    }
    sf::Text controls(control, resources.font, 15);
    while (controls.getLocalBounds().width > uiSize.x - 32.f && controls.getCharacterSize() > 8)
    {
        controls.setCharacterSize(controls.getCharacterSize() - 1);
    }
    controls.setPosition(16.f, uiSize.y - 26.f);
    controls.setFillColor(UI::Text);
    controls.setOutlineThickness(1.f);
    controls.setOutlineColor(sf::Color(5, 10, 18));
    window.draw(controls);
    float remaining[] = {world.getPlayer().getSpeedRemaining(), world.getPlayer().getJumpRemaining()};
    for (int i = 0; i < 2; i++)
    {
        if (remaining[i] <= 0.f) { continue; }
        sf::String label = L"Скорость: ";
        if (i == 1) { label = L"Высокий прыжок: "; }
        if (!menu.isRussian())
        {
            label = L"Speed: ";
            if (i == 1) { label = L"High jump: "; }
        }
        label += sf::String(std::to_string(static_cast<int>(std::ceil(remaining[i]))));
        if (menu.isRussian()) { label += L" с"; }
        else { label += L" s"; }
        UI::drawLabel(window, resources.font, label, sf::Vector2f(30.f, 113.f + i * 28.f), 18, UI::Accent);
    }
}

void Game::drawStart()
{
    drawScenery(window, resources, uiSize, menuTime);
    sf::RectangleShape shade(uiSize);
    shade.setFillColor(sf::Color(4, 12, 23, 85));
    window.draw(shade);
    sf::String title = L"ДОЗОР";
    sf::String subtitle = L"КРЫЛЬЯ НАД СТЕНОЙ";
    sf::String description = L"Камень. Снег. Тень дракона.";
    primaryButton.setLabel(L"Начать дозор");
    secondaryButton.setLabel(L"Настройки и правила");
    exitButton.setLabel(L"Выйти");
    if (!menu.isRussian())
    {
        title = L"THE WATCH";
        subtitle = L"WINGS OVER THE WALL";
        description = L"Stone. Snow. A dragon's shadow.";
        primaryButton.setLabel(L"Begin the watch");
        secondaryButton.setLabel(L"Settings and rules");
        exitButton.setLabel(L"Exit");
    }
    unsigned int titleSize = static_cast<unsigned int>(std::min(86.f, uiSize.x / 9.f));
    drawCenteredLabel(window, resources.font, title, uiSize.x / 2.f, uiSize.y * 0.24f, titleSize, UI::Text);
    drawCenteredLabel(window, resources.font, subtitle, uiSize.x / 2.f, uiSize.y * 0.24f + titleSize + 20.f, 22, UI::Accent);
    drawCenteredLabel(window, resources.font, description, uiSize.x / 2.f, uiSize.y * 0.46f, 18, UI::Muted);
    window.draw(primaryButton);
    window.draw(secondaryButton);
    window.draw(exitButton);
    sf::String footnote = L"Учебный платформер по мотивам «Песни льда и пламени»";
    if (!menu.isRussian()) { footnote = L"A student platformer inspired by A Song of Ice and Fire"; }
    drawCenteredLabel(window, resources.font, footnote, uiSize.x / 2.f, uiSize.y - 42.f, 14, UI::Muted);
}

void Game::drawGameOver()
{
    sf::RectangleShape shade(uiSize);
    shade.setFillColor(sf::Color(5, 13, 24, 225));
    window.draw(shade);
    sf::String title = L"Дозор окончен";
    sf::String message = L"Стена помнит каждого.";
    sf::String score = L"Собрано очков: ";
    primaryButton.setLabel(L"Новая попытка");
    secondaryButton.setLabel(L"Главное меню");
    exitButton.setLabel(L"Выйти");
    if (!menu.isRussian())
    {
        title = L"Your watch has ended";
        message = L"The Wall remembers.";
        score = L"Points collected: ";
        primaryButton.setLabel(L"Try again");
        secondaryButton.setLabel(L"Main menu");
        exitButton.setLabel(L"Exit");
    }
    score += sf::String(std::to_string(world.getScore()));
    drawDragon(window, resources.dragon, sf::Vector2f(uiSize.x / 2.f, uiSize.y * 0.19f), 0.9f, menuTime);
    drawCenteredLabel(window, resources.font, title, uiSize.x / 2.f, uiSize.y * 0.31f, 38, UI::Text);
    drawCenteredLabel(window, resources.font, message, uiSize.x / 2.f, uiSize.y * 0.40f, 20, UI::Muted);
    drawCenteredLabel(window, resources.font, score, uiSize.x / 2.f, uiSize.y * 0.47f, 20, UI::Accent);
    window.draw(primaryButton);
    window.draw(secondaryButton);
    window.draw(exitButton);
}

void Game::draw()
{
    window.clear(sf::Color(6, 13, 22));
    if (state == GameState::Start || (state == GameState::Paused && pauseFromStart))
    {
        window.setView(uiView);
        drawStart();
    }
    else
    {
        window.setView(worldView);
        drawScenery(window, resources, sf::Vector2f(Config::WorldWidth, Config::WorldHeight), world.getElapsedTime());
        window.draw(world);
        if (menu.isNameVisible()) { world.drawName(window, menu.getPlayerName(), menu.getNameColor()); }
        window.setView(uiView);
        drawHud();
    }
    if (state == GameState::Paused)
    {
        sf::RectangleShape shade(uiSize);
        shade.setFillColor(sf::Color(2, 8, 15, 170));
        window.draw(shade);
        window.draw(menu);
        const sf::FloatRect& panel = menu.getPanelBounds();
        drawDragon(window, resources.dragon,
            sf::Vector2f(panel.left + panel.width - 74.f, panel.top + 42.f), 0.48f, menuTime);
    }
    else if (state == GameState::GameOver)
    {
        drawGameOver();
    }
    window.display();
}

void Game::run()
{
    sf::Clock clock;
    while (window.isOpen())
    {
        float deltaTime = std::min(clock.restart().asSeconds(), 0.1f);
        processEvents();
        if (!window.isOpen()) { break; }
        update(deltaTime);
        draw();
    }
}
