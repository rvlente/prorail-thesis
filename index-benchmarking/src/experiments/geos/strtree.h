#pragma once
#include <vector>
#include <memory>
#include "geos/geom/GeometryFactory.h"
#include "geos/index/strtree/STRtree.h"
#include "common.h"
#include "../experiment.h"
#include "../../utils/progress.h"
#include "../../utils/proj.h"
#include "../../utils/data.h"

class STRtreeExperimentRunner : public BaseExperimentRunner<geos::index::strtree::STRtree, std::unique_ptr<geos::geom::Point>, GeosDistanceQuery, GeosRangeQuery>
{
private:
    ProjWrapper _transformer;
    geos::geom::GeometryFactory::Ptr _factory;

public:
    STRtreeExperimentRunner(const char *name, const char *crs) : BaseExperimentRunner(name), _transformer("EPSG:4326", crs)
    {
        _factory = geos::geom::GeometryFactory::create();
    };

private:
    std::vector<std::unique_ptr<geos::geom::Point>> load_geometry(const char *file_path, std::function<void(size_t, size_t)> progress)
    {
        auto coordinates = load_coordinates(file_path);
        std::vector<std::unique_ptr<geos::geom::Point>> geos_points;

        for (size_t i = 0; i < coordinates.size(); i++)
        {
            auto latlon = coordinates[i];
            auto xy = _transformer.transform(latlon.lat, latlon.lon);
            geos_points.push_back(_factory->createPoint(geos::geom::Coordinate(std::get<0>(xy), std::get<1>(xy))));
            progress(i, coordinates.size());
        }

        return geos_points;
    }

    std::vector<GeosDistanceQuery> load_distance_queries(const char *file_path, std::function<void(size_t, size_t)> progress)
    {
        auto raw_queries = _load_distance_queries(file_path);

        std::vector<GeosDistanceQuery> queries;

        for (size_t i = 0; i < raw_queries.size(); i++)
        {
            auto q = raw_queries[i];
            auto xy = _transformer.transform(q.coord.lat, q.coord.lon);
            auto point = _factory->createPoint(geos::geom::Coordinate(std::get<0>(xy), std::get<1>(xy)));

            queries.push_back({std::move(point), q.distance});

            progress(i, raw_queries.size());
        }

        return queries;
    }

    std::vector<GeosRangeQuery> load_range_queries(const char *file_path, std::function<void(size_t, size_t)> progress)
    {
        auto raw_queries = _load_range_queries(file_path);

        std::vector<GeosRangeQuery> queries;

        for (size_t i = 0; i < raw_queries.size(); i++)
        {
            auto q = raw_queries[i];

            auto xya = _transformer.transform(q.a.lat, q.a.lon);
            auto xa = std::get<0>(xya);
            auto ya = std::get<1>(xya);

            auto xyb = _transformer.transform(q.b.lat, q.b.lon);
            auto xb = std::get<0>(xyb);
            auto yb = std::get<1>(xyb);

            queries.push_back({geos::geom::Envelope(geos::geom::Coordinate(xa, ya),
                                                    geos::geom::Coordinate(xb, yb))});
            progress(i, raw_queries.size());
        }

        return queries;
    }

    std::unique_ptr<geos::index::strtree::STRtree> build_index(const std::vector<std::unique_ptr<geos::geom::Point>> &geometry, std::function<void(size_t, size_t)> progress)
    {
        auto index = std::make_unique<geos::index::strtree::STRtree>();

        for (int i = 0; i < geometry.size(); i++)
        {
            index->insert(geometry[i]->getEnvelopeInternal(), nullptr);
            progress(i, geometry.size());
        }

        return index;
    }

    void execute_distance_queries(geos::index::strtree::STRtree *index, const std::vector<GeosDistanceQuery> &queries, std::function<void(size_t, size_t)> progress)
    {
        std::vector<geos::geom::Geometry *> result;
        result.reserve(1e8);

        for (size_t i = 0; i < queries.size(); i++)
        {
            auto target_point = queries[i].point.get();
            auto distance = queries[i].distance;

            geos::geom::Envelope rectangle(geos::geom::Coordinate(target_point->getX() - distance, target_point->getY() - distance),
                                           geos::geom::Coordinate(target_point->getX() + distance, target_point->getY() + distance));

            std::vector<void *> candidates;
            index->query(&rectangle, candidates);

            for (auto &candidate : candidates)
            {
                const auto &point = static_cast<geos::geom::Geometry *>(candidate);
                if (point->isWithinDistance(target_point, distance))
                    result.push_back(point);
            }

            uint64_t res_size = result.size();
            result.clear();

            progress(i, queries.size());
        }
    }

    void execute_range_queries(geos::index::strtree::STRtree *index, const std::vector<GeosRangeQuery> &queries, std::function<void(size_t, size_t)> progress)
    {
        std::vector<geos::geom::Geometry *> result;
        result.reserve(1e8);

        for (size_t i = 0; i < queries.size(); i++)
        {
            std::vector<void *> result;
            index->query(&queries[i].range, result);

            uint64_t res_size = result.size();
            result.clear();

            progress(i, queries.size());
        }
    }
};