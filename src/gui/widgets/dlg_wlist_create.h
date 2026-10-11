#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QRadioButton;
class QLineEdit;

class DlgWorklistCreate : public QDialog
{
    Q_OBJECT

public:
    DlgWorklistCreate(QWidget * parent = nullptr);

    QStringList selectedMotifNames();

    QRadioButton * selMosaic;
    QRadioButton * selTiling;
    QCheckBox    * useWorklist;
    QCheckBox    * inverseMosaicWorklist;

    QCheckBox    * chkLoadFilter;
    QRadioButton * radStyle;
    QRadioButton * radMotif;
    QRadioButton * radText;

    QComboBox * styleNames;
    QComboBox * motifNames;

    QLineEdit * text;
};
