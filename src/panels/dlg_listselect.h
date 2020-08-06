#ifndef DLG_LISTSELECT_H
#define DLG_LISTSELECT_H

#include <QDialog>
#include "panels/versioned_list_widget.h"
#include "panels/layout_sliderset.h"
#include "tile/placed_feature.h"


class DlgListSelect : public QDialog
{
    Q_OBJECT

public:
    DlgListSelect(QStringList files);

    LoaderListWidget * list;

    QString selectedFile;

protected slots:
    void slot_currentRow(int row);

protected:
    virtual void selectAction() {};
    QHBoxLayout * hbox;
};

class AQFrame : public QFrame
{
public:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    PlacedFeaturePtr feature;
    int scale;
};

class GirihListSelect : public DlgListSelect
{
    Q_OBJECT

public:
    GirihListSelect(QStringList names);
    QStringList getSelected();

public slots:
    void slot_rightClick(QPoint pt);

private slots:
    void whereUsed();
    void magChanged(int mag);

protected:
    bool isUsed(QString girihname, QStringList & results);
    bool containsGirih(QString girihName, QString filename);
    void selectAction() override;

private:
    AQFrame * frame;
    SliderSet * magSlider;
};



#endif
