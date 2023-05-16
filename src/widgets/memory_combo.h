#pragma once
#ifndef MEMORY_COMBO_H
#define MEMORY_COMBO_H

#include <QComboBox>

class Configuration;

class MemoryCombo : public QComboBox
{
    Q_OBJECT

public:
    MemoryCombo(QString name);

    void initialise();
    void select (int index);
    void eraseCurrent();

    static QString getTextFor(QString name);

public slots:
    void    setCurrentText(const QString &text);
    QString getCurrentText();

protected:
    void            persistItems();
    QStringList     getItems();
private:
    QString         name;
};

#endif // MEMORY_COMBO_H
