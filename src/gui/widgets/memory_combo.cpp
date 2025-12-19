#include <QSettings>
#include "gui/widgets/memory_combo.h"
#include "sys/sys.h"

//////////////////////////////////////////////////////////////////
///
///  DirMemoryCombo
///
//////////////////////////////////////////////////////////////////

DirMemoryCombo::DirMemoryCombo(QString name) : MemoryCombo(name)
{
    connect(this, &QComboBox::editTextChanged, this, &DirMemoryCombo::slot_text_Changed);
}

void DirMemoryCombo::slot_text_Changed(QString txt)
{
    qDebug() << "DMC" << txt;
    Sys::setWorkingBMPDirectory(txt);
    QComboBox::setItemText(QComboBox::currentIndex(),txt);
}

void DirMemoryCombo::setCurrentText(const QString text)
{
    MemoryCombo::setCurrentText(text);
    Sys::setWorkingBMPDirectory(text);
}

//////////////////////////////////////////////////////////////////
///
///  MemoryCombo
///
//////////////////////////////////////////////////////////////////

MemoryCombo::MemoryCombo(QString name)
{
    this->name = name;
    setEditable(true);
    setDuplicatesEnabled(false);
    setMaxCount(10);
}

MemoryCombo::~MemoryCombo()
{
    persistItems();
}

void MemoryCombo::initialise()
{
    blockSignals(true);
    clear(); // erase the combo

    unsigned int index =0;
    QSettings s;
    QStringList itemList = s.value(name,"").toStringList();
    for (auto & file : std::as_const(itemList))
    {
        if (!file.isEmpty())
        {
            QComboBox::insertItem(index++,file);
        }
    }
    QComboBox::setCurrentIndex(0);
    blockSignals(false);
}

QString MemoryCombo::getTextFor(QString name)
{
    QSettings s;
    QStringList itemList = s.value(name,"").toStringList();
    return itemList[0];
}

QString MemoryCombo::getCurrentText()
{
    return QComboBox::itemText(0);
}

void MemoryCombo::setCurrentText(const QString text)
{
    blockSignals(true);
    QStringList items = getItems();
    if (!items.contains(text))
    {
        QComboBox::insertItem(0,text);
    }
    else
    {
        clear(); // erase the combo
        QComboBox::insertItem(0,text);
        // push down previous list
        auto index = 1;
        for (int i=0;  i< items.count(); i++)
        {
            QString item = items.at(i);
            if (item != text && !item.isEmpty())
            {
                QComboBox::insertItem(index++,item);
            }
        }
        // this could exceed max count
    }
    persistItems();
    QComboBox::setCurrentIndex(0);
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
