#pragma once

#include <QComboBox>

class MemoryCombo : public QComboBox
{
    Q_OBJECT

public:
    MemoryCombo(QString name);
    ~MemoryCombo();

    void            select (int index);
    void            eraseCurrent();

    static QString  getTextFor(QString name);

public slots:
    virtual void    setCurrentText(const QString text);
    QString         getCurrentText();

protected:
    void            initialise();
    void            persistItems();
    QStringList     getItems();

private:
    QString         name;
};
