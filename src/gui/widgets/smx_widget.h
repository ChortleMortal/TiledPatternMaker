#ifndef SMX_WIDGET_H
#define SMX_WIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QPainter>
class Layer;

//////////////////////////////////////////////////////////
///
///   BQCheckBox
///
//////////////////////////////////////////////////////////
class BQCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    BQCheckBox(const QString &text, QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *e) override;

    bool selected;
};

//////////////////////////////////////////////////////////
///
///   SMXWidget
///
//////////////////////////////////////////////////////////
class SMXWidget : public QWidget
{
    Q_OBJECT

public:
    SMXWidget(Layer *layer, bool abbrev, bool box);

    void    setLayer(Layer * layer) { _layer = layer; }
    void    refresh();

    void    setSelected(bool s);

protected:
    void    paintEvent(QPaintEvent *event) override;

private slots:
    void    slot_lock(bool enb);
    void    slot_solo(bool enb);
    void    slot_breakaway(bool enb);

private:
    Layer * _layer;
    bool    _box;
    bool    _selected;

    BQCheckBox   * chkLock;
    BQCheckBox   * chkSolo;
    BQCheckBox   * chkBreakaway;

};

#endif // SMX_WIDGET_H
