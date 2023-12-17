#pragma once
#include <vector>
#include "s2/s2latlng.h"
#include "s2/s2point_vector_shape.h"
#include "s2/s2closest_point_query.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2contains_point_query.h"
#include "s2/s2boolean_operation.h"
#include "../utils/progress.h"
#include "../utils/data.h"

std::unique_ptr<S2PointVectorShape> create_s2_points(const std::vector<Coord> &points)
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
    std::cout << "Finished." << std::endl
              << std::endl;

    return std::make_unique<S2PointVectorShape>(s2_points);
}
