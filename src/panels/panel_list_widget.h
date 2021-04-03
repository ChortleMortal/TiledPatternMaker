#ifndef PANELLISTWIDGET_H
#define PANELLISTWIDGET_H

#include <QtWidgets>

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

    int           floatIndex;
};

class ListListWidget : public AQListWidget
{
    Q_OBJECT

public:
    ListListWidget(QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent * event) override;

protected slots:
     void slot_editAction();
     void slot_deleteAction();

private:
    int index;
};

#endif // PANELLISTWIDGET_H
