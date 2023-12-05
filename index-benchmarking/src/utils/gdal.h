#pragma once
#include <iostream>
#include <memory>
#include <functional>
#include <exception>
#include <type_traits>
#include "ogrsf_frmts.h"

class OGRGeometryWrapper
{
public:
    typedef std::unique_ptr<OGRGeometryWrapper> Ptr;

    OGRGeometryWrapper(OGRFeature *feature)
    {
        this->feature = feature;
        this->geometry = feature->GetGeometryRef();
    }

    ~OGRGeometryWrapper()
    {
        OGRFeature::DestroyFeature(feature);
    }

    OGRPoint *get_point()
    {
        if (geometry->getGeometryType() != wkbPoint)
            return nullptr;

        return geometry->toPoint();
    }

    OGRLineString *get_line_string()
    {
        if (geometry->getGeometryType() != wkbLineString)
            return nullptr;

        return geometry->toLineString();
    }

    OGRPolygon *get_polygon()
    {
        if (geometry->getGeometryType() != wkbPolygon)
            return nullptr;

        return geometry->toPolygon();
    }

private:
    OGRFeature *feature;
    OGRGeometry *geometry;
};

class GDALGeometryIterator
{
public:
    static std::unique_ptr<GDALGeometryIterator> fromGPKG(const char *gpkgFile, const char *layerName)
    {
        GDALAllRegister();
        auto dataset = GDALDataset::Open(gpkgFile, GA_ReadOnly);

        if (dataset == nullptr)
        {
            std::cout << "ERROR: Failed to open dataset." << std::endl;
            throw std::runtime_error("Failed to open dataset.");
        }

        return std::make_unique<GDALGeometryIterator>(dataset, layerName);
    }

    bool has_next()
    {
        return (currentFeature = layer->GetNextFeature()) != NULL;
    }

    OGRGeometryWrapper::Ptr next()
    {
        if (currentFeature == NULL && !hasNext())
        {
            return nullptr;
        }

        return std::make_unique<OGRGeometryWrapper>(currentFeature);
    }

private:
    GDALDataset *dataset;
    OGRLayer *layer;
    OGRFeature *currentFeature;

public:
    GDALGeometryIterator(GDALDataset *dataset, const char *layerName) : dataset(dataset)
    {
        this->layer = dataset->GetLayerByName(layerName);
    }

    ~GDALGeometryIterator()
    {
        GDALClose(dataset);
    }
};
