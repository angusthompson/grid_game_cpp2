#ifndef MAPGENERATOR_HPP
#define MAPGENERATOR_HPP

#include <vector>
#include <SFML/Graphics.hpp> // Include SFML Graphics
#include <cstdlib>
#include <ctime>

class MapGenerator {
public:
    MapGenerator(int rows, int cols);
    void generateMap();
    const std::vector<std::vector<int>>& getMap() const;
    std::vector<std::vector<int>> generateHeightMap();
    // In MapGenerator.hpp
    sf::Color getTileColor(int tileType) const;
    // std::vector<sf::RectangleShape> createGrid(float cellSize);
    sf::Texture createGridTexture(float cellSize);
    sf::Color getColorForTile(int tileType);
    sf::VertexArray createGrid(float cellSize);

private:
    std::vector<std::vector<int>> map;
    std::vector<std::vector<int>> heightMap; // New height map
    std::vector<std::vector<int>> riverMap; // New height map

    // int rows, cols; // Dimensions of the map
    int seaLevel;

    void initializeMap();

    void landBiome(int biomeID);
    void seaBiome(int biomeID);
    void mountainBiome(int biomeID);
    void hillBiome(int biomeID);

    void fillUnassignedWithSea();
    void applyModifiers();
    bool isSurroundedByMountainsOrIce(int row, int col);
    void smoothMap();
    void blendMap();

    void MountainPeaks();
    void ForceTundra();
    void ForceDesert();

    void flowRivers(std::vector<std::vector<int>>& heightMap, std::vector<std::vector<int>>& map);
    void resetHeightMapToZero(std::vector<std::vector<int>>& heightMap, const std::vector<std::vector<int>>& map);
    void changeSmallSeasToRivers(std::vector<std::vector<int>>& map);

    void changeDesertToFloodplains(std::vector<std::vector<int>>& map);
    void applyForestChance(std::vector<std::vector<int>>& map);
    void convertToTaiga(std::vector<std::vector<int>>& map);
    void applyJungleChance(std::vector<std::vector<int>>& map);
    void applyDeepOceanChance(std::vector<std::vector<int>>& map);

    void applyCoastChance(std::vector<std::vector<int>>& map);
    int rows, cols;
    std::vector<int> tiles; // Replace placeholder type with actual tile data type
};

#endif
