#pragma once
#ifndef STYLE_COLOR_FILL_FACE_H
#define STYLE_COLOR_FILL_FACE_H

#include <QVBoxLayout>

class QTableWidget;
class AQTableWidget;
class FilledEditor;

typedef std::shared_ptr<class Filled>       FilledPtr;

class StyleColorFillFace : public QObject
{
    Q_OBJECT

    enum eCol
    {
        COL_STATUS      = 0,
        COL_COLOR       = 1,
        COL_EDIT        = 2
    };

public:
    StyleColorFillFace(FilledEditor * parent, FilledPtr style, QVBoxLayout * vbox);

    void onRefresh();

    void display();

    void select(QPointF mpt, Qt::MouseButton btn);

private slots:
    void slot_cellClicked(int row,int column);

private:
    FilledEditor *  parent;
    FilledPtr       filled;
    QTableWidget  * table;
    int             iPaletteSelection;

};
#endif
