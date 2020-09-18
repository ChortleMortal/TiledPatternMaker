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

#include "makers/figure_maker/feature_launcher.h"
#include "makers/figure_maker/master_figure_editor.h"
#include "style/style.h"
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

void FeatureLauncher::launch(PrototypePtr proto, TilingPtr tiling)
{
    buttons.clear();

    if (!proto || !tiling)
    {
        FeatureBtnPtr fbp;
        setCurrentButton(fbp);
        return;
    }

    // The tiling keep track of PlacedFeatures, but (right now) it
    // doesn't know what _Feature_s those PlacedFeatures refer to.
    // So we have to uniquify the list of Features
    QVector<FeaturePtr> features = tiling->getUniqueFeatures();
    if (features.size() == 0)
    {
        FeatureBtnPtr fbp;
        setCurrentButton(fbp);
        return;
    }

    // If possible, obtain design elements from existing design,
    // so when "reset with feature" is eventually called, it can deal with
    // an existing figure.
    // we also have to deal with too many or too few design elements

    QVector<DesignElementPtr> & dels = proto->getDesignElements();
    if (dels.size() == 0)
    {
        create(proto,features);
    }
    else
    {
        QVector<DesignElementPtr> unmatchedDels;
        // first verify existing dels are valid
        for (auto del : dels)
        {
            if (!features.contains(del->getFeature()))
            {
                unmatchedDels.push_back(del);
            }
        }
        for (auto del : unmatchedDels)
        {
            proto->removeElement(del);
        }
        // does the feature have a matching del
        for (auto feature : features)
        {
            bool matched = false;
            for (auto del : dels)
            {
                if (feature == del->getFeature())
                {
                    matched = true;
                    break;
                }
            }
            if (!matched)
            {
                DesignElementPtr dep = make_shared<DesignElement>(feature);
                proto->addElement(dep);
            }
        }
    }

    populate(dels);

    Q_ASSERT(buttons.size());
    setCurrentButton(buttons[0]);
}

void FeatureLauncher::create(PrototypePtr proto, QVector<FeaturePtr> & features)
{
    int count = 0;
    for (auto feature : features)
    {
        if (++count <= MAX_UNIQUE_FEATURE_INDEX)
        {
            DesignElementPtr dep = make_shared<DesignElement>(feature);
            proto->addElement(dep);
        }
        else
        {
            qWarning() << "Too many unqique features in tiling. count = " << count;
        }
    }
}

void FeatureLauncher::populate(QVector<DesignElementPtr> & dels)
{
    int idx = 0;
    for (auto del : dels)
    {
        FeatureBtnPtr fb = make_shared<FeatureButton>(del,idx);
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

void FeatureLauncher::populateProtoWithDELsFromButtons(PrototypePtr proto)
{
    proto->getDesignElements().clear();
    for(auto btn : buttons)
    {
        DesignElementPtr dep = btn->getDesignElement();
        FigurePtr        fp  = dep->getFigure();
        Q_ASSERT(fp);
        proto->addElement(dep);
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

bool FeatureLauncher::verify()
{
    bool rv = true;
    for (auto btn : buttons)
    {
        DesignElementPtr del = btn->getDesignElement();
        FigurePtr        fp  = del->getFigure();
        qDebug() << "FeatureLauncher::verify figure =" << fp.get();
        if (!fp)
            rv = false;
    }
    return rv;
}
