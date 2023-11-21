#pragma once
#include <iostream>
#include <memory>
#include <functional>
#include <exception>
#include "shapefil.h"

typedef std::unique_ptr<SHPObject, decltype(&SHPDestroyObject)> SHPObject_ptr;

enum ShapeType
{
    Point = SHPT_POINT,
    Line = SHPT_ARC,
    Polygon = SHPT_POLYGON
};

ShapeType toShapeType(int shapeType)
{
    switch (shapeType)
    {
    case SHPT_POINT:
        return ShapeType::Point;
    case SHPT_ARC:
        return ShapeType::Line;
    case SHPT_POLYGON:
        return ShapeType::Polygon;
    default:
        return ShapeType::Point;
    }
}

std::vector<SHPObject_ptr> loadShapefile(const char *shapeFile, ShapeType *shapeType)
{
    std::vector<SHPObject_ptr> shapes;

    // Read shapefile.
    SHPHandle handle = SHPOpen(shapeFile, "rb");
    if (handle == NULL)
    {
        return shapes;
    }

    int nEntities, rawShapeType;
    SHPGetInfo(handle, &nEntities, &rawShapeType, NULL, NULL);
    *shapeType = toShapeType(rawShapeType);

    // Read objects.
    for (int i = 0; i < nEntities; i++)
    {
        SHPObject *obj = SHPReadObject(handle, i);
        if (obj == NULL)
        {
            std::cout << "ERROR: Failed to read object " << i << "." << std::endl;
            continue;
        }

        shapes.push_back(SHPObject_ptr(obj, SHPDestroyObject));
    }

    // Close handle.
    SHPClose(handle);
    return shapes;
}
