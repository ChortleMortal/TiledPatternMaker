#include "model/settings/filldata.h"
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

bool FillData::operator==(const FillData & other) const
{
    if (minX != other.minX) return false;
    if (minY != other.minY) return false;
    if (maxX != other.maxX) return false;
    if (maxY != other.maxY) return false;
    if (singleton != other.singleton) return false;
    //_set = other._set;

    return true;
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

void FillData::setLegacyDefaults()
{
    minX = -4;
    minY = -4;
    maxX = 4;
    maxY = 4;
}
