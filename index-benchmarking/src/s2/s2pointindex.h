#pragma once
#include <vector>
#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "s2/s2point_index.h"
#include "../utils/progress.h"
#include "common.h"

void benchmark_s2pointindex(const std::vector<S2Point> &points)
{
    std::cout << "Building S2PointIndex..." << std::endl;

    S2PointIndex<int> index;
    ProgressBar progress(points.size());
    progress.start();

    for (int i = 0; i < points.size(); i++)
    {
        index.Add(points[i], i);
        progress.update(i);
    }

    progress.finish();
    std::cout << "Finished. Built index of size " << index.SpaceUsed() << " bytes." << std::endl
              << std::endl;
}