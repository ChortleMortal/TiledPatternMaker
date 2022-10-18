#include "widgets/memory_combo.h"
#include "settings/configuration.h"

MemoryCombo::MemoryCombo(QString name)
{
    this->name = name;
    setEditable(true);
    setDuplicatesEnabled(false);
    setMaxCount(10);
    config = Configuration::getInstance();
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
        config->viewImages = getItems();
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
    setCurrentIndex(0);
    config->viewImages = getItems();
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
    config->viewImages = getItems();
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
