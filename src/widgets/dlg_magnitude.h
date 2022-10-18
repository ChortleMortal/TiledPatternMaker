#ifndef DLG_MAGNITUDE_H
#define DLG_MAGNITUDE_H

#include <memory>
#include <QDialog>

typedef std::shared_ptr<class TileSelector>   TilingSelectorPtr;
typedef std::shared_ptr<class Edge>             EdgePtr;

class DlgMagnitude : public QDialog
{
    Q_OBJECT

public:
    DlgMagnitude(TilingSelectorPtr sel, QWidget * parent = nullptr);

signals:
    void sig_magnitudeChanged();

private slots:
    void slot_valueChanged(qreal val);



private:
    class DoubleSliderSet   * magWidget;
    TilingSelectorPtr   sel;
    EdgePtr             edge;
};

#endif
