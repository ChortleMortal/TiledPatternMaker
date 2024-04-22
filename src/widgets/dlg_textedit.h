#pragma once
#ifndef DLGTEXTEDIT_H
#define DLGTEXTEDIT_H

class QVBoxLayout;

#include <QDialog>
#include <QTextEdit>
#include "misc/qtapplog.h"

// The TextEditorWidget is a container which holds the current log or a
// saved log which is loaded from disk
class TextEditorWidget : public QWidget
{
public:
    TextEditorWidget();
    TextEditorWidget(QTextEdit * ee);

    void        set(QTextEdit * te);
    QTextEdit * get() { return ed; }

private:
    QTextEdit * ed;
};

// Used to pop Tiling verification message
class DlgTextEdit : public QDialog
{
public:
    DlgTextEdit(QWidget * parent = nullptr);
    ~DlgTextEdit() {}

    void append(QString msg) { txt.append(msg); }
    void set(QString msg)    { txt.setPlainText(msg); }
    void set(const QVector<sTrapMsg> &msgs);

    QTextEdit txt;

protected:
    class QVBoxLayout * vbox;
};

// Used to pop up Map verification messags
class DlgMapVerify : public DlgTextEdit
{
public:
    DlgMapVerify(QString name, QWidget * parent = nullptr);
    ~DlgMapVerify() {}
};

// Used to pop up information About this app
class DlgAbout : public DlgTextEdit
{
public:
    DlgAbout(QWidget * parent = nullptr);

};
#endif // DLGTEXTEDIT_H
