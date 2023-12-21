#pragma once
#include <vector>
#include <fstream>
#include <sstream>
#include <tuple>

struct Coord
{
    double lat;
    double lon;
};

struct DQuery
{
    Coord coord;
    double distance;
};

struct RQuery
{
    Coord a;
    Coord b;
};

std::vector<Coord> load_coordinates(std::string binFile, const Coord &translation = {0, 0})
{
    std::vector<Coord> coordinates;

    std::ifstream fin(binFile, std::ios::binary);

    double lat, lon;

    while (fin.read(reinterpret_cast<char *>(&lat), sizeof(double)) && fin.read(reinterpret_cast<char *>(&lon), sizeof(double)))
    {
        coordinates.push_back({lat + translation.lat, lon + translation.lon});
    }

    return coordinates;
}

std::vector<DQuery> _load_distance_queries(std::string queryFile, const Coord &translation = {0, 0})
{
    std::vector<DQuery> queries;
    std::ifstream fin(queryFile);

    std::string line;

    while (std::getline(fin, line))
    {
        std::string value;
        std::stringstream ss(line);

        std::getline(ss, value, ',');
        double lat = std::stod(value);
        std::getline(ss, value, ',');
        double lon = std::stod(value);
        std::getline(ss, value);
        double d = std::stod(value);

        queries.push_back({{lat + translation.lat, lon + translation.lon}, d});
    }

    return queries;
}

std::vector<RQuery> _load_range_queries(std::string queryFile, const Coord &translation = {0, 0})
{
    std::vector<RQuery> queries;
    std::ifstream fin(queryFile);

    std::string line;

    while (std::getline(fin, line))
    {
        std::string value;
        std::stringstream ss(line);

        std::getline(ss, value, ',');
        double lata = std::stod(value);
        std::getline(ss, value, ',');
        double lona = std::stod(value);
        std::getline(ss, value, ',');
        double latb = std::stod(value);
        std::getline(ss, value);
        double lonb = std::stod(value);

        queries.push_back({{lata + translation.lat, lona + translation.lon}, {latb + translation.lat, lonb + translation.lon}});
    }

    return queries;
}