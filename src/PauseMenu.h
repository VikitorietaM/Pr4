#pragma once
#include "Widgets.h"

enum class MenuAction { None, Resume, Restart, Exit };

class PauseMenu : public sf::Drawable
{
private:
    const sf::Font& font;
    TextField nameField;
    Checkbox showName;
    Checkbox respawnBonuses;
    OptionSelector language;
    OptionSelector enemyCount;
    OptionSelector nameColor;
    ScrollableText welcome;
    Button resume;
    Button restart;
    Button exit;
    std::vector<Widget*> widgets;
    sf::FloatRect panel;
    bool openedFromStart = false;
    int currentLanguage = 0;
    void translate();
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    explicit PauseMenu(const sf::Font& font);
    void layout(sf::Vector2f availableSize);
    void open(bool fromStart);
    void cancelInteraction();
    MenuAction handleEvent(const sf::Event& event, sf::Vector2f mouse);
    bool isRussian() const;
    bool isNameVisible() const;
    bool isBonusRespawnEnabled() const;
    int getEnemyCount() const;
    sf::Color getNameColor() const;
    const sf::String& getPlayerName() const;
    const sf::FloatRect& getPanelBounds() const;
};
