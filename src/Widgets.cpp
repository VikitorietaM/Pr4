#include "Widgets.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

void UI::drawLabel(sf::RenderTarget& target, const sf::Font& font,
                   const sf::String& value, sf::Vector2f position,
                   unsigned int size, sf::Color color)
{
    sf::Text text(value, font, size);
    text.setPosition(position);
    text.setFillColor(color);
    target.draw(text);
}

// Вспомогательная функция нужна только этому файлу.
static void drawCenteredText(sf::RenderTarget& target, sf::RenderStates states,
                             const sf::Font& font, const sf::String& label,
                             const sf::FloatRect& area, sf::Color color)
{
    sf::Text text(label, font, 18);
    sf::FloatRect textBounds = text.getLocalBounds();
    // Длинная подпись уменьшается, а не вылезает из своей кнопки.
    while (textBounds.width > area.width - 12.f && text.getCharacterSize() > 10)
    {
        text.setCharacterSize(text.getCharacterSize() - 1);
        textBounds = text.getLocalBounds();
    }
    text.setOrigin(textBounds.left + textBounds.width / 2.f,
                   textBounds.top + textBounds.height / 2.f);
    text.setPosition(area.left + area.width / 2.f, area.top + area.height / 2.f);
    text.setFillColor(color);
    target.draw(text, states);
}

Widget::Widget(const sf::Font& font, const sf::FloatRect& area)
    : font(font), size(area.width, area.height)
{
    setPosition(area.left, area.top);
}

void Widget::setSize(sf::Vector2f newSize)
{
    size.x = std::max(1.f, newSize.x);
    size.y = std::max(1.f, newSize.y);
}

sf::Vector2f Widget::getSize() const
{
    return size;
}

bool Widget::contains(sf::Vector2f worldPoint) const
{
    sf::Vector2f localPoint = getInverseTransform().transformPoint(worldPoint);
    return sf::FloatRect(0.f, 0.f, size.x, size.y).contains(localPoint);
}

void Widget::cancelInteraction()
{
}

void Widget::drawBackground(sf::RenderTarget& target, sf::RenderStates states,
                            sf::Color fill, sf::Color outline) const
{
    sf::RectangleShape rectangle(size);
    rectangle.setFillColor(fill);
    rectangle.setOutlineColor(outline);
    rectangle.setOutlineThickness(-1.f);
    target.draw(rectangle, states);
}

Button::Button(const sf::Font& font, const sf::FloatRect& area,
               const sf::String& label) : Widget(font, area), label(label)
{
}

void Button::setLabel(const sf::String& value)
{
    label = value;
}

void Button::handleEvent(const sf::Event& event, sf::Vector2f mouse)
{
    if (event.type == sf::Event::LostFocus)
    {
        cancelInteraction();
        return;
    }
    hovered = contains(mouse);
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        pressed = hovered;
    }
    if (event.type == sf::Event::MouseButtonReleased &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        if (pressed && hovered)
        {
            clicked = true;
        }
        pressed = false;
    }
}

void Button::cancelInteraction()
{
    hovered = false;
    pressed = false;
    clicked = false;
}

bool Button::consumeClick()
{
    bool result = clicked;
    clicked = false;
    return result;
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    sf::Color fill = UI::Surface;
    if (hovered)
    {
        fill = sf::Color(50, 68, 81);
    }
    if (pressed)
    {
        fill = sf::Color(66, 78, 83);
    }
    drawBackground(target, states, fill, UI::Accent);
    drawCenteredText(target, states, font, label,
                     sf::FloatRect(0.f, 0.f, size.x, size.y), UI::Text);
}

TextField::TextField(const sf::Font& font, const sf::FloatRect& area,
                     const sf::String& initialValue) : Widget(font, area)
{
    for (std::size_t i = 0; i < initialValue.getSize(); i++)
    {
        appendCharacter(initialValue[i]);
    }
}

