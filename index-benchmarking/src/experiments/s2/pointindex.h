#pragma once
#include <vector>
#include "s2/s2point.h"
#include "s2/s2point_index.h"
#include "s2/s2closest_point_query.h"
#include "s2/s2earth.h"
#include "common.h"
#include "../experiment.h"

class S2PointIndexExperimentRunner : public BaseExperimentRunner<S2PointIndex<int>, S2Point, DistanceQuery<S2Point>, S2RangeQuery>
{
public:
    S2PointIndexExperimentRunner(std::string name, std::string executable_name) : BaseExperimentRunner(name, executable_name){};

private:
    std::vector<S2Point> load_geometry(std::string file_path, std::function<void(size_t, size_t)> progress)
    {
        auto coordinates = load_coordinates(file_path);
        std::vector<S2Point> s2_points;

        for (size_t i = 0; i < coordinates.size(); i++)
        {
            auto coordinate = coordinates[i];
            s2_points.push_back(S2LatLng::FromDegrees(coordinate.lat, coordinate.lon).ToPoint());
            progress(i, coordinates.size());
        }

        return s2_points;
    }

    std::vector<S2DistanceQuery> load_distance_queries(std::string file_path, std::function<void(size_t, size_t)> progress)
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

    std::vector<S2RangeQuery> load_range_queries(std::string file_path, std::function<void(size_t, size_t)> progress)
    {
        auto raw_queries = _load_range_queries(file_path);

        std::vector<S2RangeQuery> queries;

        for (size_t i = 0; i < raw_queries.size(); i++)
        {
            auto q = raw_queries[i];
            queries.push_back({S2LatLngRect(
                S2LatLng::FromDegrees(q.a.lat, q.a.lon),
                S2LatLng::FromDegrees(q.b.lat, q.b.lon))});
            progress(i, raw_queries.size());
        }

        return queries;
    }

    std::unique_ptr<S2PointIndex<int>> build_index(std::vector<S2Point> &geometry, std::function<void(size_t, size_t)> progress)
    {
        auto index = std::make_unique<S2PointIndex<int>>();

        for (size_t i = 0; i < geometry.size(); i++)
        {
            index->Add(geometry[i], i);
            progress(i, geometry.size());
        }

        return index;
    }

    void execute_distance_queries(S2PointIndex<int> *index, std::vector<S2DistanceQuery> &queries, std::function<void(size_t, size_t)> progress)
    {
        S2ClosestPointQuery<int> query(index);

        // Run for at most 2 minutes to prevent excessive compute usage. This should be enough to get an approximate throughput.
        int max_seconds = 2 * 60;
        auto start_time = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < queries.size(); i++)
        {
            S2ClosestPointQueryPointTarget target(queries[i].point);

            query.mutable_options()->set_max_distance(S2Earth::ToAngle(util::units::Meters(queries[i].distance)));
            query.FindClosestPoints(&target);

            auto current_time = std::chrono::high_resolution_clock::now();
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

            progress(i, queries.size());

            if (seconds >= max_seconds)
            {
                break;
            }
        }
    }

    void execute_range_queries(S2PointIndex<int> *index, std::vector<S2RangeQuery> &queries, std::function<void(size_t, size_t)> progress)
    {
        S2ClosestPointQuery<int> query(index);

        // Run for at most 2 minutes to prevent excessive compute usage. This should be enough to get an approximate throughput.
        int max_seconds = 2 * 60;
        auto start_time = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < queries.size(); i++)
        {
            S2ClosestPointQueryPointTarget target(queries[i].range.GetCenter().ToPoint());

            query.mutable_options()->set_region(&queries[i].range);
            auto result = query.FindClosestPoints(&target);

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