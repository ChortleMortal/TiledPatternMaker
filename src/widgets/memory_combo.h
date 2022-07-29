#ifndef MEMORY_COMBO_H
#define MEMORY_COMBO_H

#include <QComboBox>

class Configuration;

class MemoryCombo : public QComboBox
{
    Q_OBJECT

public:
    MemoryCombo(QString name);

    void select (int index);

public slots:
    void setCurrentText(const QString &text);

protected:
    QStringList getItems();

private:
    QString name;
    Configuration * config;
};

#endif // MEMORY_COMBO_H
