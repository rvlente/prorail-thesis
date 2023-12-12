#pragma once
#include <vector>
#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "s2/s2point_index.h"
#include "../utils/progress.h"

std::vector<S2Point> create_s2_points(const std::vector<Coord> &points)
{
    std::cout << "Creating S2Points from coordinates..." << std::endl;

    std::vector<S2Point> s2_points;
    ProgressBar progress(points.size());
    progress.start();

    for (int i = 0; i < points.size(); i++)
    {
        s2_points.push_back(S2LatLng::FromDegrees(points[i].lat, points[i].lon).ToPoint());
        progress.update(i);
    }

    progress.finish();

    return s2_points;
}