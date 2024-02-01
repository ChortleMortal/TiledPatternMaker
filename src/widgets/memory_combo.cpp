#include "widgets/memory_combo.h"
#include <QSettings>

MemoryCombo::MemoryCombo(QString name)
{
    this->name = name;
    setEditable(true);
    setDuplicatesEnabled(false);
    setMaxCount(10);
}

void MemoryCombo::initialise()
{
    clear(); // erase the combo

    unsigned int index =0;
    QSettings s;
    QStringList itemList = s.value(name,"").toStringList();
    for (auto & file : std::as_const(itemList))
    {
        if (!file.isEmpty())
        {
            insertItem(index++,file);
        }
    }
    setCurrentIndex(0);
}

QString MemoryCombo::getTextFor(QString name)
{
    QSettings s;
    QStringList itemList = s.value(name,"").toStringList();
    return itemList[0];
}


QString MemoryCombo::getCurrentText()
{
    return itemText(0);
}

void MemoryCombo::setCurrentText(const QString &text)
{
    blockSignals(true);
    auto items = getItems();
    if (!items.contains(text))
    {
        insertItem(0,text);
    }
    else
    {
        clear(); // erase the combo

        insertItem(0,text);
        auto index = 1;
        for (int i=0;  i< items.count(); i++)
        {
            auto item = items.at(i);
            if (item != text && !item.isEmpty())
            {
                insertItem(index++,item);
            }
        }
        // this could exceed max count
    }
    persistItems();
    setCurrentIndex(0);
    blockSignals(false);
}

void MemoryCombo::select(int index)
{
    blockSignals(true);

    if (index == 0)
    {
        blockSignals(false);
        return;     // don't need to do anything
    }

    auto items = getItems();
    if (index < 0 || index >= items.size())
    {
        blockSignals(false);
        return;     // don't need to do anything
    }
    auto selection = items.at(index);

    if (selection.isEmpty())
    {
        blockSignals(false);
        return;
    }

    clear();  // erase combo

    insertItem(0,selection);
    auto index2 = 1;
    for (int i=0;  i< items.count(); i++)
    {
        auto item = items.at(i);
        if (item != selection && !item.isEmpty())
        {
            insertItem(index2++,item);
        }
    }
    setCurrentIndex(0);
    persistItems();
    blockSignals(false);
}

void MemoryCombo::eraseCurrent()
{
    blockSignals(true);
    removeItem(currentIndex());
    setCurrentIndex(0);
    persistItems();
    blockSignals(false);
}

QStringList MemoryCombo::getItems()
{
    QStringList items;
    for (int i = 0; i < count(); i++)
    {
        items << itemText(i);
    }
    return items;
}

void MemoryCombo::persistItems()
{
    QStringList items = getItems();
    QSettings s;
    s.setValue(name,items);
}
