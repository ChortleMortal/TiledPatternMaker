#ifndef DLGTEXTEDIT_H
#define DLGTEXTEDIT_H

#include <QtWidgets>

class DlgTextEdit : public QDialog
{
public:
    DlgTextEdit(QWidget * parent = nullptr);
    ~DlgTextEdit();

    void append(QString msg) { txt.append(msg); }
    void set(QString msg)    { txt.setPlainText(msg); }
    void set(QStringList & qsl);

    QTextEdit txt;

protected:
    QVBoxLayout * vbox;
};

class DlgMapVerify : public DlgTextEdit
{
public:
    DlgMapVerify(QString name, QWidget * parent = nullptr);
    ~DlgMapVerify();
};


class DlgLogView : public DlgTextEdit
{
public:
    DlgLogView(QString name, QWidget * parent = nullptr);
    ~DlgLogView();

protected:
    void slot_save();
};

#endif // DLGTEXTEDIT_H
