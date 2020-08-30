/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "panels/versioned_list_widget.h"
#include <algorithm>

LoaderListWidget::LoaderListWidget(QWidget *parent) : QListWidget(parent)
{
    setSortingEnabled(false);
    setMouseTracking(true);
    setStyleSheet("QListWidget::item:selected { background:yellow; color:red; }");
}

void LoaderListWidget::addItemList(QStringList list)
{
    clear();
    for (auto item : list)
    {
        addItem(item);
    }
}

bool LoaderListWidget::selectItemByName(QString name)
{
    Q_ASSERT(!name.contains(".xml"));

    for (int i=0; i < count(); i++)
    {
        QListWidgetItem * qitem = item(i);
        if (qitem->text() == name)
        {
            //qDebug() << "selected" << name;
            setCurrentItem(qitem);
            return true;
        }
    }

    // not found
    return false;
}

bool LoaderListWidget::selectItemByValue(QVariant val)
{
    for (int i=0; i < count(); i++)
    {
        QListWidgetItem * qitem = item(i);
        if (qitem->data(Qt::UserRole) == val)
        {
            //qDebug() << "selected" << val;
            setCurrentItem(qitem);
            return true;
        }
    }

    // not found
    return false;
}

void LoaderListWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        emit rightClick(event->pos());
    }
    else
    {
        QListWidget::mousePressEvent(event);
    }
}

void LoaderListWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit leftDoubleClick(event->pos());
    }
    else
    {
        QListWidget::mouseDoubleClickEvent(event);
    }
}

void LoaderListWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        emit listEnter();
    }
}

VersionedListWidget::VersionedListWidget()
{
}

void VersionedListWidget::addItemList(QStringList list)
{
    VersionList vList;
    vList.create(list);
    QStringList list2 = vList.recompose();
    LoaderListWidget::addItemList(list2);
}

bool VersionList::ciCharLess( char c1, char c2 )
{
    return (tolower( static_cast<unsigned char>( c1 ) ) < tolower( static_cast<unsigned char>( c2) ));
}

bool VersionList::CompareNoCase( const std::string& s1, const std::string& s2 )
{
    return std::lexicographical_compare( s1.begin(), s1.end(), s2.begin(), s2.end(), ciCharLess);
}

bool VersionList::ComparePair(const VersionSet & a, VersionSet &b)
{
    return CompareNoCase(a.first,b.first);
}

void VersionList::create(QStringList list)
{
    for (auto file : list)
    {
        QStringList l = file.split('.');
        int size = l.size();

        QString name;
        QString value;
        int version = 0;
        if (size > 1)
        {
            value = l.last();
            if (value.contains('v'))
            {
                value.remove('v');
                version = value.toInt();
                l.removeLast();
                name = l.join('.');
            }
        }
        name = l.join('.');

        VersionSet * set = find(name);
        if (set)
        {
            set->second.push_back(version);
        }
        else
        {
            QVector<int> qv;
            qv.push_back(version);
            VersionSet v = std::make_pair(name.toStdString(),qv);
            versionList.push_back(v);
        }
    }
    // sort names
    std::sort(versionList.begin(), versionList.end(),ComparePair);

    // sort versions
    for (int i=0; i < versionList.size(); i++)
    {
        VersionSet * set = &versionList[i];
        QVector<int> & vers = set->second;
        std::sort(vers.begin(), vers.end());
    }
}

QStringList VersionList::recompose()
{
    QStringList list;
    for  (auto set : versionList)
    {
        std::string name = set.first;
        for  (int version : set.second)
        {
            std::string fullName = name;
            if (version > 0)
            {
               fullName += ".v";
               fullName += std::to_string(version);
            }
            list << QString(fullName.c_str());
        }
    }
    return list;
}

VersionSet * VersionList::find(QString name)
{
   for (int i=0; i < versionList.size(); i++)
   {
       VersionSet * set = &versionList[i];
       if (set->first == name.toStdString())
       {
           return set;
       }
   }
   return nullptr;
}

