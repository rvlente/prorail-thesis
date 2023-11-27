#pragma once
#include <proj.h>
#include <tuple>

class ProjWrapper
{
private:
    PJ *P;

public:
    ProjWrapper(const char *crsFrom, const char *crsTo)
    {
        this->P = proj_create_crs_to_crs(NULL, crsFrom, crsTo, NULL);
    }

    ~ProjWrapper()
    {
        proj_destroy(P);
    }

    std::tuple<double, double> transform(std::tuple<double, double> c) const
    {
        double x, y;
        std::tie(x, y) = c;

        auto a = proj_coord(x, y, 0, 0);
        auto b = proj_trans(P, PJ_FWD, a);

        return std::tuple<double, double>(b.xy.x, b.xy.y);
    }
};
