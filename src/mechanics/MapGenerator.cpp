#include "MapGenerator.hpp"
#include <cstdlib>
#include <queue>
#include <vector>
#include <utility> // For std::pair
#include <ctime>   // For std::srand, std::rand
#include <algorithm> // For std::shuffle
#include <random>    // For std::default_random_engine
#include <cmath>
#include <ctime>   // For time()
#include <set>
#include <limits> // For std::numeric_limits
#include <iostream>
#include <stack> // Add this include at the top of your file
#include <SFML/Graphics.hpp> // Ensure SFML is included

MapGenerator::MapGenerator(int rows, int cols)
    : rows(rows), cols(cols), map(rows, std::vector<int>(cols, 0)) {
    std::srand(std::time(nullptr)); // Seed the random number generator
}

const std::vector<std::vector<int>>& MapGenerator::getMap() const {
    return map;
}

// MASTER FUNCTION //
void MapGenerator::generateMap() {

    initializeMap();

    fillUnassignedWithSea();
    applyModifiers();

    // Randomisation and smoothing
    blendMap();
    smoothMap();

    ForceTundra();
    // ForceDesert();

    changeSmallSeasToRivers(map);
    MountainPeaks();

    std::vector<std::vector<int>> heightMap = generateHeightMap();

    resetHeightMapToZero(heightMap,map);

    int seaLevel = 0;

    // // Check if the heightMap is valid before printing its size
    // if (!heightMap.empty() && !heightMap[0].empty()) {
    //     std::cout << "HeightMap size: " << heightMap.size() << " x " << heightMap[0].size() << std::endl;
    // } else {
    //     std::cerr << "Error: heightMap is empty or improperly initialized!" << std::endl;
    // }
    // std::cout << "Map size: " << map.size() << " x " << map[0].size() << std::endl;
    // std::cout << "HeightMap size: " << heightMap.size() << " x " << heightMap[0].size() << std::endl;

    flowRivers(heightMap, map);
    changeDesertToFloodplains(map);
    applyForestChance(map);
    convertToTaiga(map);
    applyJungleChance(map);
    applyCoastChance(map);
    applyDeepOceanChance(map);
}


// Calculate the Euclidean distance between two points
float calculateDistance(int row1, int col1, int row2, int col2) {
    return std::sqrt((row1 - row2) * (row1 - row2) + (col1 - col2) * (col1 - col2));
}

void MapGenerator::initializeMap() {
    // Seed the random number generator
    std::srand(std::time(0));

    // Initialize map as unassigned (-1)
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            map[row][col] = -1; // Unassigned
        }
    }

    int numBiomes = 60; // Number of biomes
    std::vector<int> biomeSeeds; // To store starting points of biomes
    std::vector<std::pair<int, int>> biomeSeedPositions; // To store the actual (row, col) positions of biome seeds

    // Step 1: Place initial seeds for biomes
    for (int i = 0; i < numBiomes; ++i) {
        int seedRow = std::rand() % rows;
        int seedCol = std::rand() % cols;
        biomeSeeds.push_back(seedRow * cols + seedCol); // Store seed as a single index
        biomeSeedPositions.push_back({seedRow, seedCol}); // Store seed as (row, col)
        map[seedRow][seedCol] = i; // Mark seed with biome ID
    }

    // Step 2: Grow biomes using randomized flood fill
    for (int biomeID = 0; biomeID < numBiomes; ++biomeID) {
        int maxSize = (rows * cols) / 35; // Approximate size of each biome
        int currentSize = 1;

        std::queue<std::pair<int, int>> biomeQueue; // Queue for flood fill
        int seedRow = biomeSeedPositions[biomeID].first;
        int seedCol = biomeSeedPositions[biomeID].second;
        biomeQueue.push({seedRow, seedCol});

        while (!biomeQueue.empty() && currentSize < maxSize) {
            auto [row, col] = biomeQueue.front();
            biomeQueue.pop();

            // Define 4 cardinal directions (up, down, left, right)
            std::vector<std::pair<int, int>> directions = {
                {row - 1, col}, {row + 1, col}, {row, col - 1}, {row, col + 1}};

            // Use a random number generator for shuffling
            std::random_device rd;
            std::default_random_engine rng(rd());
            std::shuffle(directions.begin(), directions.end(), rng); // Shuffle directions

            for (const auto& dir : directions) {
                int newRow = dir.first;
                int newCol = dir.second;

                // Check bounds and if tile is unassigned
                if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols &&
                    map[newRow][newCol] == -1) {
                    
                    // Calculate distances to all biome seeds
                    float minDistance = std::numeric_limits<float>::max();
                    int closestBiome = -1;

                    for (int i = 0; i < numBiomes; ++i) {
                        int seedRow = biomeSeedPositions[i].first;
                        int seedCol = biomeSeedPositions[i].second;
                        float dist = calculateDistance(newRow, newCol, seedRow, seedCol);

                        // If this biome's seed is closer, assign it
                        if (dist < minDistance) {
                            minDistance = dist;
                            closestBiome = i;
                        }
                        // If the distance is equal, pick the first seed (i will be the first seed if it's equal)
                    }

                    map[newRow][newCol] = closestBiome; // Assign the tile to the closest biome
                    biomeQueue.push({newRow, newCol});
                    ++currentSize;

                    // Stop growing this biome if max size is reached
                    if (currentSize >= maxSize) {
                        break;
                    }
                }
            }
        }
    }

    // Step 3: Assign biome types using landBiome or seaBiome
    for (int biomeID = 0; biomeID < numBiomes; ++biomeID) {
        int biome = std::rand() % 100; // Random value to decide the biome types
        // int biome = 90; // For now fix whole map to a set biome

        // Assign biomes based on random chance, ensuring each biome gets only one type
        if (biome < 40) {
            seaBiome(biomeID); // Call sea biome generation for 0-29
        } 
        else if (biome < 70) {
            landBiome(biomeID); // Call land biome generation for 30-59
        } 
        else if (biome < 82) {
            hillBiome(biomeID); // Call hill biome generation for 60-79
        } 
        else {
            mountainBiome(biomeID); // Call mountain biome generation for 80-99
        }
    }
}




// Sea biome generation function (fills biome with sea, i.e., 0)
void MapGenerator::seaBiome(int biomeID) {
    // Seed the random number generator (if not already done in other parts of your code)
    std::srand(std::time(0));

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (map[row][col] == biomeID) {
                // Generate a random number between 0 and 99 to determine the biome type
                int randVal = std::rand() % 100; // Random number between 0 and 99

                if (randVal < 60) {
                    map[row][col] = 0; // 80% chance: Sea
                } else if (randVal < 75) {
                    map[row][col] = 1; // 10% chance: Land
                } else if (randVal < 90) {
                    map[row][col] = 2; // 10% chance: Hills
                } else {
                    map[row][col] = 3; // 10% chance: Mountains
                }
            }
        }
    }
}