void TextField::appendCharacter(sf::Uint32 character)
{
    // 20 символов Unicode, а не 20 байт UTF-8. Управляющие коды исключаем.
    if (character >= 32 && character != 127 && character <= 0x10FFFF &&
        !(character >= 0xD800 && character <= 0xDFFF) && value.getSize() < 20)
    {
        value += character;
    }
}

const sf::String& TextField::getValue() const
{
    return value;
}

sf::String TextField::getVisibleValue() const
{
    sf::String visible = value;
    sf::Text measured(visible, font, 20);
    // При сужении поля имя не теряется: показываем помещающийся конец.
    while (!visible.isEmpty() &&
           measured.findCharacterPos(visible.getSize()).x > size.x - 26.f)
    {
        visible.erase(0, 1);
        measured.setString(visible);
    }
    return visible;
}

void TextField::handleEvent(const sf::Event& event, sf::Vector2f mouse)
{
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        focused = contains(mouse);
    }
    if (event.type == sf::Event::LostFocus ||
        (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter))
    {
        focused = false;
    }
    if (!focused || event.type != sf::Event::TextEntered)
    {
        return;
    }
    if (event.text.unicode == 8)
    {
        if (!value.isEmpty())
        {
            value.erase(value.getSize() - 1, 1);
        }
    }
    else
    {
        appendCharacter(event.text.unicode);
    }
}

void TextField::cancelInteraction()
{
    focused = false;
}

void TextField::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    sf::Color outline = UI::Border;
    if (focused)
    {
        outline = UI::Accent;
    }
    drawBackground(target, states, UI::Surface, outline);
    sf::String visible = getVisibleValue();
    sf::Text text(visible, font, 20);
    text.setFillColor(UI::Text);
    text.setPosition(10.f, (size.y - 27.f) / 2.f);
    target.draw(text, states);
    if (focused)
    {
        sf::RectangleShape cursor(sf::Vector2f(2.f, std::max(4.f, size.y - 16.f)));
        cursor.setPosition(text.findCharacterPos(visible.getSize()).x + 2.f, 8.f);
        cursor.setFillColor(UI::Accent);
        target.draw(cursor, states);
    }
}

Checkbox::Checkbox(const sf::Font& font, const sf::FloatRect& area,
                     const sf::String& label, bool checked)
    : Widget(font, area), label(label), checked(checked)
{
}

bool Checkbox::isChecked() const
{
    return checked;
}

void Checkbox::setLabel(const sf::String& value)
{
    label = value;
}

void Checkbox::handleEvent(const sf::Event& event, sf::Vector2f mouse)
{
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left && contains(mouse))
    {
        checked = !checked;
    }
}

void Checkbox::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    sf::RectangleShape box(sf::Vector2f(22.f, 22.f));
    box.setPosition(2.f, (size.y - 22.f) / 2.f);
    box.setFillColor(UI::Surface);
    box.setOutlineColor(UI::Border);
    box.setOutlineThickness(1.f);
    if (checked)
    {
        box.setFillColor(UI::Accent);
    }
    target.draw(box, states);
    if (checked)
    {
        float y = (size.y - 22.f) / 2.f;
        sf::Vertex tick[] = {
            sf::Vertex(sf::Vector2f(6.f, y + 11.f), UI::Panel),
            sf::Vertex(sf::Vector2f(11.f, y + 16.f), UI::Panel),
            sf::Vertex(sf::Vector2f(20.f, y + 6.f), UI::Panel)
        };
        target.draw(tick, 3, sf::LineStrip, states);
    }
    sf::Text text(label, font, 18);
    while (text.getLocalBounds().width > size.x - 36.f && text.getCharacterSize() > 10)
    {
        text.setCharacterSize(text.getCharacterSize() - 1);
    }
    text.setPosition(36.f, (size.y - 26.f) / 2.f);
    text.setFillColor(UI::Text);
    target.draw(text, states);
}

