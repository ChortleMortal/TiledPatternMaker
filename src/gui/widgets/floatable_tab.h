#ifndef FLOATABLE_TAB_H
#define FLOATABLE_TAB_H

#include <QWidget>

class QTabWidget;

class FloatableTab : public QWidget
{
public:
    FloatableTab();

    void detach(QTabWidget * tabwidget, QString title);
    void reattach();

    virtual void closeEvent(QCloseEvent * event) override;

    bool floating;
    QTabWidget * parent;
    QString title;

public slots:
    void slot_raiseDetached();
};

#endif // FLOATABLE_TAB_H