// Land biome generation function (fills biome with land, sea, or hills)
void MapGenerator::landBiome(int biomeID) {
    // Seed the random number generator (if not already done in other parts of your code)
    std::srand(std::time(0));

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (map[row][col] == biomeID) {
                // Generate a random number between 0 and 99 to determine the biome type
                int randVal = std::rand() % 100; // Random number between 0 and 99

                if (randVal < 40) {
                    map[row][col] = 1; // 80% chance: Land
                } else if (randVal < 70) {
                    map[row][col] = 0; // 25% chance: Sea
                } else {
                    map[row][col] = 2; // 25% chance: Hills (2 represents hills)
                }
            }
        }
    }
}


// Hill biome generation function (fills biome with hill, i.e., 2)
void MapGenerator::hillBiome(int biomeID) {
    // Seed the random number generator (if not already done in other parts of your code)
    std::srand(std::time(0));

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (map[row][col] == biomeID) {
                // Generate a random number between 0 and 99 to determine the biome type
                int randVal = std::rand() % 100; // Random number between 0 and 99

                if (randVal < 50) {
                    map[row][col] = 2; // 80% chance: Hills
                } else if (randVal < 70) {
                    map[row][col] = 0; // 10% chance: Sea
                } else if (randVal < 85) {
                    map[row][col] = 3; // 10% chance: Mountain
                } else {
                    map[row][col] = 1; // 10% chance: Land
                }
                
            }
        }
    }
}

// Mountain biome generation function (fills biome with mountain, i.e., 0)
void MapGenerator::mountainBiome(int biomeID) {
    // Seed the random number generator (if not already done in other parts of your code)
    std::srand(std::time(0));

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (map[row][col] == biomeID) {
                // Generate a random number between 0 and 99 to determine the biome type
                int randVal = std::rand() % 100; // Random number between 0 and 99

                if (randVal < 45) {
                    map[row][col] = 3; // 80% chance: Land
                } else if (randVal < 65) {
                    map[row][col] = 0; // 10% chance: Sea (0 represents sea)
                } else if (randVal < 75) {
                    map[row][col] = 1; // 10% chance: Sea (0 represents sea)
                } else {
                    map[row][col] = 2; // 10% chance: Hills (2 represents hills)
                }
            }
        }
    }
}


// Step 5: Function to fill any unassigned cells (those with -1) with sea
void MapGenerator::fillUnassignedWithSea() {
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (map[row][col] == -1) {
                map[row][col] = 0; // Replace with sea
            }
        }
    }
}

