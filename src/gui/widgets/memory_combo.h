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
    ~MemoryCombo();

    void initialise();
    void select (int index);
    void eraseCurrent();

    static QString getTextFor(QString name);

public slots:
    virtual void    setCurrentText(const QString text);
    QString getCurrentText();

protected:
    void            persistItems();
    QStringList     getItems();

private:
    QString         name;
};

class DirMemoryCombo : public MemoryCombo
{
    Q_OBJECT

public:
    DirMemoryCombo(QString name);

    virtual void setCurrentText(const QString text) override;


public slots:
    void slot_text_Changed(QString txt);
};

#endif // MEMORY_COMBO_H
