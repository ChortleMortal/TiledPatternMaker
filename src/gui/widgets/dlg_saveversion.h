#pragma once

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include "sys/enums/efilesystem.h"
#include "sys/sys/versioning.h"

class DlgSaveVersion : public QDialog
{
public:
    DlgSaveVersion(QWidget * parent = nullptr);

    void setText1(QString txt) { text1->setText(txt); }
    void setText2(QString txt) { text2->setText(txt); }

    VersionedName bumpedName(eFileType type, const VersionedName & vname);

    QComboBox   * combo;

private:
    QLabel * text1;
    QLabel * text2;
};