void MapGenerator::smoothMap() {
    // Create a copy of the map to store new values (to prevent modifying while iterating)
    std::vector<std::vector<int>> newMap = map;

    // Directions for checking neighbors: up, down, left, right, and diagonals
    std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},  // Cardinal directions
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Diagonal directions
    };

    // Number of smoothing iterations (you can adjust this number for more/less smoothing)
    int smoothingIterations = 6;

    for (int iter = 0; iter < smoothingIterations; ++iter) {
        // Iterate over the entire map
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                // Skip tiles that haven't been assigned yet
                if (map[row][col] == -1) {
                    continue;
                }

                // Count occurrences of each type of tile in the surrounding tiles
                int landCount = 0;
                int seaCount = 0;
                int hillsCount = 0;
                int mountainCount = 0;
                int iceCount = 0;
                int tundraCount = 0;
                int tundraHillsCount = 0;
                int taigaCount = 0;
                int taigaHillsCount = 0;
                int desertCount = 0;
                int desertHillsCount = 0;

                // Check neighbors
                for (const auto& dir : directions) {
                    int newRow = row + dir.first;
                    int newCol = col + dir.second;

                    // Check if the neighbor is within bounds
                    if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
                        int neighborTile = map[newRow][newCol];
                        // Count the neighbors based on tile type
                        if (neighborTile == 1) {
                            ++landCount;
                        } else if (neighborTile == 0) {
                            ++seaCount;
                        } else if (neighborTile == 2) {
                            ++hillsCount;
                        } else if (neighborTile == 3) {
                            ++mountainCount;
                        } else if (neighborTile == 7) {
                            ++iceCount;
                        } else if (neighborTile == 8) {
                            ++tundraCount;
                        } else if (neighborTile == 9) {
                            ++tundraHillsCount;
                        } else if (neighborTile == 10) {
                            ++taigaCount;
                        } else if (neighborTile == 11) {
                            ++taigaHillsCount;
                        } else if (neighborTile == 12) {
                            ++desertCount;
                        } else if (neighborTile == 13) {
                            ++desertHillsCount;
                        }
                    }
                }

                // Normalize counts: consider the counts in proportion
                int totalCount = landCount + seaCount + hillsCount + mountainCount + iceCount + tundraCount + tundraHillsCount +
                                 taigaCount + taigaHillsCount + desertCount + desertHillsCount;

                // Avoid division by zero and only proceed if there's a valid total count of neighbors
                if (totalCount == 0) {
                    continue;
                }

                // Determine the majority type in the surrounding area
                int majorityTile = 1; // Default to land

                // Normalize counts to get a relative weight for each type
                double landWeight = (double)landCount / totalCount;
                double seaWeight = (double)seaCount / totalCount;
                double hillsWeight = (double)hillsCount / totalCount;
                double mountainWeight = (double)mountainCount / totalCount;
                double iceWeight = (double)iceCount / totalCount;
                double tundraWeight = (double)tundraCount / totalCount;
                double tundraHillsWeight = (double)tundraHillsCount / totalCount;
                double taigaWeight = (double)taigaCount / totalCount;
                double taigaHillsWeight = (double)taigaHillsCount / totalCount;
                double desertWeight = (double)desertCount / totalCount;
                double desertHillsWeight = (double)desertHillsCount / totalCount;

                // Based on the relative weight of each type, decide on the majority terrain
                if (seaWeight > landWeight && seaWeight > hillsWeight && seaWeight > mountainWeight && seaWeight > iceWeight &&
                    seaWeight > tundraWeight && seaWeight > tundraHillsWeight && seaWeight > taigaWeight && seaWeight > taigaHillsWeight &&
                    seaWeight > desertWeight && seaWeight > desertHillsWeight) {
                    majorityTile = 0; // Majority sea
                } else if (landWeight > hillsWeight && landWeight > seaWeight && landWeight > mountainWeight && landWeight > iceWeight &&
                           landWeight > tundraWeight && landWeight > tundraHillsWeight && landWeight > taigaWeight && landWeight > taigaHillsWeight &&
                           landWeight > desertWeight && landWeight > desertHillsWeight) {
                    majorityTile = 1; // Majority hills
                } else if (hillsWeight > landWeight && hillsWeight > seaWeight && hillsWeight > mountainWeight && hillsWeight > iceWeight &&
                           hillsWeight > tundraWeight && hillsWeight > tundraHillsWeight && hillsWeight > taigaWeight && hillsWeight > taigaHillsWeight &&
                           hillsWeight > desertWeight && hillsWeight > desertHillsWeight) {
                    majorityTile = 2; // Majority hills
                } else if (mountainWeight > landWeight && mountainWeight > seaWeight && mountainWeight > hillsWeight && mountainWeight > iceWeight &&
                           mountainWeight > tundraWeight && mountainWeight > tundraHillsWeight && mountainWeight > taigaWeight && mountainWeight > taigaHillsWeight &&
                           mountainWeight > desertWeight && mountainWeight > desertHillsWeight) {
                    majorityTile = 3; // Majority mountain
                } else if (iceWeight > landWeight && iceWeight > seaWeight && iceWeight > hillsWeight && iceWeight > mountainWeight &&
                           iceWeight > tundraWeight && iceWeight > tundraHillsWeight && iceWeight > taigaWeight && iceWeight > taigaHillsWeight &&
                           iceWeight > desertWeight && iceWeight > desertHillsWeight) {
                    majorityTile = 7; // Majority ice
                } else if (tundraWeight > landWeight && tundraWeight > seaWeight && tundraWeight > hillsWeight && tundraWeight > mountainWeight &&
                           tundraWeight > iceWeight && tundraWeight > tundraHillsWeight && tundraWeight > taigaWeight && tundraWeight > taigaHillsWeight &&
                           tundraWeight > desertWeight && tundraWeight > desertHillsWeight) {
                    majorityTile = 8; // Majority tundra
                } else if (tundraHillsWeight > landWeight && tundraHillsWeight > seaWeight && tundraHillsWeight > hillsWeight && tundraHillsWeight > mountainWeight &&
                           tundraHillsWeight > iceWeight && tundraHillsWeight > tundraWeight && tundraHillsWeight > taigaWeight && tundraHillsWeight > taigaHillsWeight &&
                           tundraHillsWeight > desertWeight && tundraHillsWeight > desertHillsWeight) {
                    majorityTile = 9; // Majority tundra hills
                } else if (taigaWeight > landWeight && taigaWeight > seaWeight && taigaWeight > hillsWeight && taigaWeight > mountainWeight &&
                           taigaWeight > iceWeight && taigaWeight > tundraWeight && taigaWeight > tundraHillsWeight && taigaWeight > taigaHillsWeight &&
                           taigaWeight > desertWeight && taigaWeight > desertHillsWeight) {
                    majorityTile = 10; // Majority taiga
                } else if (taigaHillsWeight > landWeight && taigaHillsWeight > seaWeight && taigaHillsWeight > hillsWeight && taigaHillsWeight > mountainWeight &&
                           taigaHillsWeight > iceWeight && taigaHillsWeight > tundraWeight && taigaHillsWeight > tundraHillsWeight && taigaHillsWeight > taigaWeight &&
                           taigaHillsWeight > desertWeight && taigaHillsWeight > desertHillsWeight) {
                    majorityTile = 11; // Majority taiga hills
                } else if (desertWeight > landWeight && desertWeight > seaWeight && desertWeight > hillsWeight && desertWeight > mountainWeight &&
                           desertWeight > iceWeight && desertWeight > tundraWeight && desertWeight > tundraHillsWeight && desertWeight > taigaWeight &&
                           desertWeight > taigaHillsWeight && desertWeight > desertHillsWeight) {
                    majorityTile = 12; // Majority desert
                } else if (desertHillsWeight > landWeight && desertHillsWeight > seaWeight && desertHillsWeight > hillsWeight && desertHillsWeight > mountainWeight &&
                           desertHillsWeight > iceWeight && desertHillsWeight > tundraWeight && desertHillsWeight > tundraHillsWeight && desertHillsWeight > taigaWeight &&
                           desertHillsWeight > taigaHillsWeight && desertHillsWeight > desertWeight) {
                    majorityTile = 13; // Majority desert hills
                }

                // Set the new tile type in the new map
                newMap[row][col] = majorityTile;
            }
        }

        // Update the original map with the new values after one iteration
        map = newMap;
    }
}


