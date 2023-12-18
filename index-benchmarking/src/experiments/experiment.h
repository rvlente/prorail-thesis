#pragma once
#include <vector>
#include <future>
#include <thread>
#include <atomic>
#include <gperftools/heap-profiler.h>
#include <s2/s2point_index.h>
#include <s2/s2point.h>
#include "../utils/progress.h"
#include "../utils/data.h"

template <typename TPoint>
struct DistanceQuery
{
    TPoint point;
    double distance;
};

template <typename TRect>
struct RangeQuery
{
    TRect range;
};

template <typename TIndex, typename TGeom, typename TDQuery, typename TRQuery>
class BaseExperimentRunner
{
private:
    std::string _name;

public:
    BaseExperimentRunner(const char *name) : _name(name){};

    virtual std::vector<TGeom> load_geometry(const char *file_path, std::function<void(size_t, size_t)> progress) = 0;
    virtual std::vector<TDQuery> load_distance_queries(const char *file_path, std::function<void(size_t, size_t)> progress) = 0;
    virtual std::vector<TRQuery> load_range_queries(const char *file_path, std::function<void(size_t, size_t)> progress) = 0;

    virtual std::unique_ptr<TIndex> build_index(const std::vector<TGeom> &geometry, std::function<void(size_t, size_t)> progress) = 0;
    virtual void execute_distance_queries(const TIndex *index, const std::vector<TDQuery> &queries, std::function<void(size_t, size_t)> progress) = 0;
    virtual void execute_range_queries(const TIndex *index, const std::vector<TRQuery> &queries, std::function<void(size_t, size_t)> progress) = 0;

    void run(const char *run_name, const char *geom_file, const char *dquery_file, const char *rquery_file)
    {
        std::string full_name = _name + '_' + run_name;

        std::cout << std::string(full_name.length() + 4, '=') << std::endl
                  << "= " << full_name << " =" << std::endl
                  << std::string(full_name.length() + 4, '=') << std::endl
                  << std::endl;

        ProgressBar pb;

        std::cout << "Loading geometry... " << std::endl;

        pb.start();
        auto geometry = load_geometry(geom_file, pb.bind());
        pb.stop();

        std::cout << "Done. Loaded " << geometry.size() << " objects." << std::endl;

        std::cout << "Loading distance queries... " << std::endl;

        pb.start();
        auto dqueries = load_distance_queries(dquery_file, pb.bind());
        pb.stop();

        std::cout << "Loading range queries... " << std::endl;
        pb.start();
        auto rqueries = load_range_queries(rquery_file, pb.bind());
        pb.stop();

        std::cout << "Done. Loaded " << dqueries.size() << "/" << rqueries.size() << " distance/range queries." << std::endl;

        std::cout << "Building index..." << std::endl;

        std::string hp_name = "heapprofile/"; // this causes the dumps to be written to the heapprofile/ folder.
        hp_name += _name;

        HeapProfilerStart(hp_name.c_str());

        pb.start();
        auto index = build_index(geometry, pb.bind());
        pb.stop();

        HeapProfilerStop();

        std::cout << "Done. Executing distance queries..." << std::endl;

        pb.start();
        execute_distance_queries(index.get(), dqueries, pb.bind());
        pb.stop();

        std::cout << "Executing range queries..." << std::endl;

        pb.start();
        execute_range_queries(index.get(), rqueries, pb.bind());
        pb.stop();

        std::cout << "Done." << std::endl;
    }
};