#pragma once

#include <SFML/Graphics.hpp>
#include <string>


struct UIButton {
    sf::RectangleShape shape;
    sf::Text text;
    sf::Vector2f initialPosition;
    sf::Vector2f initialSize;
    sf::Vector2f initialViewCenter;
    bool transformable;
    unsigned int baseFontSize;

    UIButton(const sf::Vector2f& pos, const sf::Vector2f& size,
             const sf::Font& font, const std::string& label,
             bool transform = true, unsigned int fontSize=10)
        : shape(),
          text(font, label, fontSize), // ✅ use constructor param
          initialPosition(pos),
          initialSize(size),
          transformable(transform),
          baseFontSize(fontSize)       // ✅ assign it here
    {
        shape.setPosition(pos);
        shape.setSize(size);
        shape.setFillColor(sf::Color(200, 200, 200));
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Black);

        text.setFillColor(sf::Color::Black);
    }
};




class UITools {
public:
    UITools(sf::Font& font);

    // Draws a button. Returns true if clicked this frame.
    bool drawButton(sf::RenderWindow& window, const std::string& text, UIButton& button);
    void ButtonTransform(UIButton& button, const sf::View& view, const sf::RenderWindow& window);
    void drawMenuElement(sf::RenderWindow& window, UIButton& button);        
private:
    sf::Font& font;
};
