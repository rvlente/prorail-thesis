#pragma once
#include "geos/index/quadtree/Quadtree.h"
#include "common.h"

class QuadtreeExperimentRunner : public GeosIndexExperimentRunner<geos::index::quadtree::Quadtree>
{
public:
    QuadtreeExperimentRunner(std::string name, std::string crs, std::string executable_name) : GeosIndexExperimentRunner<geos::index::quadtree::Quadtree>(name, crs, executable_name) {}

private:
    std::unique_ptr<geos::index::quadtree::Quadtree> build_index(std::vector<std::unique_ptr<geos::geom::Point>> &geometry, std::function<void(size_t, size_t)> progress)
    {
        auto index = std::make_unique<geos::index::quadtree::Quadtree>();

        for (int i = 0; i < geometry.size(); i++)
        {
            index->insert(geometry[i]->getEnvelopeInternal(), geometry[i].get());
            progress(i, geometry.size());
        }

        return index;
    }
};