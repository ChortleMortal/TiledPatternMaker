#pragma once
#ifndef DLGTEXTEDIT_H
#define DLGTEXTEDIT_H

#include <QDialog>

#include "sys/qt/qtapplog.h"

class QVBoxLayout;

// The LogWidget is a container which holds the current log or a
// saved log which is loaded from disk
class LogWidget : public QWidget
{
public:
    LogWidget();
    virtual ~LogWidget();

    void        set(TextEditPtr te);
    TextEditPtr get() { return ted; }

protected:
    void        unload();

private:
    TextEditPtr ted;
};

// Used to pop Tiling verification message
class DlgTextEdit : public QDialog
{
public:
    DlgTextEdit(QWidget * parent = nullptr);
    ~DlgTextEdit() {}

    void append(QString msg) { txt.append(msg); }
    void set(QString msg)    { txt.setPlainText(msg); }
    void setHtml(QString msg){ txt.setHtml(msg); }
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

class DlgStartOptions : public DlgTextEdit
{
public:
    DlgStartOptions(QWidget * parent = nullptr);

};

class DlgSupport : public DlgTextEdit
{
public:
    DlgSupport(QWidget * parent = nullptr);

};

class DlgKbdOpts : public QDialog
{
public:
    DlgKbdOpts(QWidget * parent = nullptr);

};
#endif // DLGTEXTEDIT_H
