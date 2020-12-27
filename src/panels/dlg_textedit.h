#ifndef DLGTEXTEDIT_H
#define DLGTEXTEDIT_H

#include <QDialog>
#include <QTextEdit>

class DlgTextEdit : public QDialog
{
public:
    DlgTextEdit(bool show);

    void append(QString msg) { txt.append(msg); }
    void set(QString msg) { txt.setPlainText(msg); }

    QTextEdit txt;
};

#endif // DLGTEXTEDIT_H
