#pragma once
#include <vector>
#include <fstream>
#include <tuple>

struct Coord
{
    double lat;
    double lon;
};

std::vector<Coord> loadCoordinatesFromFile(const char *binFile)
{
    std::vector<Coord> coordinates;

    std::ifstream fin(binFile, std::ios::binary);

    float lat, lon;

    while (fin.read(reinterpret_cast<char *>(&lat), sizeof(float)) && fin.read(reinterpret_cast<char *>(&lon), sizeof(float)))
    {
        coordinates.push_back({lat, lon});
    }

    return coordinates;
}