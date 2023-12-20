#pragma once
#include <memory>
#include "s2/s2polygon.h"
#include "s2/s2latlng_rect.h"
#include "s2/s2point.h"
#include "../experiment.h"

typedef DistanceQuery<S2Point> S2DistanceQuery;
typedef RangeQuery<S2LatLngRect> S2RangeQuery;
typedef RangeQuery<std::unique_ptr<S2Polygon>> S2ShapeRangeQuery;
