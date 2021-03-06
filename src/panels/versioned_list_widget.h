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

#ifndef VERSIONED_LIST_WIDGET_H
#define VERSIONED_LIST_WIDGET_H

#include <QtWidgets>

class LoaderListWidget : public QListWidget
{
    Q_OBJECT

public:
    LoaderListWidget(QWidget *parent = nullptr);

    virtual void addItemList(QStringList  list);
    bool selectItemByName(QString name);
    bool selectItemByValue(QVariant val);


private:
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;

signals:
    void rightClick(QPoint pos);
    void leftDoubleClick(QPoint pos);
    void listEnter();
};

typedef  std::pair<std::string,QVector<int>> VersionSet;

class VersionList
{
public:
    VersionList() {}

    void create(QStringList list);

    QStringList recompose();

protected:
    VersionSet * find(QString name);

private:
    static bool ciCharLess( char c1, char c2 );
    static bool CompareNoCase( const std::string& s1, const std::string& s2 );
    static bool ComparePair(const VersionSet & a, VersionSet &b);

    QVector<VersionSet>  versionList;
};


class VersionedListWidget : public LoaderListWidget
{
public:
    VersionedListWidget();

    void addItemList(QStringList list) override;
};



#endif
