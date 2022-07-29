#ifndef MODEL_SETTINGS_H
#define MODEL_SETTINGS_H

#include <QSize>
#include <QColor>
#include <QPointF>

#include "settings/filldata.h"

class ModelSettings
{
public:
    ModelSettings();
    ModelSettings(const ModelSettings & other);
    ~ModelSettings();

    ModelSettings & operator=(const ModelSettings & other);

    void            clear();

    QColor          getBackgroundColor();
    void            setBackgroundColor(QColor color);

    void            setSize(QSize size);
    QSize           getSize() { return _size; }

    void            setZSize(QSize size) { _zsize = size; }
    QSize           getZSize() { return _zsize; }

    QPointF         getStartTile();
    void            setStartTile(QPointF pt);

    void            setFillData(FillData *fd);
    FillData *      getFillData();

    QPointF         getCenter();

protected:

private:
    QSize           _size;          // crop size
    QSize           _zsize;         // zoom size
    QColor          _bkgdColor;
    QPointF         _startTile;
    FillData        _fillData;
};

#endif
