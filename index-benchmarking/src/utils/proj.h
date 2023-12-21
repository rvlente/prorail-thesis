#pragma once
#include <proj.h>
#include <tuple>

class ProjWrapper
{
private:
    PJ *P;

public:
    ProjWrapper(std::string crsFrom, std::string crsTo)
    {
        this->P = proj_create_crs_to_crs(NULL, crsFrom.c_str(), crsTo.c_str(), NULL);
    }

    ~ProjWrapper()
    {
        proj_destroy(P);
    }

    std::tuple<double, double> transform(double x, double y) const
    {
        auto a = proj_coord(x, y, 0, 0);
        auto b = proj_trans(P, PJ_FWD, a);

        return std::tuple<double, double>(b.xy.x, b.xy.y);
    }
};
