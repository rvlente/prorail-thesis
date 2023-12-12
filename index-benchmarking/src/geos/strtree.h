#pragma once
#include <vector>
#include <memory>
#include <gperftools/heap-profiler.h>
#include "geos/index/strtree/STRtree.h"
#include "common.h"
#include "../utils/progress.h"

void build_strtree(const std::vector<std::unique_ptr<geos::geom::Point>> &points)
{
    ProgressBar progress(points.size() * 2); // Geometry insertion takes up less than half of the total progress.
    progress.start();

    auto index = new geos::index::strtree::STRtree();

    for (int i = 0; i < points.size(); i++)
    {
        index->insert(points[i]->getEnvelopeInternal(), nullptr);
        progress.update(i);
    }

    progress.finish();
}

void benchmark_strtree(std::vector<std::unique_ptr<geos::geom::Point>> &points)
{
    std::cout << "Running STRtree benchmark..." << std::endl;

    HeapProfilerStart("heapprofile/strtree");

    build_strtree(points);

    HeapProfilerStop();

    std::cout << "Finished. Check strtree heap profile for memory usage." << std::endl
              << std::endl;
}