#pragma once
#ifndef WORKLIST_WIDGET_H
#define WORKLIST_WIDGET_H

#include <QListWidget>

class WorklistWidget : public QListWidget
{
    Q_OBJECT

public:
    WorklistWidget(QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent * event) override;

protected slots:
     void slot_insertAction();
     void slot_editAction();
     void slot_deleteAction();

private:
};

#endif
