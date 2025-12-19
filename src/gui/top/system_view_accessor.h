#ifndef SYSTEM_VIEW_ACCESSOR_H
#define SYSTEM_VIEW_ACCESSOR_H

#include <QColor>
#include <QSize>
#include <QPixmap>
#include <QLayout>

class SystemView;
class Layer;

class SystemViewAccessor
{
public:
    SystemViewAccessor();

    const QVector<Layer *> getActiveLayers();

    QColor  getViewBackgroundColor();
    void    setViewBackgroundColor(QColor color);

    void    setSize(QSize sz);
    void    setFixedSize(QSize);

    void    clearLayout();

    void    appSuspendPaint(bool suspend);
    void    setPaintDisable(bool disable);
    bool    viewCanPaint();
    void    debugSuspendPaint(bool suspend);
    bool    splashCanPaint();

    void    repaintView();
    QPixmap grabView();
    QPoint  mapToGlobal(const QPoint & pt);
    QRect   viewRect();
    void    showView();
    void    raiseView();
    qreal   viewWidth();
    qreal   viewHeight();

    void    setWindowTitle(const QString & s);
    QLayout *  layout();
    void    move(const QPoint & pt);
    void    setWindowState(Qt::WindowStates ws);
    Qt::WindowStates windowState();
    void    activateWindow();
    QWindow * windowHandle();

protected:
    SystemView * theView;
};

#endif // SYSTEM_VIEW_ACCESSOR_H
