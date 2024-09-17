#pragma once
#ifndef DLG_CLEANSE_H
#define DLG_CLEANSE_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>

class QCheckBox;

typedef std::shared_ptr<class Map> MapPtr;

class DlgCleanse : public QDialog
{
    Q_OBJECT

public:
    DlgCleanse(MapPtr map, uint cleanseLevel, qreal sensitivity, QWidget * parent = nullptr);

    uint  getLevel();
    qreal getSsnsitivity() { return sensitivity; }

protected:
    void toCheckboxes(uint level);

signals:
    void  sig_cleansed();

public slots:
    void slot_mergeSensitivity(qreal r);

private slots:
    void slot_analyse();
    void slot_cleanse();
    void slot_quickset();

private:
    QCheckBox * badV0;
    QCheckBox * badV1;
    QCheckBox * badEdges0;
    QCheckBox * nearPoints;
    QCheckBox * joinEdges;
    QCheckBox * divEdges;
    QCheckBox * cleanNeigh;
    QCheckBox * buildNeigh;

    class DoubleSpinSet * sen;
    QLabel        * status;
    QLabel        * status2;

    MapPtr      map;
    qreal       sensitivity;
};

class QuicksetCleanse : public QDialog
{
    Q_OBJECT

public:
    QuicksetCleanse(QWidget * parent);

    qreal sensitivity;

protected slots:
    void slot_mergeSensitivity(int sens);

private:
    QComboBox  * box;
};

#endif
