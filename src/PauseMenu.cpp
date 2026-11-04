#include "PauseMenu.h"
#include <algorithm>

static void placeWidget(Widget& widget, float x, float y, float width, float height)
{
    widget.setPosition(x, y);
    widget.setSize(sf::Vector2f(width, height));
}

PauseMenu::PauseMenu(const sf::Font& font)
    : font(font),
      nameField(font, sf::FloatRect(0.f, 0.f, 400.f, 42.f)),
      showName(font, sf::FloatRect(0.f, 0.f, 400.f, 34.f), L"", true),
      respawnBonuses(font, sf::FloatRect(0.f, 0.f, 400.f, 34.f), L"", true),
      language(font, sf::FloatRect(0.f, 0.f, 400.f, 40.f), {L"Русский", L"English"}),
      enemyCount(font, sf::FloatRect(0.f, 0.f, 400.f, 40.f), {L"0", L"1", L"2", L"3", L"4", L"5"}, 3),
      nameColor(font, sf::FloatRect(0.f, 0.f, 400.f, 40.f), {L"Снег", L"Золото", L"Лёд", L"Роза"}),
      welcome(font, sf::FloatRect(0.f, 0.f, 400.f, 300.f), L""),
      resume(font, sf::FloatRect(0.f, 0.f, 160.f, 42.f), L""),
      restart(font, sf::FloatRect(0.f, 0.f, 160.f, 42.f), L""),
      exit(font, sf::FloatRect(0.f, 0.f, 160.f, 42.f), L"")
{
    // Адреса объектов, которыми владеет это меню. new/delete не нужны.
    widgets = {&nameField, &showName, &respawnBonuses, &language, &enemyCount,
               &nameColor, &welcome, &resume, &restart, &exit};
    translate();
}

bool PauseMenu::isRussian() const { return currentLanguage == 0; }
bool PauseMenu::isNameVisible() const { return showName.isChecked(); }
bool PauseMenu::isBonusRespawnEnabled() const { return respawnBonuses.isChecked(); }
int PauseMenu::getEnemyCount() const { return enemyCount.getSelectedIndex(); }
const sf::String& PauseMenu::getPlayerName() const { return nameField.getValue(); }
const sf::FloatRect& PauseMenu::getPanelBounds() const { return panel; }

sf::Color PauseMenu::getNameColor() const
{
    const sf::Color colors[] = {sf::Color(240, 243, 243), sf::Color(242, 197, 112),
                                sf::Color(121, 211, 249), sf::Color(245, 153, 182)};
    return colors[nameColor.getSelectedIndex()];
}

