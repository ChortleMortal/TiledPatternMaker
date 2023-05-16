#pragma once
#ifndef DLG_CLEANSE_H
#define DLG_CLEANSE_H

#include <QDialog>

class QCheckBox;

typedef std::shared_ptr<class Mosaic> MosaicPtr;

class DlgCleanse : public QDialog
{
    Q_OBJECT

public:
    DlgCleanse(MosaicPtr mosaic, uint cleanseLevel, QWidget * parent = nullptr);

    uint getLevel();

protected:
    void toCheckboxes(uint level);

private slots:
    void slot_analyse();

private:
    QCheckBox * badV0;
    QCheckBox * badV1;
    QCheckBox * badEdges0;
    QCheckBox * joinEdges;
    QCheckBox * divEdges;
    QCheckBox * cleanNeigh;
    QCheckBox * buildNeigh;

    MosaicPtr   mosaic;
};

#endif
