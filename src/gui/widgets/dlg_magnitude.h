#pragma once
#ifndef DLG_MAGNITUDE_H
#define DLG_MAGNITUDE_H

#include <QDialog>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

typedef std::shared_ptr<class PlacedTileSelector>   PlacedTileSelectorPtr;
typedef std::shared_ptr<class Edge>           EdgePtr;

class DlgMagnitude : public QDialog
{
    Q_OBJECT

public:
    DlgMagnitude(PlacedTileSelectorPtr sel, QWidget * parent = nullptr);

signals:
    void sig_magnitudeChanged();

private slots:
    void slot_valueChanged(qreal val);

private:
    class DoubleSliderSet  * magWidget;
    PlacedTileSelectorPtr    sel;
    EdgePtr                  edge;
};

#endif
