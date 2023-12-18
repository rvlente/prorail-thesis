#pragma once
#include <tuple>
#include <memory>
#include "geos/geom/Envelope.h"
#include "../experiment.h"

typedef DistanceQuery<std::unique_ptr<geos::geom::Point>> GeosDistanceQuery;
typedef RangeQuery<geos::geom::Envelope> GeosRangeQuery;
