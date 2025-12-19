#pragma once
#ifndef FILLDATA_H
#define FILLDATA_H

class FillData
{
public:
    FillData();
    FillData(const FillData & fdata);

    FillData& operator=(const FillData& rhs);
    bool operator==(const FillData & other) const;
    bool operator != (const FillData & other) const { return !(*this == other); }

    bool isSet() const { return _set; }
    void set(bool singleton, int minX, int minY, int maxX, int maxY);
    void get(bool & singleton, int & minX, int & maxX, int & minY, int & maxY) const;

    void setLegacyDefaults();

protected:
    bool _set;
    bool singleton;
    int minX;
    int minY;
    int maxX;
    int maxY;
};

#endif // FILLDATA_H



