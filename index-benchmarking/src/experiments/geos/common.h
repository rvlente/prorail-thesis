#pragma once
#include <vector>
#include <memory>
#include "geos/index/SpatialIndex.h"
#include "geos/geom/GeometryFactory.h"
#include "geos/geom/Envelope.h"
#include "common.h"
#include "../experiment.h"
#include "../../utils/progress.h"
#include "../../utils/proj.h"
#include "../../utils/data.h"

typedef DistanceQuery<std::unique_ptr<geos::geom::Point>> GeosDistanceQuery;
typedef RangeQuery<geos::geom::Envelope> GeosRangeQuery;

template <typename TIndex>
class GeosIndexExperimentRunner : public BaseExperimentRunner<TIndex, std::unique_ptr<geos::geom::Point>, GeosDistanceQuery, GeosRangeQuery>
{
private:
    ProjWrapper _transformer;
    geos::geom::GeometryFactory::Ptr _factory;

public:
    GeosIndexExperimentRunner(std::string name, std::string crs, std::string executable_name) : BaseExperimentRunner<TIndex, std::unique_ptr<geos::geom::Point>, GeosDistanceQuery, GeosRangeQuery>(name, executable_name), _transformer("EPSG:4326", crs)
    {
        _factory = geos::geom::GeometryFactory::create();
    };

private:
    std::vector<std::unique_ptr<geos::geom::Point>> load_geometry(std::string file_path, std::function<void(size_t, size_t)> progress)
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

    std::vector<GeosDistanceQuery> load_distance_queries(std::string file_path, std::function<void(size_t, size_t)> progress)
    {
        auto raw_queries = _load_distance_queries(file_path);

        std::vector<GeosDistanceQuery> queries;

        for (size_t i = 0; i < raw_queries.size(); i++)
        {
            auto q = raw_queries[i];
            auto xy = _transformer.transform(q.coord.lat, q.coord.lon);

            queries.push_back({_factory->createPoint(geos::geom::Coordinate(std::get<0>(xy), std::get<1>(xy))), q.distance});

            progress(i, raw_queries.size());
        }

        return queries;
    }

    std::vector<GeosRangeQuery> load_range_queries(std::string file_path, std::function<void(size_t, size_t)> progress)
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

    void execute_distance_queries(TIndex *index, std::vector<GeosDistanceQuery> &queries, std::function<void(size_t, size_t)> progress)
    {
        // Run for at most 2 minutes to prevent excessive compute usage. This should be enough to get an approximate throughput.
        int max_seconds = 2 * 60;
        auto start_time = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < queries.size(); i++)
        {
            auto target_point = queries[i].point.get();
            auto distance = queries[i].distance;

            geos::geom::Envelope rectangle(geos::geom::Coordinate(target_point->getX() - distance, target_point->getY() - distance),
                                           geos::geom::Coordinate(target_point->getX() + distance, target_point->getY() + distance));

            std::vector<geos::geom::Point *> result;
            std::vector<void *> candidates;

            index->query(&rectangle, candidates);

            for (const auto &candidate : candidates)
            {
                auto point = static_cast<geos::geom::Point *>(candidate);
                if (point->isWithinDistance(target_point, distance))
                {
                    result.push_back(point);
                }
            }

            auto current_time = std::chrono::high_resolution_clock::now();
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

            progress(i, queries.size());

            if (seconds >= max_seconds)
            {
                break;
            }
        }
    }

    void execute_range_queries(TIndex *index, std::vector<GeosRangeQuery> &queries, std::function<void(size_t, size_t)> progress)
    {
        std::vector<geos::geom::Geometry *> result;
        result.reserve(1e8);

        // Run for at most 2 minutes to prevent excessive compute usage. This should be enough to get an approximate throughput.
        int max_seconds = 2 * 60;
        auto start_time = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < queries.size(); i++)
        {
            std::vector<geos::geom::Point *> result;
            std::vector<void *> candidates;

            index->query(&queries[i].range, candidates);
            auto target_range = _factory->toGeometry(&queries[i].range);

            for (const auto &candidate : candidates)
            {
                auto point = static_cast<geos::geom::Point *>(candidate);
                if (point->within(target_range.get()))
                {
                    result.push_back(point);
                }
            }

            auto current_time = std::chrono::high_resolution_clock::now();
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

            progress(i, queries.size());

            if (seconds >= max_seconds)
            {
                break;
            }
        }
    }
};