#ifndef NAMES_HPP
#define NAMES_HPP

#include <string>
#include <vector>

class Names {
public:
    // Constructor
    Names();

    // Method to generate a name
    std::string generateName();

private:
    // Lists of syllables
    std::vector<std::string> firstSyllables;
    std::vector<std::string> middleSyllables;
    std::vector<std::string> lastSyllables;

    // Helper method to get a random syllable from a list
    std::string getRandomSyllable(const std::vector<std::string>& syllables);
};

#endif // NAMES_HPP