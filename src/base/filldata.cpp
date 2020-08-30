#include "base/filldata.h"
#include <QDebug>

FillData::FillData()
{
    minX = -4;
    minY = -4;
    maxX = 4;
    maxY = 4;
}

void FillData::set(int minX, int maxX, int minY, int maxY)
{
    this->minX = minX;
    this->minY = minY;
    this->maxX = maxX;
    this->maxY = maxY;
}

void FillData::set(FillData & fdata)
{
    minX = fdata.minX;
    minY = fdata.minY;
    maxX = fdata.maxX;
    maxY = fdata.maxY;
}

void FillData::get(int & minX, int & maxX, int & minY, int & maxY)
{
    minX = this->minX;
    minY = this->minY;
    maxX = this->maxX;
    maxY = this->maxY;
}

