#include <chrono>
#include <memory>
#include <tuple>
#include <iostream>
#include "s2/pointindex.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " <data_file> <dq_file> <rq_file>" << std::endl;
        return 1;
    }

    const char *data_file = argv[1];
    const char *distance_query_file = argv[2];
    const char *range_query_file = argv[3];

    auto s2runner = S2PointIndexRunner("s2pointindex");
    s2runner.run("nyc-taxi-30m", data_file, distance_query_file, range_query_file);

    return 0;
}
