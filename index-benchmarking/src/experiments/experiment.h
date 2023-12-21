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
    const std::string _name;
    const Coord _translation;

public:
    BaseExperimentRunner(std::string name, Coord translation = {0, 0}) : _name(name), _translation(translation){};

    virtual std::vector<TGeom> load_geometry(std::string file_path, const Coord &translation, std::function<void(size_t, size_t)> progress) = 0;
    virtual std::vector<TDQuery> load_distance_queries(std::string file_path, const Coord &translation, std::function<void(size_t, size_t)> progress) = 0;
    virtual std::vector<TRQuery> load_range_queries(std::string file_path, const Coord &translation, std::function<void(size_t, size_t)> progress) = 0;

    virtual std::unique_ptr<TIndex> build_index(std::vector<TGeom> &geometry, std::function<void(size_t, size_t)> progress) = 0;
    virtual void execute_distance_queries(TIndex *index, std::vector<TDQuery> &queries, std::function<void(size_t, size_t)> progress) = 0;
    virtual void execute_range_queries(TIndex *index, std::vector<TRQuery> &queries, std::function<void(size_t, size_t)> progress) = 0;

    void run(std::string run_name, std::string geom_file, std::string dquery_file, std::string rquery_file, std::string argv0)
    {
        std::string full_name = _name + '_' + run_name;

        std::cout << std::string(full_name.length() + 4, '=') << std::endl
                  << "= " << full_name << " =" << std::endl
                  << std::string(full_name.length() + 4, '=') << std::endl
                  << std::endl;

        std::cout << "Loading geometry..." << std::endl;

        ProgressTracker pt_load_geometry;
        auto geometry = load_geometry(geom_file, _translation, pt_load_geometry.bind());
        pt_load_geometry.stop();

        std::cout << "Done. Loaded " << geometry.size() << " objects." << std::endl;

        std::cout << "Loading distance queries... " << std::endl;

        ProgressTracker pt_load_distance_queries;
        auto dqueries = load_distance_queries(dquery_file, _translation, pt_load_distance_queries.bind());
        pt_load_distance_queries.stop();

        std::cout << "Loading range queries... " << std::endl;

        ProgressTracker pt_load_range_queries;
        auto rqueries = load_range_queries(rquery_file, _translation, pt_load_range_queries.bind());
        pt_load_range_queries.stop();

        std::cout << "Done. Loaded " << dqueries.size() << "/" << rqueries.size() << " distance/range queries." << std::endl;

        std::cout << "Building index..." << std::endl;

        auto hp_name = "tmp/" + full_name; // this causes the dumps to be written to the tmp/ folder.
        HeapProfilerStart(hp_name.c_str());

        ProgressTracker pt_build_index;
        auto index = build_index(geometry, pt_build_index.bind());
        pt_build_index.stop();

        HeapProfilerDump("done");
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
                      << "tmp/$(ls tmp | grep " << full_name << " | tail -1)" // last dump in folder
                      << " | grep ::build_index"                              // extract relevant function
                      << " | awk -F ' +' '{ print $5 }'";                     // extract memory allocation field

        auto index_size = exec(pprof_command.str().c_str());
        index_size.pop_back(); // remove new-line at the end

        std::ofstream file;
        file.open("results/" + full_name + ".txt");

        file << "run_name          | " << full_name << std::endl
             << "geometry_file     | " << geom_file << std::endl
             << "n_geometries      | " << geometry.size() << std::endl
             << "dquery_file       | " << dquery_file << std::endl
             << "n_dqueries        | " << dqueries.size() << std::endl
             << "rquery_file       | " << rquery_file << std::endl
             << "n_rqueries        | " << rqueries.size() << std::endl
             << "index_size        | " << index_size << " MB" << std::endl
             << "build_time        | " << pt_build_index.get_time() << " hh:mm:ss" << std::endl
             << "dquery_throughput | " << pt_execute_distance_queries.get_throughput() << " queries/s" << std::endl
             << "rquery_throughput | " << pt_execute_range_queries.get_throughput() << " queries/s" << std::endl;

        file.close();

        std::cout << "Report written to " << full_name << ".txt." << std::endl;
    }
};