#ifndef VERSION_STEPPING_ENGINE_H
#define VERSION_STEPPING_ENGINE_H

#include <QImage>
#include "engine/stepping_engine.h"

class QComboBox;

class VersionStepper : public SteppingEngine
{
public:
    VersionStepper(ImageEngine * parent);

    void connect(QComboBox * mediaA, QComboBox * mediaB, QComboBox * versionsA, QComboBox * versionsB);

    virtual bool begin() override;
    virtual bool tick()  override;
    virtual bool next()  override;
    virtual bool prev()  override;
    virtual bool end()   override;

    void loadVersionCombos();
    void compareVersions();
    void compareNextVersions();

    void mediaAChanged();
    void mediaBChanged();

protected:
    QStringList           mediaNames;
    QStringList           versions;
    QStringList::iterator imgList_it;
    QStringList::iterator imgListVerA_it;
    QStringList::iterator imgListVerB_it;
    QImage                imgA;
    QImage                imgB;

    QComboBox   * mediaA;
    QComboBox   * mediaB;
    QComboBox   * versionsA;
    QComboBox   * versionsB;
};

#endif