void MapGenerator::blendMap() {
    // Create a copy of the map to store new values (to prevent modifying while iterating)
    std::vector<std::vector<int>> newMap = map;

    // Directions for checking neighbors: up, down, left, right, and diagonals
    std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},  // Cardinal directions
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Diagonal directions
    };

    // Number of smoothing iterations (you can adjust this number for more/less smoothing)
    int smoothingIterations = 3;

    // Random number generator for weighted random selection
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int iter = 0; iter < smoothingIterations; ++iter) {
        // Iterate over the entire map
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                // Skip tiles that haven't been assigned yet
                if (map[row][col] == -1) {
                    continue;
                }

                // Count occurrences of each type of tile in the surrounding tiles
                int landCount = 0;
                int seaCount = 0;
                int hillsCount = 0;
                int mountainCount = 0;
                int iceCount = 0;
                int tundraCount = 0;   // New counter for tundra tiles
                int tundraHillsCount = 0;   // New counter for tundra hills tiles
                int taigaCount = 0;   // New counter for taiga tiles
                int taigaHillsCount = 0;   // New counter for taiga hills tiles
                int desertCount = 0;   // New counter for desert tiles
                int desertHillsCount = 0;   // New counter for desert hills tiles

                // Check neighbors
                for (const auto& dir : directions) {
                    int newRow = row + dir.first;
                    int newCol = col + dir.second;

                    // Check if the neighbor is within bounds
                    if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
                        int neighborTile = map[newRow][newCol];
                        // Count the neighbors based on tile type
                        if (neighborTile == 1) {
                            ++landCount;
                        } else if (neighborTile == 0) {
                            ++seaCount;
                        } else if (neighborTile == 2) {
                            ++hillsCount;
                        } else if (neighborTile == 3) {
                            ++mountainCount;
                        } else if (neighborTile == 7) {
                            ++iceCount;
                        } else if (neighborTile == 8) {
                            ++tundraCount;
                        } else if (neighborTile == 9) {
                            ++tundraHillsCount;
                        } else if (neighborTile == 10) {
                            ++taigaCount;
                        } else if (neighborTile == 11) {
                            ++taigaHillsCount;
                        } else if (neighborTile == 12) {
                            ++desertCount;
                        } else if (neighborTile == 13) {
                            ++desertHillsCount;
                        }
                    }
                }

                // Calculate total count of neighbors
                int totalCount = landCount + seaCount + hillsCount + mountainCount + iceCount + tundraCount + tundraHillsCount + taigaCount + taigaHillsCount + desertCount + desertHillsCount;

                // Normalize the counts to probabilities
                float landWeight = (totalCount > 0) ? (float)landCount / totalCount : 0.1f;
                float seaWeight = (totalCount > 0) ? (float)seaCount / totalCount : 0.1f;
                float hillsWeight = (totalCount > 0) ? (float)hillsCount / totalCount : 0.1f;
                float mountainWeight = (totalCount > 0) ? (float)mountainCount / totalCount : 0.1f;
                float iceWeight = (totalCount > 0) ? (float)iceCount / totalCount : 0.1f;
                float tundraWeight = (totalCount > 0) ? (float)tundraCount / totalCount : 0.1f;
                float tundraHillsWeight = (totalCount > 0) ? (float)tundraHillsCount / totalCount : 0.1f;
                float taigaWeight = (totalCount > 0) ? (float)taigaCount / totalCount : 0.1f;
                float taigaHillsWeight = (totalCount > 0) ? (float)taigaHillsCount / totalCount : 0.1f;
                float desertWeight = (totalCount > 0) ? (float)desertCount / totalCount : 0.1f;
                float desertHillsWeight = (totalCount > 0) ? (float)desertHillsCount / totalCount : 0.1f;

                // Randomize based on weights
                float randValue = dist(rng); // Random float between 0 and 1
                int newTile = 1; // Default to land

                if (randValue < landWeight) {
                    newTile = 1; // Land
                } else if (randValue < landWeight + seaWeight) {
                    newTile = 0; // Sea
                } else if (randValue < landWeight + seaWeight + hillsWeight) {
                    newTile = 2; // Hills
                } else if (randValue < landWeight + seaWeight + hillsWeight + mountainWeight) {
                    newTile = 3; // Mountain
                } else if (randValue < landWeight + seaWeight + hillsWeight + mountainWeight + iceWeight) {
                    newTile = 7; // Ice
                } else if (randValue < landWeight + seaWeight + hillsWeight + mountainWeight + iceWeight + tundraWeight) {
                    newTile = 8; // Tundra
                } else if (randValue < landWeight + seaWeight + hillsWeight + mountainWeight + iceWeight + tundraWeight + tundraHillsWeight) {
                    newTile = 9; // Tundra Hills
                } else if (randValue < landWeight + seaWeight + hillsWeight + mountainWeight + iceWeight + tundraWeight + tundraHillsWeight + taigaWeight) {
                    newTile = 10; // Taiga
                } else if (randValue < landWeight + seaWeight + hillsWeight + mountainWeight + iceWeight + tundraWeight + tundraHillsWeight + taigaWeight + taigaHillsWeight) {
                    newTile = 11; // Taiga Hills
                } else if (randValue < landWeight + seaWeight + hillsWeight + mountainWeight + iceWeight + tundraWeight + tundraHillsWeight + taigaWeight + taigaHillsWeight + desertWeight) {
                    newTile = 12; // Desert
                } else {
                    newTile = 13; // Desert Hills
                }

                // Assign the randomized tile type to the current tile in the new map
                newMap[row][col] = newTile;
            }
        }

        // After completing the iteration, copy the newMap back to the original map
        map = newMap;
    }
}


// Function to apply the modifiers based on map position and surroundings
void MapGenerator::applyModifiers() {
    // Random number generator for probabilities
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Iterate over the entire map to apply modifiers
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // Skip tiles that haven't been assigned yet
            if (map[row][col] == -1) {
                continue;
            }

            // 1. Edge-based sea conversion
            if (map[row][col] != 0) {  // If it's not already sea
                if (col < 3 || col >= cols - 3) {
                    map[row][col] = 0;  // Convert to sea
                } else if ((col >= 3 && col <= 6) || (col >= cols - 6 && col < cols - 3)) {
                    // 60% chance of becoming sea
                    if (dist(rng) < 0.6f) {
                        map[row][col] = 0;  // Convert to sea
                    }
                }
            }

            // 2. Ice conversion based on proximity to top or bottom
            if (row < 2 || row >= rows - 2) {
                map[row][col] = 7;  // Convert to ice
            } else if ((row >= 2 && row <= 4) || (row >= rows - 4 && row < rows - 2)) {
                // 50% chance of becoming ice
                if (dist(rng) < 0.5f) {
                    map[row][col] = 7;  // Convert to ice
                }
            }

            // 3. Tundra conversion based on proximity to top or bottom
            if (map[row][col] == 1) {  // Land
                if ((row >= 4 && row <= 20) || (row >= rows - 20 && row < rows - 4)) {
                    map[row][col] = 8;  // Convert to tundra
                } else if ((row >= 20 && row <= 35) || (row >= rows - 35 && row < rows - 20)) {
                    // 50% chance of becoming tundra
                    if (dist(rng) < 0.5f) {
                        map[row][col] = 8;  // Convert to tundra
                    }
                }
            }
            if (map[row][col] == 2) {  // Hills
                if ((row >= 4 && row <= 20) || (row >= rows - 20 && row < rows - 4)) {
                    map[row][col] = 9;  // Convert to tundra hills
                } else if ((row >= 20 && row <= 35) || (row >= rows - 35 && row < rows - 20)) {
                    // 50% chance of becoming tundra hills
                    if (dist(rng) < 0.5f) {
                        map[row][col] = 9;  // Convert to tundra hills
                    }
                }
            }

            // 4. Check if surrounded only by mountains or mountains + ice
            if (isSurroundedByMountainsOrIce(row, col)) {
                if (dist(rng) < 0.5f) {
                    map[row][col] = 7;  // Convert to ice
                }
            }

            // 5. Desert conversion based on proximity to equator
            if (row >= rows / 3 && row < 2 * rows / 3) {  // Equatorial band
                if (map[row][col] == 1) {  // Land
                    map[row][col] = 12;  // Convert to desert
                } else if (map[row][col] == 2) {  // Hills
                    int randVal = std::rand() % 100; // Random number between 0 and 99
                    if (randVal < 40) {
                        map[row][col] = 12; // 80% chance: Land
                    }
                }
            }
        }
    }
}



