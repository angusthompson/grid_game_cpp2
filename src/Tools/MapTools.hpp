#pragma once
#include <SFML/Graphics.hpp>
#include <utility>

sf::RectangleShape highlightTileAt(int row, int col, float cellSize, sf::Color color);

// Convert mouse position in window to grid (row, col) indices
std::pair<int, int> windowToTile(const sf::RenderWindow& window, float cellSize);
// Convert from tile indices (row, col) to top-left world position
sf::Vector2f tileToWorld(int row, int col, float cellSize);
// Returns a rectangle highlighting the tile under the mouse
sf::RectangleShape getHoveredTileHighlight(sf::RenderWindow& window, const sf::View& view, float cellSize);


// Returns a list of (row, col) tile indices within the given radius of the center tile
std::vector<std::pair<int, int>> getTilesInRadius(int centerRow, int centerCol, int radius);
