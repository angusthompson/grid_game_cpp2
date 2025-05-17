#pragma once

#include "MapGenerator.hpp"
#include <vector>

class FertilityMap {
public:
    FertilityMap(int rows, int cols);

    void generateFromTerrain(const std::vector<std::vector<int>>& terrainMap);
    const std::vector<std::vector<float>>& getFertilityGrid() const;
    sf::VertexArray createFertilityOverlay(float cellSize) const;

private:
    int rows, cols;
    std::vector<std::vector<float>> fertilityGrid;
};
