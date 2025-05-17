#include "Tribe.hpp"
#include "../Tools/UITools.hpp"
#include <iostream>
#include <random>

Tribe::Tribe(int rows, int cols) : rows(rows), cols(cols) {}

void Tribe::spawn(const std::vector<std::vector<int>>& terrainMap) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distRow(0, rows - 1);
    std::uniform_int_distribution<> distCol(0, cols - 1);

    while (true) {
        int r = distRow(gen);
        int c = distCol(gen);

        int terrainType = terrainMap[r][c];
        if (terrainType == 1 || terrainType == 2 || terrainType == 18 || terrainType == 20) {
            playerRow = r;
            playerCol = c;
            break;
        }
    }
}

void Tribe::revealFoW(std::vector<std::vector<int>>& fogGrid) const {
    const int radius = 8;
    for (int dr = -radius; dr <= radius; ++dr) {
        for (int dc = -radius; dc <= radius; ++dc) {
            int r = playerRow + dr;
            int c = playerCol + dc;
            if (r >= 0 && r < rows && c >= 0 && c < cols) {
                if (dr * dr + dc * dc <= radius * radius) {
                    fogGrid[r][c] = 2;
                }
            }
        }
    }
}

sf::RectangleShape Tribe::getPlayerMarker(float cellSize) const {
    sf::RectangleShape marker(sf::Vector2f(cellSize, cellSize));
    marker.setPosition(sf::Vector2f(playerCol * cellSize, playerRow * cellSize));
    marker.setFillColor(sf::Color(128, 0, 128, 180));
    return marker;
}

int Tribe::getRow() const { return playerRow; }
int Tribe::getCol() const { return playerCol; }


void Tribe::onMoveClicked(float cellSize, std::pair<int, int> playerPos) {
    if (settleCooldown.getElapsedTime().asSeconds() < cooldownDuration) return;
        settleCooldown.restart();
        moveModeActive = !moveModeActive;

        if (moveModeActive) {
            const int radius = 5;
            moveHighlights.clear();

            for (int dr = -radius; dr <= radius; ++dr) {
                for (int dc = -radius; dc <= radius; ++dc) {
                    int r = playerPos.first + dr;
                    int c = playerPos.second + dc;
                    float r2 = playerPos.first + dr*cellSize - cellSize/2;
                    float c2 = playerPos.second + dc*cellSize - cellSize/2;
                    // std::cout << "r2, c2 (" << r2 << "," << c2 <<")\n";
                    // std::cout << "playerPos (" << playerPos.second <<")\n";

                    // if (r >= 0 && r < rows && c >= 0 && c < cols) {
                        if (dr * dr + dc * dc <= radius * radius) {
                            sf::RectangleShape tile(sf::Vector2f(cellSize, cellSize));
                            // tile.setPosition({c, r});
                            tile.setPosition({r2, c2});

                            tile.setFillColor(sf::Color(255, 255, 255, 180));
                            moveHighlights.push_back(tile);
                        }
                    // }
                }
            }

            std::cout << "Move mode activated.\n";
        } else {
            moveHighlights.clear();
            std::cout << "Move mode deactivated.\n";
        }
}


void Tribe::drawMoveHighlights(sf::RenderWindow& window) const {
    // std::cout << "Drawing move highlights (" << moveHighlights.size() << " tiles)\n";

    if (moveModeActive) {
        for (const auto& tile : moveHighlights) {
            window.draw(tile);
            
        }
    }
}


void Tribe::onSettleClicked() {
    if (settleCooldown.getElapsedTime().asSeconds() < cooldownDuration) return;
    settleCooldown.restart();
    std::cout << "Settle button clicked.\n";
}

void Tribe::onTest1Clicked() {
    if (test1Cooldown.getElapsedTime().asSeconds() < cooldownDuration) return;
    test1Cooldown.restart();
    std::cout << "Test1 button clicked.\n";
}

void Tribe::onTest2Clicked() {
    if (test2Cooldown.getElapsedTime().asSeconds() < cooldownDuration) return;
    test2Cooldown.restart();
    std::cout << "Test2 button clicked.\n";
}

void Tribe::onTest3Clicked() {
    if (test3Cooldown.getElapsedTime().asSeconds() < cooldownDuration) return;
    test3Cooldown.restart();
    std::cout << "Test3 button clicked.\n";
}

// --- Tribe menu rendering + interaction ---
bool Tribe::drawTribeMenu(UITools& UITools, sf::RenderWindow& window, const sf::View& view, sf::Vector2f position, sf::Font& font, float cellSize, std::pair<int, int> playerPos) {
    const sf::Vector2f menuSize = {200.f, 150.f};
    sf::RectangleShape menuShape(menuSize);
    menuShape.setFillColor(sf::Color(50, 50, 50, 200));
    menuShape.setOutlineColor(sf::Color::White);
    menuShape.setOutlineThickness(2.f);
    menuShape.setPosition(position);

    window.draw(menuShape);

    const float buttonWidth = 85.f;
    const float buttonHeight = 15.f;
    const float buttonPadding = 10.f;
    sf::Vector2f currentPos = position + sf::Vector2f(10.f, 10.f);

    std::vector<std::pair<std::string, std::function<void()>>> buttons = {
        {"Move",    [this, cellSize, playerPos]() { onMoveClicked(cellSize, playerPos); }},
        {"Settle",  [this]() { onSettleClicked(); }},
        {"Test1",   [this]() { onTest1Clicked(); }},
        {"Test2",   [this]() { onTest2Clicked(); }},
        {"Test3",   [this]() { onTest3Clicked(); }}
    };

    bool anyClicked = false;

    for (auto& [label, action] : buttons) {
        UIButton button(currentPos, {buttonWidth, buttonHeight}, font, label, true, 20);
        UITools.ButtonTransform(button, view, window);
        bool clicked = UITools.drawButton(window, label, button);
        if (clicked) {
            action(); // Call the associated function
            anyClicked = true;
        }
        currentPos.y += buttonHeight + buttonPadding;
    }

    return anyClicked;
}

void Tribe::moveToTile(int newRow, int newCol) {
    // Bounds check (optional but safe)
    if (newRow < 0 || newRow >= rows || newCol < 0 || newCol >= cols) {
        std::cout << "Invalid move target: (" << newRow << ", " << newCol << ")\n";
        return;
    }

    playerRow = newRow;
    playerCol = newCol;
    moveModeActive = false;
    moveHighlights.clear();

    std::cout << "Tribe moved to tile (" << newRow << ", " << newCol << ")\n";
}