void PauseMenu::translate()
{
    if (isRussian())
    {
        showName.setLabel(L"Показывать имя над героем");
        respawnBonuses.setLabel(L"Восстанавливать собранные бонусы");
        resume.setLabel(L"Продолжить");
        restart.setLabel(L"Заново");
        exit.setLabel(L"Выйти");
        if (openedFromStart)
        {
            resume.setLabel(L"Назад");
            restart.setLabel(L"Начать игру");
        }
        nameColor.setOptions({L"Снег", L"Золото", L"Лёд", L"Роза"});
        welcome.setText(
            L"НОЧЬ ТЕМНА. ДОЗОР НЕ СПИТ.\n\n"
            L"Ты — молодой брат Ночного дозора у Стены. Ветер сдувает снег "
            L"с камней, вдали летит дракон, а на стенах бродят ледяные мертвецы. "
            L"Собирай золотые знаки, поднимайся на уступы и береги здоровье.\n\n"
            L"УПРАВЛЕНИЕ\n"
            L"A / D или стрелки влево и вправо — движение. "
            L"Пробел или стрелка вверх — прыжок. Для следующего прыжка отпусти кнопку. "
            L"Esc — пауза. R — новая попытка во время игры.\n\n"
            L"ТРИ ВИДА БОНУСОВ\n"
            L"Золотой знак: +10 очков. Голубая молния: скорость выше в 1,6 раза "
            L"на 10 секунд. Фиолетовое перо: начальная скорость прыжка выше в 1,3 раза "
            L"на 10 секунд. Повторный подбор обновляет таймер, но не складывает усиления.\n\n"
            L"Бонусы возвращаются через 7 секунд игрового времени. "
            L"Выключенный флажок останавливает их восстановление; включение продолжает отсчёт.\n\n"
            L"ЗДОРОВЬЕ И ПАДЕНИЕ\n"
            L"Касание врага снимает 25 из 100 очков здоровья. После удара действует "
            L"защита на одну секунду. Врагов нельзя атаковать: обходи и перепрыгивай их. "
            L"Падение за край также снимает 25 здоровья и возвращает к началу. "
            L"При нуле здоровья герой рассыпается на перья, затем можно начать заново.\n\n"
            L"НАСТРОЙКИ\n"
            L"Имя, его цвет, язык и число врагов применяются сразу. "
            L"Пауза останавливает врагов, бонусы, физику и все игровые таймеры. "
            L"Изменение окна не меняет расстояния в мире или силу прыжка.\n\n"
            L"Это небольшой учебный уровень, а не сюжетная кампания. "
            L"Отдельного экрана победы нет: исследуй платформы и набирай очки.\n\n"
            L"И МОЙ ДОЗОР НАЧИНАЕТСЯ.");
    }
    else
    {
        showName.setLabel(L"Show the name above the hero");
        respawnBonuses.setLabel(L"Respawn collected bonuses");
        resume.setLabel(L"Resume");
        restart.setLabel(L"Restart");
        exit.setLabel(L"Exit");
        if (openedFromStart)
        {
            resume.setLabel(L"Back");
            restart.setLabel(L"Start game");
        }
        nameColor.setOptions({L"Snow", L"Gold", L"Ice", L"Rose"});
        welcome.setText(
            L"THE NIGHT IS DARK. THE WATCH STANDS.\n\n"
            L"You are a young brother of the Night's Watch near the Wall. "
            L"Snow sweeps over stone, a dragon crosses the distant sky and wights patrol the walls. "
            L"Collect golden tokens, reach the ledges and protect your health.\n\n"
            L"CONTROLS\n"
            L"A / D or Left / Right: move. Space or Up: jump. Release the key before jumping again. "
            L"Esc: pause. R: restart while playing.\n\n"
            L"THREE PICKUPS\n"
            L"Gold token: +10 points. Blue lightning: 1.6x speed for 10 seconds. "
            L"Purple feather: 1.3x initial jump speed for 10 seconds. "
            L"Collecting the same effect refreshes its timer without stacking power.\n\n"
            L"Pickups return after 7 seconds of game time. "
            L"Disabling respawn freezes this timer; enabling it resumes the countdown.\n\n"
            L"HEALTH AND FALLING\n"
            L"Enemy contact removes 25 out of 100 health, followed by one second of protection. "
            L"There is no attack button: avoid enemies or jump over them. "
            L"Falling removes 25 health and returns you to the starting platform. "
            L"At zero health the watchman dissolves into feathers. Then you may restart.\n\n"
            L"SETTINGS\n"
            L"Your name, name color, language and enemy count update immediately. "
            L"Pausing freezes physics, enemies, pickups and game timers. "
            L"Resizing the window does not change world distances or jump strength.\n\n"
            L"This is a small practice level, not a campaign. "
            L"There is no victory screen: explore the platforms and collect points.\n\n"
            L"AND NOW MY WATCH BEGINS.");
    }
}

void PauseMenu::open(bool fromStart)
{
    openedFromStart = fromStart;
    cancelInteraction();
    translate();
}

void PauseMenu::cancelInteraction()
{
    for (std::size_t i = 0; i < widgets.size(); i++)
    {
        widgets[i]->cancelInteraction();
    }
}

