#ifndef DLG_LINE_EDIT_H
#define DLG_LINE_EDIT_H

#include <QtWidgets>

class EdgePoly;

class DlgLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    DlgLineEdit(EdgePoly & epoly, int row, int col);

    void focusInEvent(QFocusEvent *) override;

signals:
    void currentPoint(QPointF pt);

public slots:
    void slot_update();

protected slots:
    void slot_menu(QPointF spt);
    void slot_down();
    void slot_nearest();
    void slot_up();
    void slot_undo();
    void slot_editingFinished();

private:
    class View * view;

    int row;
    int col;
    EdgePoly & _poly;
    QPointF    _orig;
};

#endif
