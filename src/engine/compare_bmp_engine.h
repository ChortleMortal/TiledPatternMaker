#ifndef COMPAREBMPENGINE_H
#define COMPAREBMPENGINE_H

#include <QString>
#include <QImage>

class CompareBMPEngine
{
public:
    CompareBMPEngine();

    bool compareBMPs(QString name, QString pathA, QString pathB);

protected:
    bool compareImages(QString name, QImage & imageA, QImage & imageB);

};

#endif // COMPAREBMPENGINE_H
