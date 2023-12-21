#pragma once
#include "geos/index/strtree/STRtree.h"
#include "common.h"

class STRtreeExperimentRunner : public GeosIndexExperimentRunner<geos::index::strtree::STRtree>
{
public:
    STRtreeExperimentRunner(std::string name, std::string crs) : GeosIndexExperimentRunner<geos::index::strtree::STRtree>(name, crs) {}

private:
    std::unique_ptr<geos::index::strtree::STRtree> build_index(std::vector<std::unique_ptr<geos::geom::Point>> &geometry, std::function<void(size_t, size_t)> progress)
    {
        auto index = std::make_unique<geos::index::strtree::STRtree>();

        for (int i = 0; i < geometry.size(); i++)
        {
            index->insert(geometry[i]->getEnvelopeInternal(), geometry[i].get());
            progress(i, geometry.size());
        }

        index->build();
        return index;
    }
};