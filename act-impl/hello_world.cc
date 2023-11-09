#include <iostream>
#include <string>
#include <vector>
#include "absl/strings/str_join.h"
#include "s2/s2point.h"
#include "s2/s2latlng.h"

int main()
{
    S2Point point(S2LatLng::FromDegrees(0.0, 0.0));

    std::cout << "Does this work? " << point.Size() << "\n";
}
