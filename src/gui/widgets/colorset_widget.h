#ifndef COLORSET_WIDGET_H
#define COLORSET_WIDGET_H

#include <QWidget>

#include "model/styles/colorset.h"

class ColorSet;
class AQHBoxLayout;
class QButtonGroup;
class StyleEditor;

class ColorSetWidget : public QWidget
{
    Q_OBJECT

public:
    ColorSetWidget(StyleEditor * parent, ColorSet * cset);

    void updateFromColorSet();

signals:
    void sig_updateView();

private slots:
    void colorButtonClicked(int id);

protected:
    void buildGroup();
    void refreshGroup();

public:
    ColorSet     * cset;
    StyleEditor  * parent;

    QButtonGroup * group;
    AQHBoxLayout * hbox;
};
#endif // COLORSET_WIDGET_H
