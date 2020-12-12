#ifndef PANELLISTWIDGET_H
#define PANELLISTWIDGET_H

#include <QtWidgets>

class PanelListWidget : public QListWidget
{
    Q_OBJECT

public:
    PanelListWidget(QWidget *parent = nullptr);

    void setCurrentRow(QString name);
    void setCurrentRow(int row);

    void hide(QString name);
    void show(QString name);

    void addSeparator();

    void mousePressEvent(QMouseEvent * event) override;

    void  establishSize();

signals:
    void sig_detachWidget(QString name);

protected slots:
    void slot_floatAction();

protected:

private:
    volatile bool localDrop;
    int           floatIndex;
    int           separators;
};

#endif // PANELLISTWIDGET_H
