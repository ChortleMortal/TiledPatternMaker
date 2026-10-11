#pragma once

#include <QComboBox>
#include "model/motifs/motif.h"

typedef std::shared_ptr<Motif> MotifPtr;

class MotifTypeSelector : public QWidget
{
    Q_OBJECT

public:
    MotifTypeSelector();

    void updateTypeChoices(MotifPtr motif);
    void setVersion(MotifPtr motif);
    int  getIndex(Motif::Type type);

signals:
    void sig_motifTypeChanged(Motif::Type);
    void sig_motifVersionChanged(int);

private slots:
    void slot_motifTypeChanged(int index);
    void slot_motifVersionChanged(int index);

private:
    void add(Motif::Type type, QString name);
    void adjustComboWidth(QComboBox * box);

    QComboBox types;
    QComboBox versions;
};