// Function to apply the modifiers based on map position and surroundings
void MapGenerator::MountainPeaks() {
    // Random number generator for probabilities
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Iterate over the entire map to apply modifiers
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // Skip tiles that haven't been assigned yet
            if (map[row][col] == -1) {
                continue;
            }

            // 3. Check if surrounded only by mountains or mountains + ice
            if (isSurroundedByMountainsOrIce(row, col)) {
                if (dist(rng) < 0.5f) {
                    map[row][col] = 7;  // Convert to ice
                }
            }
        }
    }
}


// Helper function to check if a tile is surrounded only by mountains or mountains + ice
bool MapGenerator::isSurroundedByMountainsOrIce(int row, int col) {
    // Directions for checking neighbors: up, down, left, right, and diagonals
    std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},  // Cardinal directions
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Diagonal directions
    };

    for (const auto& dir : directions) {
        int newRow = row + dir.first;
        int newCol = col + dir.second;

        // Check if the neighbor is within bounds
        if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
            int neighborTile = map[newRow][newCol];
            // If the neighbor is not a mountain or ice, return false
            if (neighborTile != 3 && neighborTile != 7) {
                return false;
            }
        }
    }
    return true;  // If all neighbors are mountains or ice
}


// Function to apply the modifiers based on map position and surroundings
void MapGenerator::ForceTundra() {
    // Random number generator for probabilities
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Iterate over the entire map to apply modifiers
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {


            // 3. Tundra conversion based on proximity to top or bottom
            if (map[row][col] == 1) {
                if (row >= 0 && row <= (rows/15) || row >= rows - (rows/15) && row < rows - 0) {
                    map[row][col] = 8;  // Convert to tundra
                }
            }
            if (map[row][col] == 2) {
                if (row >= 0 && row <= (rows/15) || row >= rows - (rows/15) && row < rows - 0) {
                    map[row][col] = 9;  // Convert to tundra hills
                }
            }
        }
    }
}


// Function to apply the modifiers based on map position and surroundings
void MapGenerator::ForceDesert() {
    // Random number generator for probabilities
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Iterate over the entire map to apply modifiers
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // 5. Desert conversion based on proximity to equator
            if (row >= rows / 2.3 && row < 2 * rows / 4) {  // Equatorial band
                if (map[row][col] == 1) {  // Land
                    map[row][col] = 12;  // Convert to desert
                } else if (map[row][col] == 2) {  // Hills
                    map[row][col] = 13;  // Convert to desert hills
                    int randVal = std::rand() % 100; // Random number between 0 and 99
                    if (randVal < 40) {
                        map[row][col] = 12; // 80% chance: Land
                    }
                }
            }
        }
    }
}

void MapGenerator::resetHeightMapToZero(std::vector<std::vector<int>>& heightMap, const std::vector<std::vector<int>>& map) {
    int rows = map.size();
    int cols = map[0].size();

    // Ensure heightMap has the same dimensions as map
    heightMap.resize(rows);
    for (int i = 0; i < rows; ++i) {
        heightMap[i].resize(cols, 0); // Set all elements to 0
    }
}


std::vector<std::vector<int>> MapGenerator::generateHeightMap() {
    // Create a height map with the same dimensions as the map
    std::vector<std::vector<int>> heightMap(rows, std::vector<int>(cols, 0));
    // std::cout << "HeightMap size: " << heightMap.size() << " x " << heightMap[0].size() << std::endl;
    std::vector<std::vector<int>> distanceToSea(rows, std::vector<int>(cols, std::numeric_limits<int>::max()));

    // Helper function: Multi-source BFS to precompute distances to the nearest sea
    auto precomputeDistances = [&]() {
        std::queue<std::pair<int, int>> q;

        // Add all sea tiles to the queue and set their distance to 0
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (map[r][c] == 0) {  // Sea tile
                    q.push({r, c});
                    distanceToSea[r][c] = 0;
                }
            }
        }

        // Directions for adjacent tiles
        std::vector<std::pair<int, int>> directions = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}  // Cardinal directions
        };

        // BFS to calculate minimum distance to any sea
        while (!q.empty()) {
            auto [curRow, curCol] = q.front();
            q.pop();

            for (const auto& dir : directions) {
                int newRow = curRow + dir.first;
                int newCol = curCol + dir.second;

                // Check if the tile is valid and hasn't been visited yet
                if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
                    int newDistance = distanceToSea[curRow][curCol] + 1;
                    if (newDistance < distanceToSea[newRow][newCol]) {
                        distanceToSea[newRow][newCol] = newDistance;
                        q.push({newRow, newCol});
                    }
                }
            }
        }
    };

    // Precompute distances to the nearest sea
    precomputeDistances();

    // Initialize random number generator for 5% chance
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);  // Generates a random number between 1 and 100

    // Iterate over all map tiles and calculate the height
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int height = 0;
            int numHills = 0;
            int numMountains = 0;
            int numSeas = 0;

            // Calculate the number of adjacent hills, mountains, and seas
            std::vector<std::pair<int, int>> directions = {
                {-1, 0}, {1, 0}, {0, -1}, {0, 1},  // Cardinal directions
                {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Diagonal directions
            };

            for (const auto& dir : directions) {
                int newRow = row + dir.first;
                int newCol = col + dir.second;

                if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
                    int neighborTile = map[newRow][newCol];
                    if (neighborTile == 2 || neighborTile == 9 || neighborTile == 13) { // Hills or hill types
                        numHills++;
                    } else if (neighborTile == 3) {  // Mountains
                        numMountains++;
                    } else if (neighborTile == 0) {  // Sea
                        numSeas++;
                    }
                }
            }

            // Set height based on tile type
            if (map[row][col] == 0) {  // Sea
                height = 0;
            } else if (map[row][col] == 3 || map[row][col] == 16) {  // Mountain or lake
                height = 100;
            } else if (map[row][col] == 1 || map[row][col] == 8 || map[row][col] == 12) {  // Land, tundra, desert
                height = 40 + numHills * 3 + numMountains * 5 - numSeas * 5;
            } else if (map[row][col] == 2 || map[row][col] == 9 || map[row][col] == 13) {  // Hill, tundra hill, desert hill
                height = 60 + numHills * 3 + numMountains * 5 - numSeas * 5;
            }

            // Add precomputed distance from the nearest sea
            height += distanceToSea[row][col];

            // If height > 70, check for 5% chance to convert to a river (tile number 5)
            if (height > 50 && height < 70) {
                int chance = dis(gen);  // Get a random number between 1 and 100
                if (chance <= 1) {  // 3% chance
                    map[row][col] = 5;  // Convert to river
                    continue;  // Skip further height modification for this tile
                }
            }
            else if (height > 70) {
                int chance = dis(gen);  // Get a random number between 1 and 100
                if (chance <= 1) {  // 1% chance
                    map[row][col] = 5;  // Convert to river
                    continue;  // Skip further height modification for this tile
                }
            }
            else if (height > 1) {
                int chance = dis(gen);  // Get a random number between 1 and 100
                if (chance <= 1) {  // 1% chance
                    map[row][col] = 5;  // Convert to river
                    continue;  // Skip further height modification for this tile
                }
            }

            // Set the calculated height in the height map
            heightMap[row][col] = height;
        }
    }

    // Check if all values have been set in the heightMap
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (heightMap[row][col] == 0) {  // If any tile is still 0
                heightMap[row][col] = 0;  // Set to 0 (redundant but explicit)
            }
        }
    }
    // Check if the heightMap is valid before printing its size
    // if (!heightMap.empty() && !heightMap[0].empty()) {
    //     std::cout << "HeightMap size (check 2): " << heightMap.size() << " x " << heightMap[0].size() << std::endl;
    // } else {
    //     std::cerr << "Error: heightMap (check 2) is empty or improperly initialized!" << std::endl;
    // }

    return heightMap;
    // The heightMap is now complete and can be used for further processing or rendering
}


