#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

class FogOfWarMap {
public:
    // Constructor
    FogOfWarMap(int rows, int cols);

    // Reset the entire fog grid to hidden (0)
    void resetFog();

    // Set a specific tile to visible (2)
    void reveal(int row, int col);

    // Downgrade all currently visible tiles (2) to seen (1)
    void markSeen();

    // Create a fog overlay to render based on the fogGrid values
    sf::VertexArray createFogOverlay(float cellSize) const;

    // Access the fog grid for read/write (non-const)
    std::vector<std::vector<int>>& getFogGrid();

    // Optional: Read-only access if needed elsewhere
    const std::vector<std::vector<int>>& getFogGrid() const;

private:
    int rows, cols;
    std::vector<std::vector<int>> fogGrid;  // 0 = hidden, 1 = seen, 2 = visible
};
