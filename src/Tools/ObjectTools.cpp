#include "ObjectTools.hpp"
#include "UITools.hpp"

#include <iostream>



sf::RectangleShape placeObjectAt(int row, int col, float cellSize, sf::Color color) {
    sf::RectangleShape obj(sf::Vector2f(cellSize, cellSize));
    obj.setPosition({col * cellSize, row * cellSize});
    obj.setFillColor(color);
    return obj;
}

void drawObject(sf::RenderWindow& window, const sf::RectangleShape& objectShape) {
    window.draw(objectShape);
}

void moveObjectTo(sf::RectangleShape& object, int newRow, int newCol, float cellSize) {
    object.setPosition({newCol * cellSize, newRow * cellSize});
}

