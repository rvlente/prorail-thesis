// #pragma once
// #include <vector>
// #include "s2/s2point.h"
// #include "s2/s2polygon.h"
// #include "s2/s2point_vector_shape.h"
// #include "s2/s2closest_edge_query.h"
// #include "s2/mutable_s2shape_index.h"
// #include "s2/s2contains_point_query.h"
// #include "s2/s2boolean_operation.h"
// #include "s2/s2earth.h"
// #include "common.h"
// #include "../experiment.h"

// class S2ShapeIndexExperimentRunner : public BaseExperimentRunner<MutableS2ShapeIndex, std::unique_ptr<S2PointVectorShape>, S2DistanceQuery, S2ShapeRangeQuery>
// {
// public:
//     S2ShapeIndexExperimentRunner(std::string name) : BaseExperimentRunner(name){};

// private:
//     std::vector<std::unique_ptr<S2PointVectorShape>> load_geometry(std::string file_path, std::function<void(size_t, size_t)> progress)
//     {
//         auto coordinates = load_coordinates(file_path);
//         std::vector<S2Point> s2_points;

//         for (size_t i = 0; i < coordinates.size(); i++)
//         {
//             auto coordinate = coordinates[i];
//             s2_points.push_back(S2LatLng::FromDegrees(coordinate.lat, coordinate.lon).ToPoint());
//             progress(i, coordinates.size());
//         }

//         std::vector<std::unique_ptr<S2PointVectorShape>> result;
//         auto pvs = std::make_unique<S2PointVectorShape>(s2_points);
//         result.push_back(std::move(pvs));
//         return result;
//     }

//     std::vector<S2DistanceQuery> load_distance_queries(std::string file_path, std::function<void(size_t, size_t)> progress)
//     {
//         auto raw_queries = _load_distance_queries(file_path);

//         std::vector<S2DistanceQuery> queries;

//         for (size_t i = 0; i < raw_queries.size(); i++)
//         {
//             auto q = raw_queries[i];
//             queries.push_back({S2LatLng::FromDegrees(q.coord.lat, q.coord.lon).ToPoint(), q.distance});
//             progress(i, raw_queries.size());
//         }

//         return queries;
//     }

//     std::vector<S2ShapeRangeQuery> load_range_queries(std::string file_path, std::function<void(size_t, size_t)> progress)
//     {
//         auto raw_queries = _load_range_queries(file_path);

//         std::vector<S2ShapeRangeQuery> queries;

//         for (size_t i = 0; i < raw_queries.size(); i++)
//         {
//             auto q = raw_queries[i];
//             std::vector<S2Point> points = {S2LatLng::FromDegrees(q.a.lat, q.a.lon).ToPoint(),
//                                            S2LatLng::FromDegrees(q.b.lat, q.a.lon).ToPoint(),
//                                            S2LatLng::FromDegrees(q.b.lat, q.b.lon).ToPoint(),
//                                            S2LatLng::FromDegrees(q.a.lat, q.b.lon).ToPoint()};

//             queries.push_back({std::make_unique<S2Polygon>(std::make_unique<S2Loop>(points))});
//             progress(i, raw_queries.size());
//         }

//         return queries;
//     }

//     std::unique_ptr<MutableS2ShapeIndex> build_index(std::vector<std::unique_ptr<S2PointVectorShape>> &geometry, std::function<void(size_t, size_t)> progress)
//     {
//         auto index = std::make_unique<MutableS2ShapeIndex>();

//         for (size_t i = 0; i < geometry.size(); i++)
//         {
//             index->Add(std::move(geometry[i]));
//             progress(i, geometry.size());
//         }

//         index->ForceBuild();

//         return index;
//     }

//     void execute_distance_queries(MutableS2ShapeIndex *index, std::vector<S2DistanceQuery> &queries, std::function<void(size_t, size_t)> progress)
//     {
//         S2ClosestEdgeQuery query(index);

//         for (size_t i = 0; i < queries.size(); i++)
//         {
//             auto point = queries[i].point;
//             S2ClosestEdgeQuery::PointTarget target(point);

//             query.mutable_options()->set_max_distance(S2Earth::ToAngle(util::units::Meters(queries[i].distance)));
//             query.FindClosestEdges(&target);

//             progress(i, queries.size());
//         }
//     }

//     void execute_range_queries(MutableS2ShapeIndex *index, std::vector<S2ShapeRangeQuery> &queries, std::function<void(size_t, size_t)> progress)
//     {
//         S2ClosestEdgeQuery query(index);

//         for (size_t i = 0; i < queries.size(); i++)
//         {
//             MutableS2ShapeIndex si;
//             si.Add(std::make_unique<S2Polygon::Shape>(queries[i].range.get()));
//             S2ClosestEdgeQuery::ShapeIndexTarget target(&si);

//             query.mutable_options()->set_max_distance(S2Earth::ToAngle(util::units::Meters(0)));
//             query.FindClosestEdges(&target);

//             progress(i, queries.size());
//         }
//     }
// };