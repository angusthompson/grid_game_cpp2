#include "Names.hpp"
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time() to seed the random number generator

// Constructor: Initialize syllable lists and seed the random number generator
Names::Names() {
    // Seed the random number generator
    std::srand(std::time(0));

    // Populate the syllable lists
    firstSyllables = {"Ka", "Ta", "Ma", "Sa", "La"};
    middleSyllables = {"ri", "lo", "me", "na", "si"};
    lastSyllables = {"neth", "lius", "dor", "vin", "ra"};
}

// Method to generate a random name
std::string Names::generateName() {
    // Combine random syllables from each list
    std::string name = getRandomSyllable(firstSyllables) +
                       getRandomSyllable(middleSyllables) +
                       getRandomSyllable(lastSyllables);
    return name;
}

// Helper method to get a random syllable from a list
std::string Names::getRandomSyllable(const std::vector<std::string>& syllables) {
    // Generate a random index
    int index = std::rand() % syllables.size();
    return syllables[index];
}