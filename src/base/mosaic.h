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

#ifndef MOSAIC_H
#define MOSAIC_H

#include "base/shared.h"
#include "base/model_settings.h"
#include "base/misc.h"

typedef QVector<StylePtr>  StyleSet;

class Mosaic
{
public:
    Mosaic();
    ~Mosaic();

    bool        hasContent() { return (styleSet.size() > 0); }
    int         numStyles()  { return styleSet.size(); }

    // loaded styles
    void        addStyle(StylePtr style);   // adds at front
    void        replaceStyle(StylePtr oldStyle, StylePtr newStyle);
    int         moveUp(StylePtr style);
    int         moveDown(StylePtr style);
    void        deleteStyle(StylePtr style);

    void                  setPrototype(StylePtr style, PrototypePtr pp);
    QVector<PrototypePtr> getPrototypes();
    void                  resetPrototypeMaps();

    ModelSettingsPtr getSettings();
    void             setSettings(ModelSettingsPtr settings);

    const StyleSet & getStyleSet() { return styleSet; }
    StylePtr         getFirstStyle();

    BorderPtr   getBorder() { return border; }
    void        setBorder(BorderPtr bp);

    void        setName(QString name);
    QString     getName();

    void        setNotes(QString notes);
    QString     getNotes();

    QVector<TilingPtr> getTilings();

    static const QString defaultName;

private:
    StyleSet         styleSet;
    QString          name;
    QString          designNotes;
    ModelSettingsPtr settings;
    BorderPtr        border;
};

#endif
