#ifndef SMX_WIDGET_H
#define SMX_WIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QPainter>
class Layer;

class SMXWidget : public QWidget
{
    Q_OBJECT

public:
    SMXWidget(Layer *layer, bool abbrev, bool box);

    void    setLayer(Layer * layer) { _layer = layer; }

    void    refresh();

protected:
    void    paintEvent(QPaintEvent *event) override;

private slots:
    void    slot_lock(bool enb);
    void    slot_solo(bool enb);
    void    slot_breakaway(bool enb);

private:
    Layer * _layer;
    bool    _box;

    QCheckBox   * chkLock;
    QCheckBox   * chkSolo;
    QCheckBox   * chkBreakaway;

};

#endif // SMX_WIDGET_H
