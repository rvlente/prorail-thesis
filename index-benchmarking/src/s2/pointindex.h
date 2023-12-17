#pragma once
#include <vector>
#include "s2/s2point.h"
#include "s2/s2point_index.h"
#include "s2/s2closest_point_query.h"
#include "s2/s2earth.h"
#include "../utils/experiment.h"

typedef DistanceQuery<S2Point> S2DistanceQuery;
typedef RangeQuery<S2LatLngRect> S2RangeQuery;

class S2PointIndexRunner : public BaseExperimentRunner<S2PointIndex<int>, S2Point, S2DistanceQuery, S2RangeQuery>
{
public:
    S2PointIndexRunner(const char *name) : BaseExperimentRunner(name){};

private:
    std::vector<S2Point> load_geometry(const char *file_path, std::function<void(size_t, size_t)> progress)
    {
        auto coordinates = load_coordinates(file_path);
        std::vector<S2Point> s2_points;

        for (int i = 0; i < coordinates.size(); i++)
        {
            auto coordinate = coordinates[i];
            s2_points.push_back(S2LatLng::FromDegrees(coordinate.lat, coordinate.lon).ToPoint());
            progress(i, coordinates.size());
        }

        return s2_points;
    }

    std::vector<S2DistanceQuery> load_distance_queries(const char *file_path, std::function<void(size_t, size_t)> progress)
    {
        auto raw_queries = _load_distance_queries(file_path);

        std::vector<S2DistanceQuery> queries;

        for (size_t i = 0; i < raw_queries.size(); i++)
        {
            auto q = raw_queries[i];
            queries.push_back({S2LatLng::FromDegrees(q.coord.lat, q.coord.lon).ToPoint(), q.distance});
            progress(i, raw_queries.size());
        }

        return queries;
    }

    std::vector<S2RangeQuery> load_range_queries(const char *file_path, std::function<void(size_t, size_t)> progress)
    {
        auto raw_queries = _load_range_queries(file_path);

        std::vector<S2RangeQuery> queries;

        for (size_t i = 0; i < raw_queries.size(); i++)
        {
            auto q = raw_queries[i];
            queries.push_back({S2LatLngRect(S2LatLng::FromDegrees(q.a.lat, q.a.lon), S2LatLng::FromDegrees(q.b.lat, q.b.lon))});
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

    void execute_distance_queries(const S2PointIndex<int> *index, const std::vector<S2DistanceQuery> &queries, std::function<void(size_t, size_t)> progress)
    {
        S2ClosestPointQuery<int> query(index);

        for (size_t i = 0; i < queries.size(); i++)
        {
            S2ClosestPointQueryPointTarget target(queries[i].point);

            query.mutable_options()->set_max_distance(S2Earth::ToAngle(util::units::Meters(queries[i].distance)));
            query.FindClosestPoints(&target);

            progress(i, queries.size());
        }
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