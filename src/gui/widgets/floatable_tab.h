#pragma once

#include <QWidget>

class QTabWidget;
class panel_page;

class FloatableTab : public QWidget
{
public:
    FloatableTab(panel_page * owner);

    void detach(QTabWidget * tabwidget, QString title);
    void reattach();
    void savePosition();

    virtual void closeEvent(QCloseEvent * event) override;

    bool floating;
    QTabWidget * parent;
    QString title;

public slots:
    void slot_raiseDetached();
};
