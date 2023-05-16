#ifndef VERSIONDIALOG_H
#define VERSIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>

class VersionDialog : public QDialog
{
public:
    VersionDialog(QWidget * parent = nullptr);

    void setText1(QString txt) { text1->setText(txt); }
    void setText2(QString txt) { text2->setText(txt); }

    QComboBox   * combo;

private:
    QLabel * text1;
    QLabel * text2;
};

#endif // VERSIONDIALOG_H