void MapGenerator::flowRivers(std::vector<std::vector<int>>& heightMap, std::vector<std::vector<int>>& map) {
    bool foundRiver = true; // Flag to check if there are more river tiles to process
    std::srand(std::time(0)); // Seed the random number generator

    while (foundRiver) {
        foundRiver = false;

        // Iterate through the map to find a river tile
        for (int row = 0; row < map.size(); ++row) {
            for (int col = 0; col < map[row].size(); ++col) {
                if (map[row][col] == 5) {
                    // Process the river tile
                    map[row][col] = 6; // Mark as processed
                    heightMap[row][col] = 200;
                    foundRiver = true; // Mark that we found a tile to process
                    // std::cout << "Processed tile at (" << row << ", " << col << ")\n";

                    // Find adjacent tiles
                    std::vector<std::pair<int, int>> adjacentTiles;
                    int minHeight = std::numeric_limits<int>::max();  // Initialize with the highest possible value
                    std::pair<int, int> minTile = {-1, -1};  // To store the position of the tile with the minimum height

                    for (int dr = -1; dr <= 1; ++dr) {
                        for (int dc = -1; dc <= 1; ++dc) {
                            int adjRow = row + dr;
                            int adjCol = col + dc;

                            // Ensure we stay within the map bounds
                            if (adjRow >= 0 && adjRow < map.size() && adjCol >= 0 && adjCol < map[row].size()) {
                                // Skip the current tile and check only valid adjacent tiles
                                if (!(dr == 0 && dc == 0)) {
                                    // Find the minimum height among the adjacent tiles
                                    if (heightMap[adjRow][adjCol] < minHeight) {
                                        minHeight = heightMap[adjRow][adjCol];
                                        minTile = {adjRow, adjCol};
                                    }
                                }
                            }
                        }
                    }

                    // If a valid adjacent tile with the minimum height is found, set it to 6
                    if (minTile.first != -1 && minTile.second != -1) {
                        int adjRow = minTile.first;
                        int adjCol = minTile.second;

                        // Set the selected adjacent tile to 6
                        if (map[adjRow][adjCol] != 0 && map[adjRow][adjCol] != 6) {
                            map[adjRow][adjCol] = 5;
                        }
                        else {break;}
                        // std::cout << "Processed adjacent tile at (" << adjRow << ", " << adjCol << ")\n";
                        // std::cout << "Adjacent tile height (" << heightMap[adjRow][adjCol] << ")\n";
                    }

                    break; // Exit inner loop after processing the river tile
                }
            }
            if (foundRiver) break; // Exit outer loop if a river tile was found
        }
    }
}



void MapGenerator::changeSmallSeasToRivers(std::vector<std::vector<int>>& map) {
    // Direction vectors for 8-connected neighbors (N, NE, E, SE, S, SW, W, NW)
    const std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };
    
    // Helper function to perform a DFS flood fill
    auto floodFill = [&](int row, int col, std::vector<std::vector<int>>& visited) {
        std::stack<std::pair<int, int>> stack;  // Stack for DFS
        stack.push({row, col});
        visited[row][col] = 1;  // Mark as visited
        
        int seaSize = 0;
        std::vector<std::pair<int, int>> seaTiles;  // To store the sea tiles

        while (!stack.empty()) {
            auto [r, c] = stack.top();
            stack.pop();
            seaSize++;
            seaTiles.push_back({r, c});

            // Check all 8 adjacent directions
            for (const auto& dir : directions) {
                int adjRow = r + dir.first;
                int adjCol = c + dir.second;

                if (adjRow >= 0 && adjRow < map.size() && adjCol >= 0 && adjCol < map[0].size()) {
                    if (map[adjRow][adjCol] == 0 && visited[adjRow][adjCol] == 0) {
                        visited[adjRow][adjCol] = 1;  // Mark as visited
                        stack.push({adjRow, adjCol});
                    }
                }
            }
        }
        return std::make_pair(seaSize, seaTiles);
    };

    // Initialize a visited grid with 0s (unvisited)
    std::vector<std::vector<int>> visited(map.size(), std::vector<int>(map[0].size(), 0));

    // Iterate over the map to find sea tiles (0 represents sea)
    for (int row = 0; row < map.size(); ++row) {
        for (int col = 0; col < map[row].size(); ++col) {
            if (map[row][col] == 0 && visited[row][col] == 0) {  // Found an unvisited sea tile
                auto [seaSize, seaTiles] = floodFill(row, col, visited);

                // If the sea is smaller than 100 tiles, change all its tiles to river (5)
                int minSeaSize = cols/3;
                if (seaSize < minSeaSize) {
                    for (const auto& tile : seaTiles) {
                        map[tile.first][tile.second] = 16;  // Change sea to river
                    }
                    // std::cout << "Small sea found with size " << seaSize << " at (" << row << ", " << col << "), converted to river.\n";
                }
            }
        }
    }
}


