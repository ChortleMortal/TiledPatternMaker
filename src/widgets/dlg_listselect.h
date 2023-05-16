#pragma once
#ifndef DLG_LISTSELECT_H
#define DLG_LISTSELECT_H

class QHBoxLayout;

#include <QDialog>
#include "widgets/versioned_list_widget.h"

typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;

class DlgListSelect : public QDialog
{
    Q_OBJECT

public:
    DlgListSelect(QStringList files,QWidget * parent);

    LoaderListWidget * list;

    QString selectedFile;

protected slots:
    void slot_currentRow(int row);
    void slot_dclick(QPoint pos);

protected:
    virtual void selectAction() {};
    QHBoxLayout * hbox;
};

class AQFrame : public QFrame
{
public:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    PlacedTilePtr tile;
    int scale;
};

class GirihListSelect : public DlgListSelect
{
    Q_OBJECT

public:
    GirihListSelect(QStringList names, QWidget * parent);
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
    class SliderSet * magSlider;
};



#endif
