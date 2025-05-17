#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

class UITools;

class Tribe {
public:
    Tribe(int rows, int cols);
    void spawn(const std::vector<std::vector<int>>& terrainMap);
    void revealFoW(std::vector<std::vector<int>>& fogGrid) const;
    sf::RectangleShape getPlayerMarker(float cellSize) const;
    int getRow() const;
    int getCol() const;

    bool drawTribeMenu(UITools& UITools, sf::RenderWindow& window, const sf::View& view, sf::Vector2f position, sf::Font& font, float cellSize, std::pair<int, int> playerPos);
    void moveToTile(int newRow, int newCol);

    void drawMoveHighlights(sf::RenderWindow& window) const;
    // std::vector<HighlightTile> moveHighlights;
    // void handleMoveClick(sf::Vector2f mouseWorldPos);
    // float cellSize;

private:
    int playerRow, playerCol;
    int rows, cols;

    // Dummy button handlers
    void onMoveClicked(float cellSize, std::pair<int, int> playerPos);
    void onSettleClicked();
    void onTest1Clicked();
    void onTest2Clicked();
    void onTest3Clicked();

    sf::Clock moveCooldown;
    sf::Clock settleCooldown;
    sf::Clock test1Cooldown;
    sf::Clock test2Cooldown;
    sf::Clock test3Cooldown;
    const float cooldownDuration = 0.3f; // seconds

    bool moveModeActive = false;
    std::vector<sf::RectangleShape> moveHighlights; // Highlight overlay
};
