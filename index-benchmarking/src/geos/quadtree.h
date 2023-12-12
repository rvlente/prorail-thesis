#pragma once
#include <vector>
#include <memory>
#include <gperftools/heap-profiler.h>
#include "common.h"
#include "geos/index/strtree/STRtree.h"
#include "geos/index/quadtree/Quadtree.h"
#include "geos/geom/GeometryFactory.h"
#include "../utils/bin.h"
#include "../utils/progress.h"
#include "../utils/proj.h"

void build_quadtree(const std::vector<std::unique_ptr<geos::geom::Point>> &points)
{
    ProgressBar progress(points.size());
    progress.start();

    geos::index::quadtree::Quadtree index;

    for (int i = 0; i < points.size(); i++)
    {
        index.insert(points[i]->getEnvelopeInternal(), reinterpret_cast<void *>(i));
        progress.update(i);
    }

    progress.finish();
}

void benchmark_quadtree(const std::vector<std::unique_ptr<geos::geom::Point>> &points)
{
    std::cout << "Running Quadtree benchmark..." << std::endl;

    HeapProfilerStart("heapprofile/quadtree");

    build_quadtree(points);

    HeapProfilerStop();

    std::cout << "Finished. Check quadtree heap profile for memory usage." << std::endl
              << std::endl;
}