#pragma once

#include <QString>
#include <QImage>
#include "sys/sys/versioning.h"

class CompareBMPEngine
{
public:
    CompareBMPEngine();

    bool compareBMPs(VersionedName name, QString pathA, QString pathB);

protected:
    bool compareImages(QString name, QImage & imageA, QImage & imageB);

};
