#pragma once
#include <vector>
#include <memory>
#include <gperftools/heap-profiler.h>
#include "common.h"
#include "geos/index/strtree/STRtree.h"
#include "geos/index/quadtree/Quadtree.h"
#include "geos/geom/GeometryFactory.h"
#include "../utils/progress.h"
#include "../utils/proj.h"

std::shared_ptr<geos::index::quadtree::Quadtree> build_quadtree(const std::vector<std::unique_ptr<geos::geom::Point>> &points)
{
    ProgressBar progress(points.size());
    progress.start();

    auto index = std::make_shared<geos::index::quadtree::Quadtree>();

    for (int i = 0; i < points.size(); i++)
    {
        index->insert(points[i]->getEnvelopeInternal(), reinterpret_cast<void *>(i));
        progress.update(i);
    }

    progress.finish();
    return index;
}

void run_query(const std::unique_ptr<geos::index::quadtree::Quadtree> index)
{
}

void benchmark_quadtree(const std::vector<std::unique_ptr<geos::geom::Point>> &points)
{
    std::cout << "Running Quadtree benchmark..." << std::endl;

    HeapProfilerStart("heapprofile/quadtree");

    auto index = build_quadtree(points);

    HeapProfilerStop();

    std::cout << "Finished." << std::endl
              << "Running distance queries..." << std::endl;

    // run_query(index);
}
