#ifndef FILLDATA_H
#define FILLDATA_H

class FillData
{
public:
    FillData();
    void set(int minX, int minY, int maxX, int maxY);
    void set(FillData & fdata);
    void get(int & minX, int & maxX, int & minY, int & maxY);

protected:
    int minX;
    int minY;
    int maxX;
    int maxY;
};

#endif // FILLDATA_H
