#ifndef TPMSPLASH_H
#define TPMSPLASH_H

#include <QStack>
#include <QLabel>

class TPMSplash : public QLabel
{
    Q_OBJECT

public:
    TPMSplash();

    void display(QString & txt);
    void remove();

protected:
    void draw();

private:
    QStack<QString>  msgStack;
};

#endif
