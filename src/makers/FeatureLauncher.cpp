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

#include "FeatureLauncher.h"
#include "master_figure_editor.h"
#include "style/Style.h"
#include <QtWidgets>

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

FeatureLauncher::FeatureLauncher()
{
    qRegisterMetaType<FeatureButton*>();

    //setGeometry(QRect(0,0,300,300));
}

FeatureBtnPtr  FeatureLauncher::launchFromPrototype(PrototypePtr proto)
{
    if (!proto)
    {
        FeatureBtnPtr fbp;
        return fbp;
    }

    tiling = proto->getTiling();

    buttons.clear();

    if (!tiling)
    {
        FeatureBtnPtr fbp;
        return fbp;
    }

    // The tiling keep track of PlacedFeatures, but (right now) it
    // doesn't know what _Feature_s those PlacedFeatures refer to.
    // So we have to uniquify the list of Features

    QList<FeaturePtr> fs = tiling->getUniqueFeatures();
    int iFeatures     = fs.size();

    // If possible, obtain design elements from existing design,
    // so when "reset with feature" is eventually called, it can deal with
    // an existing figure.

    QVector<DesignElementPtr> & dels = proto->getDesignElements();
    int iDels = proto->getDesignElements().size();

    int row = 0;
    int col = 0;
    for( int idx = 0; idx <iFeatures; ++idx )
    {
        FeatureBtnPtr fb;
        FeaturePtr f  = fs[idx];
        if (idx < iDels)
        {
            // use existing figure
            fb = make_shared<FeatureButton>(make_shared<DesignElement>(f,dels[idx]->getFigure()),idx);
        }
        else
        {
            fb = make_shared<FeatureButton>(make_shared<DesignElement>(f),idx);
        }
        buttons.push_back(fb);
        fb->setSize( QSize( 130, 130 ) );
        fb->installEventFilter(this);

        getPosition(idx, row, col);
        addWidget(fb.get(),row,col,Qt::AlignTop);
    }

    Q_ASSERT(buttons.size());
    return(buttons[0]);
}

// DAC added
FeatureBtnPtr  FeatureLauncher::launchFromStyle(StylePtr style)
{
    buttons.clear();

    if (!style)
    {
        FeatureBtnPtr fbp;
        return fbp;
    }

    PrototypePtr pp = style->getPrototype();
    Q_ASSERT(pp);
    QVector<DesignElementPtr> & dels = pp->getDesignElements();
    QList<FeaturePtr> fs;
    int idx = 0;
    int row = 0;
    int col = 0;

    for (auto it = dels.begin(); it != dels.end(); it++)
    {
        DesignElementPtr delp = *it;
        FeaturePtr fp = delp->getFeature();
        fs.push_back(fp);

        FeatureBtnPtr fb = make_shared<FeatureButton>(delp,idx++);
        buttons.push_back(fb);
        fb->setSize( QSize( 130, 130 ) );
        fb->installEventFilter(this);

        getPosition(idx-1,row,col);
        addWidget(fb.get(),row,col,Qt::AlignTop);
    }

    Q_ASSERT(buttons.size());
    return(buttons[0]);
}

bool FeatureLauncher::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        FeatureButton * fb = dynamic_cast<FeatureButton*>(watched);
        if (fb)
        {
            qDebug() << "FeatureLauncher::eventFilter - found button";
            for (auto it = buttons.begin(); it != buttons.end(); it++)
            {
                FeatureBtnPtr fbp = *it;
                if (fbp.get() == fb)
                {
                    setCurrentButton(fbp);
                }
            }
        }
    }
    return QGridLayout::eventFilter(watched, event);
}

void FeatureLauncher::setCurrentButton(FeatureBtnPtr btn)
{
    if (!btn)
    {
        _current = btn;
        return;
    }
    qDebug() << "FeatureLauncher::setCurrent" << btn.get() << "index=" << btn->getIndex();
    if  (btn != _current.lock())
    {
        if (_current.lock())
        {
            _current.lock()->setSelected( false );
            _current.lock()->setStyleSheet("background-color: white;");
        }

        _current = btn;
        if (_current.lock())
        {
            _current.lock()->setSelected( true );
            _current.lock()->setStyleSheet("background-color: rgb(255,240,240);");
        }

        emit sig_launcherButton();
    }
}

void FeatureLauncher::getPosition(int index, int & row, int & col)
{
    Q_ASSERT(index < 8);
    if (index < 4)
    {
        row = index;
        col = 0;
    }
    else
    {
        row = index-4;
        col = 1;
    }
}