OptionSelector::OptionSelector(const sf::Font& font, const sf::FloatRect& area,
                                const std::vector<sf::String>& options, int selectedIndex)
    : Widget(font, area), selectedIndex(selectedIndex)
{
    setOptions(options);
}

int OptionSelector::getSelectedIndex() const
{
    return selectedIndex;
}

void OptionSelector::setOptions(const std::vector<sf::String>& values)
{
    options = values;
    if (options.empty())
    {
        selectedIndex = -1;
    }
    else if (selectedIndex < 0 || selectedIndex >= static_cast<int>(options.size()))
    {
        selectedIndex = 0;
    }
}

sf::FloatRect OptionSelector::getButtonBounds(std::size_t index) const
{
    float gap = 6.f;
    float count = static_cast<float>(options.size());
    float width = (size.x - gap * (count - 1.f)) / count;
    return sf::FloatRect(index * (width + gap), 0.f, width, size.y);
}

void OptionSelector::handleEvent(const sf::Event& event, sf::Vector2f mouse)
{
    if (event.type != sf::Event::MouseButtonPressed ||
        event.mouseButton.button != sf::Mouse::Left)
    {
        return;
    }
    sf::Vector2f localMouse = getInverseTransform().transformPoint(mouse);
    for (std::size_t i = 0; i < options.size(); i++)
    {
        if (getButtonBounds(i).contains(localMouse))
        {
            selectedIndex = static_cast<int>(i);
            return;
        }
    }
}

void OptionSelector::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    for (std::size_t i = 0; i < options.size(); i++)
    {
        sf::FloatRect area = getButtonBounds(i);
        sf::RectangleShape button(sf::Vector2f(area.width, area.height));
        button.setPosition(area.left, area.top);
        button.setFillColor(UI::Surface);
        button.setOutlineColor(UI::Border);
        button.setOutlineThickness(-1.f);
        sf::Color textColor = UI::Text;
        if (static_cast<int>(i) == selectedIndex)
        {
            button.setFillColor(UI::Accent);
            textColor = UI::Panel;
        }
        target.draw(button, states);
        drawCenteredText(target, states, font, options[i], area, textColor);
    }
}

ScrollableText::ScrollableText(const sf::Font& font, const sf::FloatRect& area,
                                const sf::String& source)
    : Widget(font, area), source(source), text("", font, 18)
{
    text.setFillColor(UI::Text);
    text.setLineSpacing(1.25f);
    setSize(size);
}

void ScrollableText::setSize(sf::Vector2f newSize)
{
    newSize.x = std::max(80.f, newSize.x);
    newSize.y = std::max(60.f, newSize.y);
    if (texture.getSize().x != 0 && newSize == size)
    {
        return;
    }
    Widget::setSize(newSize);
    unsigned int width = static_cast<unsigned int>(size.x - 42.f);
    unsigned int height = static_cast<unsigned int>(size.y - 24.f);
    if (!texture.create(width, height))
    {
        throw std::runtime_error("Cannot create a texture for ScrollableText.");
    }
    texture.setSmooth(true);
    track = sf::FloatRect(size.x - 20.f, 12.f, 8.f, static_cast<float>(height));
    dragging = false;
    wrapText();
    scrollTo(scrollOffset);
}

void ScrollableText::setText(const sf::String& value)
{
    source = value;
    dragging = false;
    scrollOffset = 0.f;
    wrapText();
    scrollTo(0.f);
}

