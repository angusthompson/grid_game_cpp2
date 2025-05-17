#include "FoW.hpp"

FogOfWarMap::FogOfWarMap(int rows, int cols) : rows(rows), cols(cols) {
    fogGrid.resize(rows, std::vector<int>(cols, 0));
}

void FogOfWarMap::resetFog() {
    for (auto& row : fogGrid)
        std::fill(row.begin(), row.end(), 0);
}

void FogOfWarMap::reveal(int row, int col) {
    if (row >= 0 && row < rows && col >= 0 && col < cols)
        fogGrid[row][col] = 2;
}

void FogOfWarMap::markSeen() {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (fogGrid[r][c] == 2)
                fogGrid[r][c] = 1;
        }
    }
}

std::vector<std::vector<int>>& FogOfWarMap::getFogGrid() {
    return fogGrid;
}

const std::vector<std::vector<int>>& FogOfWarMap::getFogGrid() const {
    return fogGrid;
}



sf::VertexArray FogOfWarMap::createFogOverlay(float cellSize) const {
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);
    vertices.resize(rows * cols * 6);

    auto fogToColor = [](int fogVal) -> sf::Color {
        switch (fogVal) {
            case 0: return sf::Color(0, 0, 0, 255);   // Black (full fog)
            case 1: return sf::Color(100, 100, 100, 150); // Greyed out
            case 2: return sf::Color(0, 0, 0, 0);     // Transparent (visible)
            default: return sf::Color(255, 0, 255, 255); // Debug magenta
        }
    };

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float x = col * cellSize;
            float y = row * cellSize;

            sf::Color color = fogToColor(fogGrid[row][col]);
            int i = (row * cols + col) * 6;

            vertices[i + 0].position = sf::Vector2f(x, y);
            vertices[i + 0].color = color;

            vertices[i + 1].position = sf::Vector2f(x + cellSize, y);
            vertices[i + 1].color = color;

            vertices[i + 2].position = sf::Vector2f(x, y + cellSize);
            vertices[i + 2].color = color;

            vertices[i + 3].position = sf::Vector2f(x, y + cellSize);
            vertices[i + 3].color = color;

            vertices[i + 4].position = sf::Vector2f(x + cellSize, y);
            vertices[i + 4].color = color;

            vertices[i + 5].position = sf::Vector2f(x + cellSize, y + cellSize);
            vertices[i + 5].color = color;
        }
    }

    return vertices;
}
