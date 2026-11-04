#include "Resources.h"
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

Resources::Resources()
{
    load();
}

void Resources::load()
{
    std::vector<std::string> fontPaths;
    fontPaths.push_back("assets/font.ttf");
    fontPaths.push_back("DejaVuSans.ttf");
    const char* windowsDirectory = std::getenv("WINDIR");
    if (windowsDirectory != nullptr)
    {
        fontPaths.push_back(std::string(windowsDirectory) + "/Fonts/segoeui.ttf");
        fontPaths.push_back(std::string(windowsDirectory) + "/Fonts/arial.ttf");
    }
    fontPaths.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    bool fontLoaded = false;
    for (std::size_t i = 0; i < fontPaths.size(); i++)
    {
        std::ifstream file(fontPaths[i], std::ios::binary);
        if (file.good() && font.loadFromFile(fontPaths[i]))
        {
            fontLoaded = true;
            break;
        }
    }
    if (!fontLoaded)
    {
        throw std::runtime_error("No font found. Put a Cyrillic TrueType font in assets/font.ttf.");
    }
    if (!background.loadFromFile("assets/background.png") ||
        !player.loadFromFile("assets/watchman.png") ||
        !enemy.loadFromFile("assets/wight.png") ||
        !dragon.loadFromFile("assets/dragon.png") ||
        !bonuses.loadFromFile("assets/bonuses.png"))
    {
        throw std::runtime_error("Cannot load assets. Run the game from the project folder.");
    }
    background.setSmooth(true);
    player.setSmooth(true);
    enemy.setSmooth(true);
    dragon.setSmooth(true);
    bonuses.setSmooth(true);
}
