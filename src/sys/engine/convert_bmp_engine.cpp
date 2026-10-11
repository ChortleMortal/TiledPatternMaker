#include <QImage>
#include <QFile>
#include "convert_bmp_engine.h"

ConvertBMPEngine::ConvertBMPEngine() {}

bool ConvertBMPEngine::convertToPNG(QString filename)
{
    QImage img(filename);
    if (!img.isNull())
    {
        QString pngFilename = filename;
        pngFilename.replace(".bmp", ".png", Qt::CaseInsensitive);
        if (img.save(pngFilename, "PNG"))
        {
            QImage verify(pngFilename);
            if (!verify.isNull() && verify.size() == img.size())
            {
                QFile::remove(filename);
                return true;
            }
        }
    }
    return false;
}

