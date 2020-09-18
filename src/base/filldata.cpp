#include "base/filldata.h"
#include <QDebug>

FillData::FillData()
{
    _set = false;
    minX = -4;
    minY = -4;
    maxX = 4;
    maxY = 4;
}

void FillData::set(int minX, int maxX, int minY, int maxY)
{
    _set = true;
    this->minX = minX;
    this->minY = minY;
    this->maxX = maxX;
    this->maxY = maxY;
}

void FillData::set(FillData & fdata)
{
    _set = true;
    minX = fdata.minX;
    minY = fdata.minY;
    maxX = fdata.maxX;
    maxY = fdata.maxY;
}

void FillData::get(int & minX, int & maxX, int & minY, int & maxY) const
{
    minX = this->minX;
    minY = this->minY;
    maxX = this->maxX;
    maxY = this->maxY;
}

