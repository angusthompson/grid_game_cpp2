#include "UITools.hpp"
#include <iostream>

UITools::UITools(sf::Font& font) : font(font) {}


bool UITools::drawButton(sf::RenderWindow& window, const std::string&, UIButton& button) {
    sf::RectangleShape& buttonShape = button.shape;

    // Hover logic
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePos);
    bool isHovered = buttonShape.getGlobalBounds().contains(mouseWorld);

    buttonShape.setFillColor(isHovered ? sf::Color(170, 170, 170, 180) : sf::Color(200, 200, 200, 180));

    bool isClicked = isHovered && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    window.draw(buttonShape);
    window.draw(button.text);

    return isClicked;
}


void UITools::ButtonTransform(UIButton& button, const sf::View& view, const sf::RenderWindow& window) {
    sf::Vector2u windowSize = window.getSize();
    sf::Vector2f windowSizeF(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y));
    sf::Vector2f viewSize = view.getSize();
    float zoom = windowSizeF.x / viewSize.x;
    sf::Vector2f newCenter = view.getCenter();

    if (!button.transformable) {
        // Transform with zoom and panning
        sf::Vector2f initialPos = button.initialPosition;
        sf::Vector2f initialCent = button.initialViewCenter;

        sf::Vector2f translation = (initialCent - newCenter);
        sf::Vector2f newButtonPosition = initialPos / zoom - translation;

        sf::Vector2f newSize = button.initialSize / zoom;
        button.shape.setSize(newSize);
        button.shape.setPosition(newButtonPosition);
        button.shape.setOutlineThickness(2.f / zoom);

        // Correctly set scale *before* calculating bounds
        float textScale = 1.f / zoom;
        button.text.setScale(sf::Vector2f(0.5f / zoom, 0.5f / zoom));
        button.text.setOrigin({0.f, 0.f}); // reset first

        sf::FloatRect bounds = button.text.getLocalBounds();
        sf::Vector2f origin = bounds.position + bounds.size / 2.f;
        button.text.setOrigin(origin);

        sf::Vector2f buttonCenter = newButtonPosition + newSize / 2.f;
        button.text.setPosition(buttonCenter);

    } else {

        button.shape.setSize(button.initialSize);
        button.shape.setPosition(button.initialPosition);
        button.shape.setOutlineThickness(2.f);

        float textScale = 0.5f;
        button.text.setScale(sf::Vector2f(0.5f, 0.5f));
        button.text.setOrigin({0.f, 0.f});

        sf::FloatRect bounds = button.text.getLocalBounds();
        sf::Vector2f origin = bounds.position + bounds.size / 2.f;
        button.text.setOrigin(origin);

        sf::Vector2f buttonCenter = button.initialPosition + button.initialSize / 2.f;
        button.text.setPosition(buttonCenter);
    }
}

void UITools::drawMenuElement(sf::RenderWindow& window, UIButton& button) {
    sf::RectangleShape& buttonShape = button.shape;

    // Hover logic (still change color on hover for visual feedback)
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePos);
    bool isHovered = buttonShape.getGlobalBounds().contains(mouseWorld);

    buttonShape.setFillColor(isHovered ? sf::Color(170, 170, 170, 180) : sf::Color(200, 200, 200, 180));

    window.draw(buttonShape);
    window.draw(button.text);
}

