#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

namespace UI
{
    const sf::Color Panel(16, 28, 42, 245);
    const sf::Color Surface(29, 45, 62);
    const sf::Color Border(72, 95, 112);
    const sf::Color Text(233, 238, 236);
    const sf::Color Muted(161, 184, 196);
    const sf::Color Accent(216, 176, 105);
    void drawLabel(sf::RenderTarget& target, const sf::Font& font,
                   const sf::String& value, sf::Vector2f position,
                   unsigned int size, sf::Color color = Text);
}

class Widget : public sf::Drawable, public sf::Transformable
{
protected:
    const sf::Font& font;
    sf::Vector2f size;
    bool contains(sf::Vector2f worldPoint) const;
    void drawBackground(sf::RenderTarget& target, sf::RenderStates states,
                        sf::Color fill, sf::Color outline) const;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override = 0;
public:
    Widget(const sf::Font& font, const sf::FloatRect& area);
    virtual ~Widget() = default;
    virtual void setSize(sf::Vector2f newSize);
    sf::Vector2f getSize() const;
    virtual void handleEvent(const sf::Event& event, sf::Vector2f mouse) = 0;
    virtual void cancelInteraction();
};

class Button : public Widget
{
private:
    sf::String label;
    bool hovered = false;
    bool pressed = false;
    bool clicked = false;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    Button(const sf::Font& font, const sf::FloatRect& area, const sf::String& label);
    void setLabel(const sf::String& value);
    void handleEvent(const sf::Event& event, sf::Vector2f mouse) override;
    void cancelInteraction() override;
    bool consumeClick();
};

class TextField : public Widget
{
private:
    sf::String value;
    bool focused = false;
    void appendCharacter(sf::Uint32 character);
    sf::String getVisibleValue() const;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    TextField(const sf::Font& font, const sf::FloatRect& area,
              const sf::String& initialValue = L"Дозорный");
    const sf::String& getValue() const;
    void handleEvent(const sf::Event& event, sf::Vector2f mouse) override;
    void cancelInteraction() override;
};

class Checkbox : public Widget
{
private:
    sf::String label;
    bool checked;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    Checkbox(const sf::Font& font, const sf::FloatRect& area,
             const sf::String& label, bool checked = false);
    bool isChecked() const;
    void setLabel(const sf::String& value);
    void handleEvent(const sf::Event& event, sf::Vector2f mouse) override;
};

class OptionSelector : public Widget
{
private:
    std::vector<sf::String> options;
    int selectedIndex;
    sf::FloatRect getButtonBounds(std::size_t index) const;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    OptionSelector(const sf::Font& font, const sf::FloatRect& area,
                   const std::vector<sf::String>& options, int selectedIndex = 0);
    int getSelectedIndex() const;
    void setOptions(const std::vector<sf::String>& values);
    void handleEvent(const sf::Event& event, sf::Vector2f mouse) override;
};

class ScrollableText : public Widget
{
private:
    sf::String source;
    sf::Text text;
    sf::RenderTexture texture;
    sf::FloatRect track;
    float scrollOffset = 0.f;
    float contentHeight = 0.f;
    float dragOffset = 0.f;
    bool dragging = false;
    void wrapText();
    void updateTexture();
    void scrollTo(float offset);
    void dragTo(float mouseY);
    float getMaximumScroll() const;
    sf::FloatRect getThumbBounds() const;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
    ScrollableText(const sf::Font& font, const sf::FloatRect& area,
                   const sf::String& source);
    void setSize(sf::Vector2f newSize) override;
    void setText(const sf::String& value);
    void handleEvent(const sf::Event& event, sf::Vector2f mouse) override;
    void cancelInteraction() override;
};
