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
#include "../utils/exec.h"

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
    virtual void execute_distance_queries(TIndex *index, const std::vector<TDQuery> &queries, std::function<void(size_t, size_t)> progress) = 0;
    virtual void execute_range_queries(TIndex *index, const std::vector<TRQuery> &queries, std::function<void(size_t, size_t)> progress) = 0;

    void run(const char *run_name, const char *geom_file, const char *dquery_file, const char *rquery_file, const char *argv0)
    {
        std::string full_name = _name + '_' + run_name;

        std::cout << std::string(full_name.length() + 4, '=') << std::endl
                  << "= " << full_name << " =" << std::endl
                  << std::string(full_name.length() + 4, '=') << std::endl
                  << std::endl;

        std::cout << "Loading geometry..." << std::endl;

        ProgressTracker pt_load_geometry;
        auto geometry = load_geometry(geom_file, pt_load_geometry.bind());
        pt_load_geometry.stop();

        std::cout << "Done. Loaded " << geometry.size() << " objects." << std::endl;

        std::cout << "Loading distance queries... " << std::endl;

        ProgressTracker pt_load_distance_queries;
        auto dqueries = load_distance_queries(dquery_file, pt_load_distance_queries.bind());
        pt_load_distance_queries.stop();

        std::cout << "Loading range queries... " << std::endl;

        ProgressTracker pt_load_range_queries;
        auto rqueries = load_range_queries(rquery_file, pt_load_range_queries.bind());
        pt_load_range_queries.stop();

        std::cout << "Done. Loaded " << dqueries.size() << "/" << rqueries.size() << " distance/range queries." << std::endl;

        std::cout << "Building index..." << std::endl;

        auto hp_name = "tmp/" + full_name; // this causes the dumps to be written to the tmp/ folder.
        HeapProfilerStart(hp_name.c_str());

        ProgressTracker pt_build_index;
        auto index = build_index(geometry, pt_build_index.bind());
        pt_build_index.stop();

        HeapProfilerStop();

        std::cout << "Done. Executing distance queries..." << std::endl;

        ProgressTracker pt_execute_distance_queries;
        execute_distance_queries(index.get(), dqueries, pt_execute_distance_queries.bind());
        pt_execute_distance_queries.stop();

        std::cout << "Executing range queries..." << std::endl;

        ProgressTracker pt_execute_range_queries;
        execute_range_queries(index.get(), rqueries, pt_execute_range_queries.bind());
        pt_execute_range_queries.stop();

        std::cout << "Done. Compiling report..." << std::endl;

        // Parse heapprofile using bash.
        std::stringstream pprof_command;
        pprof_command << "pprof "
                      << "--text "
                      << "--inuse_space "
                      << argv0 << " "
                      << "heapprofile/$(ls heapprofile | grep " << _name << " | tail -1)" // last dump in folder
                      << " | grep ::build_index"                                          // extract relevant function
                      << " | awk -F ' +' '{ print $4 }'";                                 // extract memory allocation field

        auto index_size = exec(pprof_command.str().c_str());

        std::ofstream file;
        file.open(full_name + ".txt");

        file << "Run name: " << full_name << std::endl
             << "Geometry file: " << geom_file << std::endl
             << "N geometries: " << geometry.size() << std::endl
             << "Distance query file: " << dquery_file << std::endl
             << "N dqueries: " << dqueries.size() << std::endl
             << "Range query file: " << rquery_file << std::endl
             << "N rqueries: " << rqueries.size() << std::endl
             << "Index size: " << index_size << std::endl
             << "Index build time: " << pt_build_index.get_time() << std::endl
             << "Distance query throughput: " << pt_execute_distance_queries.get_throughput() << std::endl
             << "Range query throughput: " << pt_execute_range_queries.get_throughput() << std::endl;

        file.close();

        std::cout << "Report written to " << report_file << "." << std::endl;
    }
};