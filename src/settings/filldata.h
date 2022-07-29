#ifndef FILLDATA_H
#define FILLDATA_H

class FillData
{
public:
    FillData();

    bool isSet() { return _set; }
    void set(bool singleton, int minX, int minY, int maxX, int maxY);
    void set(FillData & fdata);
    void get(bool & singleton, int & minX, int & maxX, int & minY, int & maxY) const;

protected:
    bool _set;
    bool singleton;
    int minX;
    int minY;
    int maxX;
    int maxY;
};

#endif // FILLDATA_H
