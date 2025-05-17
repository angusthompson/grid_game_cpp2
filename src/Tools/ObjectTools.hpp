#pragma once
#include <SFML/Graphics.hpp>
#include "UITools.hpp"

sf::RectangleShape placeObjectAt(int row, int col, float cellSize, sf::Color color);

void drawObject(sf::RenderWindow& window, const sf::RectangleShape& objectShape);
void moveObjectTo(sf::RectangleShape& object, int newRow, int newCol, float cellSize);
