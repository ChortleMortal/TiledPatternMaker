#pragma once
#ifndef PANELLISTWIDGET_H
#define PANELLISTWIDGET_H

#include <QListWidget>

class AQListWidget : public QListWidget
{
    Q_OBJECT

public:
    AQListWidget(QWidget *parent = nullptr);

    void setCurrentRow(QString name);
    void setCurrentRow(int row);

    void hide(QString name);
    void show(QString name);

    void addSeparator();

    virtual void mousePressEvent(QMouseEvent * event) override;

    void  establishSize();

signals:
    void sig_detachWidget(QString name);

private:
    volatile bool localDrop;
    int           separators;
};


class PanelListWidget : public AQListWidget
{
    Q_OBJECT

public:
    PanelListWidget(QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent * event) override;

protected slots:
    void slot_floatAction();

private:

    int floatIndex;
};

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

#endif // PANELLISTWIDGET_H
