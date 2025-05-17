#include "MapTools.hpp"
#include <vector>
#include <utility>
#include <cmath>

std::pair<int, int> windowToTile(const sf::RenderWindow& window, float cellSize) {
    // Convert from window pixel coords to world coords
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

    int col = static_cast<int>(worldPos.x / cellSize);
    int row = static_cast<int>(worldPos.y / cellSize);

    return {row, col};
}


sf::Vector2f tileToWorld(int row, int col, float cellSize) {
    float x = col * cellSize;
    float y = row * cellSize;
    return sf::Vector2f(x, y);
}


#include <SFML/Graphics.hpp>

sf::RectangleShape getHoveredTileHighlight(sf::RenderWindow& window, const sf::View& view, float cellSize) {
    // Convert mouse position to world coordinates
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, view);

    // Convert world position to tile indices
    int row = static_cast<int>(worldPos.y / cellSize);
    int col = static_cast<int>(worldPos.x / cellSize);

    // Convert back to tile top-left position
    float x = col * cellSize;
    float y = row * cellSize;

    // Create highlight shape
    sf::RectangleShape highlight(sf::Vector2f(cellSize, cellSize));
    highlight.setPosition({x, y});
    highlight.setFillColor(sf::Color(255, 255, 0, 80)); // semi-transparent yellow

    // Outline config
    highlight.setOutlineColor(sf::Color::White);
    highlight.setOutlineThickness(1.f);

    return highlight;
}



std::vector<std::pair<int, int>> getTilesInRadius(int centerRow, int centerCol, int radius) {
    std::vector<std::pair<int, int>> result;

    for (int dr = -radius; dr <= radius; ++dr) {
        for (int dc = -radius; dc <= radius; ++dc) {
            int r = centerRow + dr;
            int c = centerCol + dc;

            if (dr * dr + dc * dc <= radius * radius) {
                result.emplace_back(r, c);
            }
        }
    }

    return result;
}


sf::RectangleShape highlightTileAt(int row, int col, float cellSize, sf::Color color) {
    sf::RectangleShape highlight(sf::Vector2f(cellSize, cellSize));
    highlight.setPosition({col * cellSize, row * cellSize});
    highlight.setFillColor(color);
    return highlight;
}
