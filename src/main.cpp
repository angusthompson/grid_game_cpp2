#include "mechanics/MapGenerator.hpp"
#include "mechanics/Fertility.hpp"
#include "mechanics/FoW.hpp"
#include "mechanics/Tribe.hpp"
#include "Tools/UITools.hpp"
#include "Tools/MapTools.hpp"
#include "Tools/ObjectTools.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <cmath>

int main() {
    sf::Clock buttonCooldownClock;
    const float buttonCooldown = 0.1f;

    sf::Font font;
    if (!font.openFromFile("../resources/AovelSansRounded-rdDL.ttf")) {
        std::cerr << "Failed to load font\n";
        return -1;
    }

    sf::RenderWindow window(sf::VideoMode({1800, 900}), "Grid Game!");
    window.setVerticalSyncEnabled(true);

    const int rows = 150;
    const int cols = 250;
    float dimension = 2000.0f / cols;
    const float cellSize = dimension;
    bool MenuOpen = false;

    // --- Map and overlays ---
    MapGenerator mapGenerator(rows, cols);
    mapGenerator.generateMap();
    const std::vector<std::vector<int>>& map = mapGenerator.getMap();
    sf::VertexArray grid = mapGenerator.createGrid(cellSize);

    FertilityMap fertility(rows, cols);
    fertility.generateFromTerrain(map);
    sf::VertexArray fertilityOverlay = fertility.createFertilityOverlay(cellSize);

    FogOfWarMap fog(rows, cols);

    Tribe playerTribe(rows, cols);
    playerTribe.spawn(map); // Pick a random land tile
    playerTribe.revealFoW(fog.getFogGrid()); // Reveal fog within radius

    sf::VertexArray fogOverlay = fog.createFogOverlay(cellSize);
    sf::RectangleShape playerMarker = playerTribe.getPlayerMarker(cellSize);

    // Center the view on the player
    float playerX = playerTribe.getCol() * cellSize + cellSize / 2.f;
    float playerY = playerTribe.getRow() * cellSize + cellSize / 2.f;




    bool showFog = true;
    bool showFertility = false;

    // --- Camera setup ---
    sf::View view = window.getView();
    sf::Vector2f targetCenter = view.getCenter();
    // float targetZoom = 1.0f;
    const float panSpeed = 500.0f;
    const float zoomSpeed = 2.0f;
    const float smoothingFactor = 0.5f;

    targetCenter = {playerX, playerY};

    // Zoom in
    float targetZoom = 0.1f;

    std::unordered_map<sf::Keyboard::Scancode, bool> keyStates = {
        {sf::Keyboard::Scancode::A, false},
        {sf::Keyboard::Scancode::D, false},
        {sf::Keyboard::Scancode::W, false},
        {sf::Keyboard::Scancode::S, false},
        {sf::Keyboard::Scancode::N, false},
        {sf::Keyboard::Scancode::M, false}
    };

    sf::Clock clock;
    UITools UITools(font);

    // --- UI buttons ---
    UIButton fertilityToggleButton({-700.f, 300.f}, {200.f, 50.f}, font, "Toggle Fertility", false, 50);
    UIButton fogToggleButton({-700.f, 370.f}, {200.f, 50.f}, font, "Toggle Fog", false, 50);

    // Calculate world position above the tribe tile
    sf::Vector2f tribePos(
        playerTribe.getCol() * cellSize + cellSize / 2.f - 50.f,
        playerTribe.getRow() * cellSize - cellSize / 2.f - 25.f // one cell above
    );

    UIButton tribeButton(tribePos, {100.f, 20.f}, font, "Tribe Action", true, 30);

    bool tribeMenuOpen = false;

    sf::RectangleShape tribeMenuShape;
    // Position the menu near the tribe button (e.g. right below)
    sf::Vector2f tribeMenuPos = tribePos + sf::Vector2f(2.f, -155.f);


    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            // if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            //     auto [row, col] = windowToTile(window, cellSize);
            //     std::cout << "Clicked tile at (" << row << ", " << col << ")\n";
            // }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    window.close();
                } else {
                    keyStates[keyPressed->scancode] = true;
                }
            } else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                keyStates[keyReleased->scancode] = false;
            }
        }

        float deltaTime = clock.restart().asSeconds();

        if (keyStates[sf::Keyboard::Scancode::A] && !MenuOpen) targetCenter.x -= panSpeed * deltaTime;
        if (keyStates[sf::Keyboard::Scancode::D] && !MenuOpen) targetCenter.x += panSpeed * deltaTime;
        if (keyStates[sf::Keyboard::Scancode::W] && !MenuOpen) targetCenter.y -= panSpeed * deltaTime;
        if (keyStates[sf::Keyboard::Scancode::S] && !MenuOpen) targetCenter.y += panSpeed * deltaTime;
        if (keyStates[sf::Keyboard::Scancode::N] && !MenuOpen) targetZoom -= zoomSpeed * deltaTime;
        if (keyStates[sf::Keyboard::Scancode::M] && !MenuOpen) targetZoom += zoomSpeed * deltaTime;

        // Smooth camera movement
        sf::Vector2f interpolatedCentre = view.getCenter() + smoothingFactor * (targetCenter - view.getCenter());
        view.setCenter(interpolatedCentre);

        float currentZoom = view.getSize().x / window.getView().getSize().x;
        float interpolatedZoom = currentZoom + smoothingFactor * (targetZoom - currentZoom);
        view.setSize(window.getDefaultView().getSize() * interpolatedZoom);

        window.setView(view);
        window.clear();

        window.draw(grid);

        if (showFertility) {
            window.draw(fertilityOverlay);
        }

        window.draw(getHoveredTileHighlight(window, view, cellSize));

        window.draw(playerMarker); // <- draw tribe marker

        if (showFog) {
            fogOverlay = fog.createFogOverlay(cellSize); // regenerate
            window.draw(fogOverlay);
        }



        // --- Handle UI ---
        UITools.ButtonTransform(fertilityToggleButton, view, window);
        if (UITools.drawButton(window, "Toggle Fertility", fertilityToggleButton)) {
            if (buttonCooldownClock.getElapsedTime().asSeconds() > buttonCooldown) {
                showFertility = !showFertility;
                buttonCooldownClock.restart();
            }
        }

        UITools.ButtonTransform(fogToggleButton, view, window);
        if (UITools.drawButton(window, "Toggle Fog", fogToggleButton)) {
            if (buttonCooldownClock.getElapsedTime().asSeconds() > buttonCooldown) {
                showFog = !showFog;
                buttonCooldownClock.restart();
            }
        }


        UITools.ButtonTransform(tribeButton, view, window);
        if (UITools.drawButton(window, "Tribe Action", tribeButton)) {
            if (buttonCooldownClock.getElapsedTime().asSeconds() > buttonCooldown) {
                tribeMenuOpen = !tribeMenuOpen;
                buttonCooldownClock.restart();
            }
        }

        const float buttonCooldown2 = 2.f;

        if (tribeMenuOpen) {
            if (buttonCooldownClock.getElapsedTime().asSeconds() > buttonCooldown) {
                bool moveClicked = playerTribe.drawTribeMenu(UITools, window, view, tribeMenuPos, font, cellSize, {playerX, playerY});
            }
        }

        sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        playerTribe.drawMoveHighlights(window);

        // sf::RectangleShape tribeSprite = placeObjectAt(playerTribe.getCol(), playerTribe.getRow(), cellSize, sf::Color::Red);
        // drawObject(window, tribeSprite);


        window.display();
    }

    return 0;
}
