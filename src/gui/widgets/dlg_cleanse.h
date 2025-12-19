#pragma once
#ifndef DLG_CLEANSE_H
#define DLG_CLEANSE_H

#include <QDialog>

class QLabel;
class QCheckBox;
class DoubleSpinSet;
class QComboBox;

typedef std::shared_ptr<class Map> MapPtr;

class DlgCleanse : public QDialog
{
    Q_OBJECT

public:
    DlgCleanse(MapPtr map, uint cleanseLevel, qreal sensitivity, QWidget * parent = nullptr);

    uint  fromCheckboxes();
    qreal getSsnsitivity() { return sensitivity; }

protected:
    void toCheckboxes(uint level);
    void setStatus(QLabel * label);

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
    QCheckBox * cleanEdges;

    DoubleSpinSet * sen;
    QLabel        * status1;
    QLabel        * status2;

    MapPtr      map;
    qreal       sensitivity;
};

class QuicksetCleanse : public QDialog
{
    Q_OBJECT

public:
    QuicksetCleanse(QWidget * parent,qreal existing);

    qreal qs_sensitivity;

protected slots:
    void slot_sensitivitySelected(uint sens);

private:
    static qreal rvals[16];
    QComboBox  * box;
};

#endif
