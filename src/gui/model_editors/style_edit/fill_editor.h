#pragma once
#ifndef FILLED_EDITORS_H
#define FILLED_EDITORS_H

#include "sys/enums/efilltype.h"
#include "sys/enums/estyletype.h"
#include "gui/model_editors/style_edit/style_editor.h"

class FilledSubTypeEditor;
class ColorMaker;

class QComboBox;
class QVBoxLayout;
class QHBoxLayout;

using std::shared_ptr;
using std::weak_ptr;

typedef shared_ptr<class Filled>    FilledPtr;
typedef shared_ptr<class Style>     StylePtr;
typedef weak_ptr<class Style>      wStylePtr;

// Editor for the filled style.


class FilledEditor : public StyleEditor
{
    Q_OBJECT

public:
    FilledEditor(StylePtr style,eStyleType user);
    ~FilledEditor();

    FilledPtr getFilled() { return wfilled.lock(); }

    void onEnter() override;
    void onExit() override;
    void onRefresh() override;

    void notify();

public slots:
    void slot_algo(eFillType algo);
    void slot_colorsChanged();
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn);
    void slot_colorPick(QColor color);

protected:
    void createSubTypeEditor(eFillType algo);

private:
    weak_ptr<Filled>    wfilled;

    QComboBox         * algoBox;
    QVBoxLayout       * vbox;
    QHBoxLayout       * hbox;

    FilledSubTypeEditor * currentEditor;

    int crow;

    class ControlPanel  * panel;
};


class FilledSubTypeEditor : public QObject
{
    Q_OBJECT

public:
    FilledSubTypeEditor(FilledEditor * parent, FilledPtr filled, ColorMaker * cm);
    ~FilledSubTypeEditor();

    virtual void refresh()                                      = 0;
    virtual void mousePressed(QPointF mpt, Qt::MouseButton btn) = 0;
    virtual void colorPick(QColor color)                        = 0;
    virtual void notify()                                       = 0;

    FilledEditor    * parent;
    weak_ptr<Filled>  wfilled;
    ColorMaker      * cm;
};

#endif