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

#ifndef FEATURE_LAUNCHER
#define FEATURE_LAUNCHER


#include <QVector>
#include <QtWidgets>
#include <QWidget>

typedef std::shared_ptr<class FeatureButton>       FeatureBtnPtr;
typedef std::weak_ptr<class FeatureButton>         WeakFeatureBtnPtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;

////////////////////////////////////////////////////////////////////////////
//
// FeatureLauncher.java
//
// A repository for a collection of FeatureButtons, kind of like a
// radio group.  Manages the currently active button and enforces
// mutual exclusion.  Exports a signal for telling other objects when
// the active selection changes (FigureMaker uses this to change what's
// being edited).
//
// This class also contains some code to automatically decide what the
// initial figure should be for each feature in a tiling.  This is probably
// a bad idea -- the tiling (or some outside client) should tell you what
// it wants to have by default.  But since I'm controlling the possible
// tilings for now, it's not a big deal, and I can always add the
// functionality later.

class FeatureLauncher : public QGridLayout
{
    Q_OBJECT

public:
    FeatureLauncher();

    void            launch(PrototypePtr proto);
    FeatureBtnPtr   getCurrentButton() {return _currentButton.lock(); }
    bool            eventFilter(QObject *watched, QEvent *event) override;

signals:
    void            sig_launcherButton();

public slots:
    void            setCurrentButton(FeatureBtnPtr btn);

protected:
    void            populateFeatureButtons(QVector<DesignElementPtr> & dels);
    void            getPosition(int index, int & row, int & col);

private:
    QVector<FeatureBtnPtr>	buttons;
    WeakFeatureBtnPtr       _currentButton;
};

#endif

