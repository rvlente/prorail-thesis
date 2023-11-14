#pragma once
#include <iostream>
#include <functional>
#include "shapefil.h"

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

ulong processShapes(const char *shapeFile, std::function<void(const SHPObject *, const ShapeType)> processFunc)
{
    // Read shapefile.
    SHPHandle handle = SHPOpen(shapeFile, "rb");
    if (handle == NULL)
    {
        std::cout << "Failed to load shapefile! Exiting." << std::endl;
        return 1;
    }

    int nEntities, rawShapeType;
    SHPGetInfo(handle, &nEntities, &rawShapeType, NULL, NULL);
    ShapeType shapeType = toShapeType(rawShapeType);

    // Read objects.
    for (int i = 0; i < nEntities; i++)
    {
        SHPObject *obj = SHPReadObject(handle, i);
        if (obj == NULL)
        {
            std::cout << "Failed to read object " << i << "! Exiting." << std::endl;
            break;
        }

        processFunc(obj, shapeType);
        SHPDestroyObject(obj);
    }

    // Close handle.
    SHPClose(handle);

    // Return.
    return 0;
}
