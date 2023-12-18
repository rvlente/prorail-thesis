#pragma once
#include <tuple>
#include "geos/geom/GeometryFactory.h"

typedef DistanceQuery<std::tuple<double, double>> GeosDistanceQuery;
typedef RangeQuery<geos::geom::Envelope> GeosRangeQuery;
