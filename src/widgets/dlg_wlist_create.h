#pragma once
#ifndef DLG_WORKLIST_CREATE_H
#define DLG_WORKLIST_CREATE_H

#include <QDialog>
#include "mosaic/mosaic_io_base.h"

class QCheckBox;
class QComboBox;
class QLineEdit;

class DlgWorklistCreate : public QDialog, MosaicIOBase
{
    Q_OBJECT

public:
    DlgWorklistCreate(QWidget * parent = nullptr);

    QCheckBox * chkLoadFilter;
    QCheckBox * chkStyle;
    QCheckBox * chkMotif;

    QComboBox * styleNames;
    QComboBox * motifNames;
};

#endif
