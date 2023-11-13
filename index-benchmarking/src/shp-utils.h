#include "shapefil.h"

int read_shapefile()
{
    SHPHandle hSHP;
    hSHP = SHPOpen("../../notebooks/shape/lines/lines", "rb");
    if (hSHP == NULL)
    {
        printf("Failed\n");
        return 1;
    }
    printf("The number of shapes is: %d\n", hSHP->nRecords);
    SHPClose(hSHP);
    return 0;
}
