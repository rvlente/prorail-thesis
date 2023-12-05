#pragma once
#include <vector>
#include <fstream>
#include <tuple>

struct Coord
{
    double lat;
    double lon;
};

std::vector<Coord> load_coordinates(const char *binFile)
{
    std::vector<Coord> coordinates;

    std::ifstream fin(binFile, std::ios::binary);

    double lat, lon;

    while (fin.read(reinterpret_cast<char *>(&lat), sizeof(double)) && fin.read(reinterpret_cast<char *>(&lon), sizeof(double)))
    {
        coordinates.push_back({lat, lon});
    }

    return coordinates;
}