void PauseMenu::layout(sf::Vector2f availableSize)
{
    bool wide = availableSize.x >= 1000.f;
    float width = std::min(1100.f, availableSize.x - 32.f);
    float height = std::min(950.f, availableSize.y - 32.f);
    if (wide)
    {
        height = std::min(650.f, availableSize.y - 32.f);
    }
    panel = sf::FloatRect((availableSize.x - width) / 2.f,
                          (availableSize.y - height) / 2.f, width, height);
    float left = panel.left + 24.f;
    float top = panel.top;
    float columnWidth = width - 48.f;
    if (wide)
    {
        columnWidth = (width - 72.f) / 2.f;
    }
    placeWidget(nameField, left, top + 106.f, columnWidth, 42.f);
    placeWidget(showName, left, top + 159.f, columnWidth, 34.f);
    placeWidget(respawnBonuses, left, top + 198.f, columnWidth, 34.f);
    placeWidget(language, left, top + 270.f, columnWidth, 40.f);
    placeWidget(enemyCount, left, top + 348.f, columnWidth, 40.f);
    placeWidget(nameColor, left, top + 426.f, columnWidth, 40.f);
    if (wide)
    {
        placeWidget(welcome, left + columnWidth + 24.f, top + 106.f,
                    columnWidth, height - 188.f);
    }
    else
    {
        placeWidget(welcome, left, top + 508.f, columnWidth, height - 590.f);
    }
    float buttonWidth = (width - 72.f) / 3.f;
    float buttonTop = top + height - 60.f;
    placeWidget(resume, left, buttonTop, buttonWidth, 40.f);
    placeWidget(restart, left + buttonWidth + 12.f, buttonTop, buttonWidth, 40.f);
    placeWidget(exit, left + (buttonWidth + 12.f) * 2.f, buttonTop, buttonWidth, 40.f);
    cancelInteraction();
}

MenuAction PauseMenu::handleEvent(const sf::Event& event, sf::Vector2f mouse)
{
    for (std::size_t i = 0; i < widgets.size(); i++)
    {
        widgets[i]->handleEvent(event, mouse);
    }
    if (currentLanguage != language.getSelectedIndex())
    {
        currentLanguage = language.getSelectedIndex();
        translate();
    }
    if (resume.consumeClick()) { return MenuAction::Resume; }
    if (restart.consumeClick()) { return MenuAction::Restart; }
    if (exit.consumeClick()) { return MenuAction::Exit; }
    return MenuAction::None;
}

void PauseMenu::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    sf::RectangleShape background(sf::Vector2f(panel.width, panel.height));
    background.setPosition(panel.left, panel.top);
    background.setFillColor(UI::Panel);
    background.setOutlineThickness(-1.f);
    background.setOutlineColor(UI::Border);
    target.draw(background, states);
    sf::String title = L"Ночной привал";
    sf::String subtitle = L"Игровое время остановлено";
    if (openedFromStart)
    {
        title = L"Перед дозором";
        subtitle = L"Настрой героя и прочитай правила";
    }
    if (!isRussian())
    {
        title = L"Night's rest";
        subtitle = L"Game time is paused";
        if (openedFromStart)
        {
            title = L"Before the watch";
            subtitle = L"Choose your settings and read the rules";
        }
    }
    UI::drawLabel(target, font, title, sf::Vector2f(panel.left + 24.f, panel.top + 16.f), 28, UI::Accent);
    UI::drawLabel(target, font, subtitle, sf::Vector2f(panel.left + 24.f, panel.top + 55.f), 15, UI::Muted);
    const Widget* fields[] = {&nameField, &language, &enemyCount, &nameColor, &welcome};
    const wchar_t* russian[] = {L"Имя героя", L"Язык", L"Противники", L"Цвет имени", L"Письмо новобранцу"};
    const wchar_t* english[] = {L"Hero name", L"Language", L"Enemies", L"Name color", L"Letter to a recruit"};
    for (int i = 0; i < 5; i++)
    {
        sf::String label = russian[i];
        if (!isRussian()) { label = english[i]; }
        sf::Vector2f position = fields[i]->getPosition();
        position.y -= 27.f;
        UI::drawLabel(target, font, label, position, 17, UI::Muted);
    }
    for (std::size_t i = 0; i < widgets.size(); i++)
    {
        target.draw(*widgets[i], states);
    }
}
