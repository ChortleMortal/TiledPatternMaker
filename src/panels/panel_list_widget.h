#ifndef PANELLISTWIDGET_H
#define PANELLISTWIDGET_H

#include <QtWidgets>

class PanelListWidget : public QListWidget
{
    Q_OBJECT

public:
    PanelListWidget(QWidget *parent = nullptr);

    void setCurrentRow(QString name);
    void removeItem(QString name);

    void addSeparator();

    void mousePressEvent(QMouseEvent * event) override;

    void  establishHeight();
    QSize sizeHint() const override;

signals:
    void detachWidget(QString name);

protected slots:
    void slot_floatAction();

protected:
    QAction * floatAction;

private:
    volatile bool localDrop;
    int  floatIndex;

};

#endif // PANELLISTWIDGET_H
