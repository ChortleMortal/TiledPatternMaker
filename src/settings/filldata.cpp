#include "settings/filldata.h"
#include <QDebug>

FillData::FillData()
{
    _set = false;
    minX = -3;
    minY = -3;
    maxX = 3;
    maxY = 3;
    singleton = false;
}

FillData::FillData(const FillData & fdata)
{
    *this = fdata;
    _set = true;
}

FillData & FillData::operator=(const FillData & fdata)
{
    minX = fdata.minX;
    minY = fdata.minY;
    maxX = fdata.maxX;
    maxY = fdata.maxY;
    singleton = fdata.singleton;
    _set = fdata._set;

    return *this;
}

void FillData::set(bool singleton, int minX, int maxX, int minY, int maxY)
{
    _set = true;
    this->minX = minX;
    this->minY = minY;
    this->maxX = maxX;
    this->maxY = maxY;
    this->singleton = singleton;
}

void FillData::get(bool & singleton, int & minX, int & maxX, int & minY, int & maxY) const
{
    singleton = this->singleton;
    minX = this->minX;
    minY = this->minY;
    maxX = this->maxX;
    maxY = this->maxY;
}

