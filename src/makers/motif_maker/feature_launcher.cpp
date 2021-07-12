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

#include <QtWidgets>
#include "makers/motif_maker/feature_launcher.h"
#include "makers/motif_maker/master_figure_editor.h"
#include "makers/motif_maker/feature_button.h"
#include "style/style.h"
#include "tapp/prototype.h"
#include "tile/tiling.h"

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

// DAC 06NOV2020 - removes creation of DesignEelements from here
// This launcher is a view which displays the prototype but does not construct it
// Construction is performed in the MotifMaker

FeatureLauncher::FeatureLauncher()
{
    qRegisterMetaType<FeatureButton*>();

    setSpacing(9);
}

void FeatureLauncher::launch(PrototypePtr proto)
{
    if (!proto || !proto->getTiling() || proto->getTiling()->countPlacedFeatures() ==0 || proto->numDesignElements() == 0)
    {
        FeatureBtnPtr dummy;
        setCurrentButton(dummy);
        return;
    }

    QVector<DesignElementPtr> & dels = proto->getDesignElements();
    populateFeatureButtons(dels);

    Q_ASSERT(buttons.size());
    setCurrentButton(buttons[0]);
}

void FeatureLauncher::populateFeatureButtons(QVector<DesignElementPtr> & dels)
{
    buttons.clear();
    _currentButton.reset();

    int idx = 0;
    for (auto del : dels)
    {
        FeatureBtnPtr fb = std::make_shared<FeatureButton>(del,idx);
        buttons.push_back(fb);
        fb->setSize( QSize( 130, 130 ) );
        fb->QFrame::installEventFilter(this);

        int row = 0;
        int col = 0;
        getPosition(idx,row,col);
        addWidget(fb.get(),row,col,Qt::AlignTop);

        idx++;
    }
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
        _currentButton = btn;
        return;
    }
    qDebug() << "FeatureLauncher::setCurrent" << btn.get() << "index=" << btn->getIndex();
    if  (btn != _currentButton.lock())
    {
        if (_currentButton.lock())
        {
            _currentButton.lock()->setSelected( false );
            _currentButton.lock()->setStyleSheet("background-color: white;");
        }

        _currentButton = btn;
        if (_currentButton.lock())
        {
            _currentButton.lock()->setSelected( true );
            _currentButton.lock()->setStyleSheet("background-color: rgb(255,240,240);");
        }

        emit sig_launcherButton();
    }
}

void FeatureLauncher::getPosition(int index, int & row, int & col)
{
    if (index > MAX_UNIQUE_FEATURE_INDEX)
    {
        qWarning() << "Max unique feature limit exceeded:" << index;
        return;
    }
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
