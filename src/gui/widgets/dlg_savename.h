#ifndef DLG_SAVENAME_H
#define DLG_SAVENAME_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>

class DlgSaveName : public QDialog
{
#define SN_TIMESTAMP    0x01
#define SN_HOST         0x02
#define SN_SWVER        0x04
#define SN_GITBRANCH    0x08
#define SN_GITROOT      0x10
#define SN_GITSHA       0x20
#define SN_MOSAIC       0x40


    Q_OBJECT
public:
    DlgSaveName(QWidget * parent);

    QString     _name;
    QLineEdit * leName;

protected:
    void genName();

    void select(uint option, bool sel);

private:
    QCheckBox * cbBuild;

    uint    selections;
};

#endif // DLG_SAVENAME_H