void MapGenerator::changeDesertToFloodplains(std::vector<std::vector<int>>& map) {
    // Direction vectors for 8-connected neighbors (N, NE, E, SE, S, SW, W, NW)
    const std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };

    // Iterate over the map to check desert tiles (value 12) adjacent to river tiles (value 16)
    for (int row = 0; row < map.size(); ++row) {
        for (int col = 0; col < map[row].size(); ++col) {
            if (map[row][col] == 12) {  // Check if the current tile is a desert tile
                // Check adjacent tiles
                for (const auto& dir : directions) {
                    int adjRow = row + dir.first;
                    int adjCol = col + dir.second;

                    if (adjRow >= 0 && adjRow < map.size() && adjCol >= 0 && adjCol < map[row].size()) {
                        if (map[adjRow][adjCol] == 6) {  // If adjacent tile is a river (16)
                            map[row][col] = 17;  // Change desert tile to floodplain (17)
                            // std::cout << "Changed desert tile at (" << row << ", " << col << ") to floodplain.\n";
                            break;  // Stop checking other adjacent tiles once the change is made
                        }
                    }
                }
            }
        }
    }
}


void MapGenerator::applyForestChance(std::vector<std::vector<int>>& map) {
    // Seed the random number generator (if not done previously)
    std::srand(std::time(nullptr));

    // Get map dimensions
    int rows = map.size();
    int cols = map[0].size();

    // Iterate over the map
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int tile = map[row][col];

            // Calculate the weighted chance based on row position
            double chance = 0;  // Base chance for forest creation

            // Weighted by position: top sixth or bottom sixth gets higher chance
            if (row < rows / 2.5 || row >= 5 * rows / 6.3) {
                chance = 0.1;  // Increase the chance for top and bottom sixth
            }
            // Weighted by position: top sixth or bottom sixth gets higher chance
            if (row < rows / 4.3 || row >= 5 * rows / 6.1) {
                chance = 0.5;  // Increase the chance for top and bottom sixth
            }
            // Weighted by position: top sixth or bottom sixth gets higher chance
            if (row < rows / 5.5 || row >= 5 * rows / 5.6) {
                chance = 0.7;  // Increase the chance for top and bottom sixth
            }


            // Apply random chance to land (1) and hills (2)
            if (tile == 1) {  // Land tile
                if (static_cast<double>(std::rand()) / RAND_MAX < chance) {
                    map[row][col] = 18;  // Turn into forest (18)
                    // std::cout << "Land at (" << row << ", " << col << ") turned into forest.\n";
                }
            } else if (tile == 2) {  // Hill tile
                if (static_cast<double>(std::rand()) / RAND_MAX < chance) {
                    map[row][col] = 19;  // Turn into forest hill (19)
                    // std::cout << "Hill at (" << row << ", " << col << ") turned into forest hill.\n";
                }
            }
        }
    }
}



void MapGenerator::convertToTaiga(std::vector<std::vector<int>>& map) {
    int rows = map.size();
    int cols = map[0].size();
    
    // Calculate the indices for the top and bottom 1/6 of the map
    int topLimit = rows / 6.5;
    int bottomLimit = 5 * rows / 6;

    // Iterate over the top 1/6th of the map
    for (int row = 0; row < topLimit; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (map[row][col] == 1) {
                // Land tile, change to taiga
                map[row][col] = 10; // Taiga
            } else if (map[row][col] == 2) {
                // Hill tile, change to taiga hills
                map[row][col] = 11; // Taiga Hills
            }
        }
    }

    // Iterate over the bottom 1/6th of the map
    for (int row = bottomLimit; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (map[row][col] == 1) {
                // Land tile, change to taiga
                map[row][col] = 10; // Taiga
            } else if (map[row][col] == 2) {
                // Hill tile, change to taiga hills
                map[row][col] = 11; // Taiga Hills
            }
        }
    }
}



void MapGenerator::applyJungleChance(std::vector<std::vector<int>>& map) {
    // Seed the random number generator (if not done previously)
    std::srand(std::time(nullptr));

    // Get map dimensions
    int rows = map.size();
    int cols = map[0].size();

    // Iterate over the map
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int tile = map[row][col];

            // Calculate the weighted chance based on row position (tropical zones)
            double chance = 0;  // Base chance for jungle creation
            // Weighted by position: middle of the map (near the tropics) gets higher chance
            if (row >= rows / 3 && row <= 2 * rows / 3) {
                chance = 0.2;  // Moderate chance in the tropical region
            }
            // Weighted by position: middle of the map (near the tropics) gets higher chance
            if (row >= rows / 2.6 && row <= 2 * rows / 3.2) {
                chance = 0.4;  // Moderate chance in the tropical region
            }
            // Weighted by position: middle of the map (near the tropics) gets higher chance
            if (row >= rows / 2.4 && row <= 2 * rows / 3.4) {
                chance = 0.7;  // Moderate chance in the tropical region
            }

            // Apply random chance to land (1) and hills (2)
            if (tile == 1) {  // Land tile
                if (static_cast<double>(std::rand()) / RAND_MAX < chance) {
                    map[row][col] = 20;  // Turn into jungle (20)
                    // std::cout << "Land at (" << row << ", " << col << ") turned into jungle.\n";
                }
            } else if (tile == 2) {  // Hill tile
                if (static_cast<double>(std::rand()) / RAND_MAX < chance) {
                    map[row][col] = 21;  // Turn into jungle hill (21)
                    // std::cout << "Hill at (" << row << ", " << col << ") turned into jungle hill.\n";
                }
            }
        }
    }
}


void MapGenerator::applyCoastChance(std::vector<std::vector<int>>& map) {
    // Seed the random number generator (if not done previously)
    std::srand(std::time(nullptr));

    // Get map dimensions
    int rows = map.size();
    int cols = map[0].size();

    // Directions for neighboring tiles (including diagonals)
    std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},   // N, S, W, E
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // NE, NW, SE, SW
    };

    // Iterate over the map to find sea tiles
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (map[row][col] == 0) {  // Sea tile found
                bool adjacentToLand = false;
                bool moreAdjacentToLand = false;
                // Check neighboring tiles within a 1-tile radius
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        // Ensure we stay within the bounds of the map
                        int newRow = row + dr;
                        int newCol = col + dc;
                        if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
                            // Check if the neighbor is non-sea and non-ice
                            if (map[newRow][newCol] != 0 && map[newRow][newCol] != 7 && map[newRow][newCol] != 22) {
                                moreAdjacentToLand = true;
                                break;
                            }
                        }
                    }
                    if (moreAdjacentToLand) break;
                }
                // Check neighboring tiles within a 2-tile radius
                for (int dr = -2; dr <= 2; ++dr) {
                    for (int dc = -2; dc <= 2; ++dc) {
                        // Ensure we stay within the bounds of the map
                        int newRow = row + dr;
                        int newCol = col + dc;
                        if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
                            // Check if the neighbor is non-sea and non-ice
                            if (map[newRow][newCol] != 0 && map[newRow][newCol] != 7 && map[newRow][newCol] != 22) {
                                adjacentToLand = true;
                                break;
                            }
                        }
                    }
                    if (adjacentToLand) break;
                }

                // If the sea tile is adjacent to non-sea, non-ice tile, apply a 50% chance to turn into coast
                if (adjacentToLand) {
                    if (static_cast<double>(std::rand()) / RAND_MAX < 0.5) {
                        map[row][col] = 22;  // Turn into coast (22)
                        // std::cout << "Sea at (" << row << ", " << col << ") turned into coast.\n";
                    }
                }
                if (moreAdjacentToLand) {
                    if (static_cast<double>(std::rand()) / RAND_MAX < 1) {
                        map[row][col] = 22;  // Turn into coast (22)
                        // std::cout << "Sea at (" << row << ", " << col << ") turned into coast.\n";
                    }
                }
            }
        }
    }
}

