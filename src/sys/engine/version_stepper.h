#ifndef VERSION_STEPPING_ENGINE_H
#define VERSION_STEPPING_ENGINE_H

#include <QImage>
#include "sys/engine/stepping_engine.h"
#include "sys/sys/versioning.h"

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
    VersionList           versions;
    QStringList           mediaNames;
    QStringList::iterator imgList_it;
    QVector<VersionedName>::iterator imgListVerA_it;
    QVector<VersionedName>::iterator imgListVerB_it;
    QImage                imgA;
    QImage                imgB;

    QComboBox   * mediaA;
    QComboBox   * mediaB;
    QComboBox   * versionsA;
    QComboBox   * versionsB;
};

#endif


