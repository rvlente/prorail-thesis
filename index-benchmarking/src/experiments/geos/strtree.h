#pragma once
#include <vector>
#include <memory>
#include "geos/index/strtree/STRtree.h"
#include "common.h"
#include "../experiment.h"
#include "../../utils/progress.h"
#include "../../utils/proj.h"
#include "../../utils/data.h"

using namespace geos;

class STRtreeExperimentRunner : public BaseExperimentRunner<index::STRtree, std::unique_ptr<geos::geom::Point>, GeosDistanceQuery, GeosRangeQuery>
{
private:
    ProjWrapper _transformer;
    geos::geom::GeometryFactory::Ptr _factory;

public:
    STRtreeExperimentRunner(const char *name, const char *crs) : _transformer("EPSG:4326", crs), BaseExperimentRunner(name)
    {
        _factory = geom::GeometryFactory.create();
    };

private:
    std::vector<std::unique_ptr<geos::geom::Point>> load_geometry(const char *file_path, std::function<void(size_t, size_t)> progress)
    {
        auto coordinates = load_coordinates(file_path);

        for (int i = 0; i < points.size(); i++)
        {
            auto xy = _transformer.transform(points[i].lat, points[i].lon);
            geos_points.push_back(_factory->createPoint(geos::geom::Coordinate(std::get<0>(xy), std::get<1>(xy))));
            progress(i, points.size());
        }
    }

    std::vector<GeosDistanceQuery> load_distance_queries(const char *file_path, std::function<void(size_t, size_t)> progress)
    {
        auto raw_queries = _load_distance_queries(file_path);

        std::vector<GeosDistanceQuery> queries;

        for (size_t i = 0; i < raw_queries.size(); i++)
        {
            auto q = raw_queries[i];

            queries.push_back({_transformer.transform(q.coord.lat, q.coord.lon),
                               q.distance});

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
            auto xa = std::get<0>(xy);
            auto ya = std::get<1>(xy);

            auto xyb = _transformer.transform(q.b.lat, q.b.lon);
            auto xb = std::get<0>(xy);
            auto yb = std::get<1>(xy);

            queries.push_back({geos::geom::Envelope rectangle(geos::geom::Coordinate(coord.lat - distance, y - distance),
                                                              geos::geom::Coordinate(x + distance, y + distance))});
            progress(i, raw_queries.size());
        }

        return queries;
    }

    std::unique_ptr<S2PointIndex<int>> build_index(const std::vector<S2Point> &geometry, std::function<void(size_t, size_t)> progress)
    {
        auto index = std::make_unique<S2PointIndex<int>>();

        for (size_t i = 0; i < geometry.size(); i++)
        {
            index->Add(geometry[i], i);
            progress(i, geometry.size());
        }

        return index;
    }

    void execute_distance_queries(const index::strtree::STRtree *index, const std::vector<GeosDistanceQuery> &queries, std::function<void(size_t, size_t)> progress)
    {
        std::vector<geos::geom::Geometry *> result;
        result.reserve(1e8);

        for (size_t i = 0; i < queries.size(); i++)
        {
            auto coord = queries[i].coord;
            auto distance = queries[i].distance;
            geos::geom::Envelope rectangle(geos::geom::Coordinate(coord.lat - distance, y - distance),
                                           geos::geom::Coordinate(x + distance, y + distance));

            std::vector<void *> candidates;
            index.query(rectangle->getEnvelopeInternal(), candidates);

            for (auto &candidate : candidates)
            {
                const auto &point = static_cast<geos::geom::Geometry *>(candidate);
                if (point->isWithinDistance(target_point, distance))
                    result.push_back(point);
            }

            uint64_t res_size = result.size();
            result.clear();

            progress(i, n);
        }

        progress.finish();
    }

    void execute_range_queries(const S2PointIndex<int> *index, const std::vector<S2RangeQuery> &queries, std::function<void(size_t, size_t)> progress)
    {
        S2ClosestPointQuery<int> query(index);

        for (size_t i = 0; i < queries.size(); i++)
        {
            S2ClosestPointQueryPointTarget target(queries[i].range.GetCenter().ToPoint());

            query.mutable_options()->set_region(&queries[i].range);
            query.FindClosestPoints(&target);

            progress(i, queries.size());
        }
    }
};