void MapGenerator::applyDeepOceanChance(std::vector<std::vector<int>>& map) {
    // Seed the random number generator (if not done previously)
    std::srand(std::time(nullptr));

    // Get map dimensions
    int rows = map.size();
    int cols = map[0].size();

    // Iterate over the map
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int tile = map[row][col];

            // Calculate the weighted chance based on row position
            double chance = 0.3;

            // Apply random chance to land (1) and hills (2)
            if (tile == 0) {  // Land tile
                if (static_cast<double>(std::rand()) / RAND_MAX < chance) {
                    map[row][col] = 23;  // Turn into ocean
                    // std::cout << "Land at (" << row << ", " << col << ") turned into jungle.\n";
                }
            }
        }
    }
}


sf::Color MapGenerator::getTileColor(int tileType) const {
    switch (tileType) {
        case 0: return sf::Color::Blue;                     // Sea
        // case 1: return sf::Color::Green;                    // Land
        case 1: return {154, 255, 0};                    // Land
        case 2: return {165, 217, 117};                     // Hills
        case 3: return {169, 169, 169};                     // Mountain
        case 5: return sf::Color::Red;                      // River source
        // case 6: return {0, 128, 255};                       // River
        case 6: return {0, 94, 255};                       // River
        case 7: return sf::Color::White;                    // Ice
        case 8: return {149, 158, 133};                     // Tundra
        case 9: return {191, 201, 171};                     // Tundra hills
        case 10: return {70, 97, 24};                       // Taiga
        case 11: return {109, 148, 41};                     // Taiga hills
        case 12: return {255, 236, 91};                     // Desert
        case 13: return {224, 181, 81};                     // Desert hills
        case 14: return sf::Color::Red;                     // River source again?
        case 15: return sf::Color::White;                   // Ice cap
        // case 16: return {2, 113, 224};                      // Lake
        case 16: return {0, 94, 255};                      // Lake
        case 17: return {137, 227, 0};                       // Floodplain
        case 18: return {1, 51, 3};                         // Forest
        case 19: return {3, 107, 7};                        // Forest hills
        case 20: return {29, 173, 39};                      // Jungle
        case 21: return {36, 212, 48};                      // Jungle hills
        case 22: return {0, 94, 255};                      // Coast
        // case 22: return {54, 73, 227};                      // Coast
        default: return sf::Color::Magenta;                // Unknown / debug
    }
}






sf::VertexArray MapGenerator::createGrid(float cellSize) {
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);
    vertices.resize(rows * cols * 6); // 2 triangles per cell, 3 vertices each

    // Random number generator for dappling
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> offsetDist(-15, 15); // Small color variation

    auto getColor = [](int tile) -> sf::Color {
        switch (tile) {
            case 0: return sf::Color::Blue;
            case 1: return {154, 255, 0};
            case 2: return {165, 217, 117};
            case 3: return {169, 169, 169};
            case 5: return sf::Color::Red;
            case 6: return {0, 94, 255};
            case 7: return sf::Color::White;
            case 8: return {149, 158, 133};
            case 9: return {191, 201, 171};
            case 10: return {70, 97, 24};
            case 11: return {109, 148, 41};
            case 12: return {255, 236, 91};
            case 13: return {224, 181, 81};
            case 14: return sf::Color::Red;
            case 15: return sf::Color::White;
            case 16: return {0, 94, 255};
            case 17: return {137, 227, 0};
            case 18: return {1, 51, 3};
            case 19: return {3, 107, 7};
            case 20: return {29, 173, 39};
            case 21: return {36, 212, 48};
            case 22: return {0, 94, 255};
            case 23: return sf::Color(22, 0, 224);
            default: return sf::Color::Magenta;
        }
    };

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float x = col * cellSize;
            float y = row * cellSize;

            // Base color from tile type
            sf::Color baseColor = getColor(map[row][col]);

            // Dappling: slight random offset per RGB channel
            int r = std::clamp(baseColor.r + offsetDist(gen), 0, 255);
            int g = std::clamp(baseColor.g + offsetDist(gen), 0, 255);
            int b = std::clamp(baseColor.b + offsetDist(gen), 0, 255);
            sf::Color color(r, g, b);

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







sf::Texture MapGenerator::createGridTexture(float cellSize) {
    sf::Texture renderTexture(sf::Vector2u(cols * cellSize, rows * cellSize)); // Throws sf::Exception if an error occurs
}

sf::Color MapGenerator::getColorForTile(int tileType) {
    switch (tileType) {
        case 0: return sf::Color::Blue; // Sea
        case 1: return sf::Color::Green; // Land
        case 2: return sf::Color(176, 237, 119); // Hills
        case 3: return sf::Color(169, 169, 169); // Mountain
        case 5: return sf::Color::Red; // River source
        case 6: return sf::Color(0, 128, 255); // River
        case 7: return sf::Color::White; // Ice
        case 8: return sf::Color(149, 158, 133); // Tundra
        case 9: return sf::Color(191, 201, 171); // Tundra hills
        case 10: return sf::Color(70, 97, 24); // Taiga
        case 11: return sf::Color(109, 148, 41); // Taiga hills
        case 12: return sf::Color(255, 214, 77); // Desert
        case 13: return sf::Color(224, 181, 81); // Desert hills
        case 14: return sf::Color::Red; // River source
        case 15: return sf::Color::White; // Ice cap
        case 16: return sf::Color(2, 113, 224); // Lake
        case 17: return sf::Color(4, 148, 12); // Floodplain
        case 18: return sf::Color(1, 51, 3); // Forest
        case 19: return sf::Color(3, 107, 7); // Forest hills
        case 20: return sf::Color(29, 173, 39); // Jungle
        case 21: return sf::Color(36, 212, 48); // Jungle hills
        case 22: return sf::Color(54, 73, 227); // Coast
        case 23: return sf::Color(22, 0, 224); // Ocean
        default: return sf::Color::Blue; // Default fallback
    }
}