void ScrollableText::wrapText()
{
    // Сохраняем абзацы, переносим по пробелам; очень длинное слово делим.
    sf::String result;
    sf::String line;
    sf::Text measure("", font, text.getCharacterSize());
    float width = static_cast<float>(texture.getSize().x);
    for (std::size_t i = 0; i < source.getSize(); i++)
    {
        sf::Uint32 character = source[i];
        if (character == '\r')
        {
            continue;
        }
        if (character == '\n')
        {
            result += line + L"\n";
            line.clear();
            continue;
        }
        line += character;
        measure.setString(line);
        if (measure.findCharacterPos(line.getSize()).x <= width || line.getSize() < 2)
        {
            continue;
        }
        std::size_t split = sf::String::InvalidPos;
        for (std::size_t j = 1; j < line.getSize(); j++)
        {
            if (line[j] == ' ')
            {
                split = j;
            }
        }
        if (split != sf::String::InvalidPos)
        {
            result += line.substring(0, split) + L"\n";
            line = line.substring(split + 1);
        }
        else
        {
            result += line.substring(0, line.getSize() - 1) + L"\n";
            line = sf::String(character);
        }
    }
    result += line;
    text.setString(result);
    sf::FloatRect local = text.getLocalBounds();
    contentHeight = std::max(local.height,
        text.findCharacterPos(result.getSize()).y - text.getPosition().y + 26.f);
}

float ScrollableText::getMaximumScroll() const
{
    return std::max(0.f, contentHeight - track.height);
}

void ScrollableText::updateTexture()
{
    // Прозрачная текстура обрезает всё, что не помещается в прямоугольник.
    text.setPosition(0.f, -scrollOffset);
    texture.clear(sf::Color::Transparent);
    texture.draw(text);
    texture.display();
}

void ScrollableText::scrollTo(float offset)
{
    scrollOffset = std::max(0.f, std::min(offset, getMaximumScroll()));
    updateTexture();
}

sf::FloatRect ScrollableText::getThumbBounds() const
{
    float height = track.height;
    float top = track.top;
    float maximum = getMaximumScroll();
    if (maximum > 0.f)
    {
        height = std::max(28.f, track.height * track.height / contentHeight);
        height = std::min(height, track.height - 1.f);
        top += (track.height - height) * scrollOffset / maximum;
    }
    return sf::FloatRect(track.left, top, track.width, height);
}

void ScrollableText::dragTo(float mouseY)
{
    float distance = track.height - getThumbBounds().height;
    if (distance > 0.f)
    {
        scrollTo((mouseY - dragOffset - track.top) / distance * getMaximumScroll());
    }
}

void ScrollableText::cancelInteraction()
{
    dragging = false;
}

void ScrollableText::handleEvent(const sf::Event& event, sf::Vector2f mouse)
{
    sf::Vector2f local = getInverseTransform().transformPoint(mouse);
    if (event.type == sf::Event::LostFocus ||
        (event.type == sf::Event::MouseButtonReleased &&
         event.mouseButton.button == sf::Mouse::Left))
    {
        dragging = false;
    }
    if (event.type == sf::Event::MouseWheelScrolled &&
        event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel && contains(mouse))
    {
        scrollTo(scrollOffset - event.mouseWheelScroll.delta * 52.f);
    }
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left && track.contains(local) &&
        getMaximumScroll() > 0.f)
    {
        sf::FloatRect thumb = getThumbBounds();
        dragOffset = thumb.height / 2.f;
        if (thumb.contains(local))
        {
            dragOffset = local.y - thumb.top;
        }
        dragging = true;
        dragTo(local.y);
    }
    if (event.type == sf::Event::MouseMoved && dragging)
    {
        dragTo(local.y);
    }
}

void ScrollableText::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    drawBackground(target, states, UI::Surface, UI::Border);
    sf::Sprite content(texture.getTexture());
    content.setPosition(12.f, 12.f);
    target.draw(content, states);
    sf::RectangleShape rail(sf::Vector2f(track.width, track.height));
    rail.setPosition(track.left, track.top);
    rail.setFillColor(UI::Border);
    target.draw(rail, states);
    sf::FloatRect thumbBounds = getThumbBounds();
    sf::RectangleShape thumb(sf::Vector2f(thumbBounds.width, thumbBounds.height));
    thumb.setPosition(thumbBounds.left, thumbBounds.top);
    thumb.setFillColor(UI::Accent);
    target.draw(thumb, states);
}
