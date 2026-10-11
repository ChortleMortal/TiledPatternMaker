#include <QDebug>
#include <QFile>

#include "compare_bmp_engine.h"
#include "gui/panels/page_image_tools.h"

CompareBMPEngine::CompareBMPEngine() {}

bool CompareBMPEngine::compareBMPs(VersionedName name, QString pathA, QString pathB)
{
    if (!QFile::exists(pathA) && pathA.contains(".png"))
        pathA.replace(".png",".bmp");
    QImage imgA(pathA);

    if (!QFile::exists(pathB) && pathB.contains(".png"))
        pathB.replace(".png",".bmp");
    QImage imgB(pathB);

    bool rv = compareImages(name.get(), imgA, imgB);
    if (!rv)
    {
        page_image_tools::addToComparisonWorklist(name);
    }
    return rv;
}

bool CompareBMPEngine::compareImages(QString name, QImage & imageA, QImage & imageB)
{
    if (imageA.isNull())
    {
        qWarning() << "Image A not found" << name;
        return false;
    }

    if (imageB.isNull())
    {
        qWarning() << "Image B not found" << name;
        return false;
    }

    if (imageA == imageB)
    {
        qInfo() <<  "Images are the same"  << name;
        return true;
    }

    // files are different
    qWarning() << "Images are different" << name;
    return false;
}
