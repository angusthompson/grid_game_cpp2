#include "Fertility.hpp"
#include <iostream>
#include <random>



FertilityMap::FertilityMap(int rows, int cols) : rows(rows), cols(cols) {
    fertilityGrid.resize(rows, std::vector<float>(cols, 0.0f));
}


void FertilityMap::generateFromTerrain(const std::vector<std::vector<int>>& terrainMap) {
    std::random_device rd;
    std::mt19937 gen(rd());

    // Step 1: Populate fertilityGrid with randomized fertility
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int terrainType = terrainMap[r][c];
            float baseFertility = 0.0f;

            switch (terrainType) {
                case 0:  baseFertility = 3.0f; break;  // Sea
                case 1:  baseFertility = 5.0f; break;  // Land
                case 2:  baseFertility = 4.0f; break;  // Hills
                case 3:  baseFertility = 0.3f; break;  // Mountain
                case 5:  baseFertility = 5.0f; break;  // River source
                case 6:  baseFertility = 5.0f; break;  // River
                case 7:  baseFertility = 0.0f; break;  // Ice
                case 8:  baseFertility = 0.5f; break;  // Tundra
                case 9:  baseFertility = 0.0f; break;  // Tundra hills
                case 10: baseFertility = 1.0f; break;  // Taiga
                case 11: baseFertility = 0.5f; break;  // Taiga hills
                case 12: baseFertility = 0.0f; break;  // Desert
                case 13: baseFertility = 0.0f; break;  // Desert hills
                case 14: baseFertility = 5.0f; break;  // River source
                case 15: baseFertility = 0.0f; break;  // Icecap
                case 16: baseFertility = 5.0f; break;  // Lake
                case 17: baseFertility = 8.0f; break;  // Floodplain
                case 18: baseFertility = 5.0f; break;  // Forest
                case 19: baseFertility = 4.0f; break;  // Forest hills
                case 20: baseFertility = 2.0f; break;  // Jungle
                case 21: baseFertility = 2.0f; break;  // Jungle hills
                case 22: baseFertility = 5.0f; break;  // Coast
                case 23: baseFertility = 1.0f; break;  // Ocean
                default: baseFertility = 1.0f; break;  // Fallback default
            }

            // Define variation range
            float variation = 0.2f + 0.1f * baseFertility;
            std::uniform_real_distribution<float> dist(-variation, variation);
            float randomFertility = std::clamp(baseFertility + dist(gen), 0.0f, 10.0f);

            fertilityGrid[r][c] = randomFertility;
        }
    }

    // Step 2: Smooth fertility using a 3x3 box blur
    std::vector<std::vector<float>> smoothed(rows, std::vector<float>(cols, 0.0f));

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float sum = 0.0f;
            int count = 0;

            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    int nr = r + dr;
                    int nc = c + dc;

                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        sum += fertilityGrid[nr][nc];
                        count++;
                    }
                }
            }

            smoothed[r][c] = sum / count;
        }
    }

    fertilityGrid = smoothed;
}





const std::vector<std::vector<float>>& FertilityMap::getFertilityGrid() const {
    return fertilityGrid;
}


sf::VertexArray FertilityMap::createFertilityOverlay(float cellSize) const {
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);
    vertices.resize(rows * cols * 6);

    // We'll map fertility (0.0 to 1.0) to color from brown (low) to green (high)
    auto fertilityToColor = [](float fert) -> sf::Color {
        // clamp fert to [0,10]
        fert = std::clamp(fert, 0.0f, 5.0f);
        // Interpolate between brown (128, 64, 0) and green (0, 255, 0)
        int r = static_cast<int>(128 * (1.0f - fert));
        int g = static_cast<int>(64 + (255 - 64) * fert);
        int b = 0;
        return sf::Color(r, g, b, 100);  // semi-transparent alpha
    };

    // If fertility grid stores ints (0-100), normalize to 0-1 first:
    // float fertNorm = fertilityValue / 100.0f;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float x = col * cellSize;
            float y = row * cellSize;

            float fertValue = fertilityGrid[row][col]; // your fertility value here, float 0-1 or int normalized
            float fertNorm = fertValue / 1.0f;       // normalize if fertilityGrid is 0-100

            sf::Color color = fertilityToColor(fertNorm);

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
