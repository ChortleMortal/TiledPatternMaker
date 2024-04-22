#include <QtWidgets>
#include "makers/motif_maker/design_element_selector.h"
#include "makers/motif_maker/design_element_button.h"
#include "makers/motif_maker/motif_editor_widget.h"
#include "makers/motif_maker/motif_maker_widget.h"
#include "makers/prototype_maker/prototype.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "mosaic/design_element.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "tile/tiling.h"

////////////////////////////////////////////////////////////////////////////
//
// FeatureLauncher.java
//
// A repository for a collection of MotifButtons, kind of like a
// radio group.  Manages the currently active button and enforces
// mutual exclusion.  Exports a signal for telling other objects when
// the active selection changes (MotifMaker uses this to change what's
// being edited).
//
// This class also contains some code to automatically decide what the
// initial figure should be for each feature in a tiling.  This is probably
// a bad idea -- the tiling (or some outside client) should tell you what
// it wants to have by default.  But since I'm controlling the possible
// tilings for now, it's not a big deal, and I can always add the
// functionality later.

// DAC 06NOV2020 - removes creation of DesignElements from here
// This launcher is a view which displays the prototype but does not construct it
// Construction is performed in the MotifMaker

DELSelectorWidget::DELSelectorWidget(MotifMakerWidget * makerWidget)
{
    qRegisterMetaType<DesignElementButton*>();

    maker          = makerWidget;
    config         = Sys::config;
    protoMakerData = Sys::prototypeMaker->getProtoMakerData();

    setWidgetResizable(true);
    setFixedWidth(360);
    //setFrameShape(QFrame::NoFrame);

    widget = new QWidget;
    setWidget(widget);

    grid = new QGridLayout;
    widget->setLayout(grid);

    grid->setSpacing(9);
}

void DELSelectorWidget::setup(ProtoPtr proto)
{
    if (!proto || !proto->getTiling() || proto->getTiling()->getInTiling().count() == 0 || proto->numDesignElements() == 0)
    {
        DELBtnPtr dummy;
        buttons.clear();
        delegate(dummy,false,true);
        return;
    }

    QVector<DesignElementPtr> & dels = proto->getDesignElements();
    populateMotifButtons(dels);
}

void DELSelectorWidget::populateMotifButtons(QVector<DesignElementPtr> & dels)
{
    buttons.clear();
    delegatedButton.reset();

    int idx = 0;
    auto rit = dels.constEnd();

    // reverse iterate
    while(rit != dels.constBegin())
    {
        rit--;
        DELBtnPtr fb = std::make_shared<DesignElementButton>(*rit,idx);
        buttons.push_back(fb);
        fb->setSize( QSize( 130, 130 ) );
        fb->QFrame::installEventFilter(this);

        int row = 0;
        int col = 0;
        getNextPosition(idx,row,col);
        grid->addWidget(fb.get(),row,col,Qt::AlignTop);

        idx++;
    }
}

bool DELSelectorWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        qDebug() << "DELSelectorWidget::eventFilter - button mouse button press";
        DesignElementButton * fb = dynamic_cast<DesignElementButton*>(watched);
        if (fb)
        {
            qDebug() << "MotifSelector::eventFilter - found button";
            for (auto & btn : std::as_const(buttons))
            {
                if (btn.get() == fb)
                {
                    bool add = config->motifMultiView;
                    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
                    if (kms == Qt::SHIFT)
                    {
                        add = true;
                    }
                    bool set = true;
                    if (config->motifMultiView && fb->isDelegated())
                    {
                        set = false;
                    }
                    // this performs complete delegation of maker
                    maker->delegate(btn,add,set);
                    break;
                }
            }
        }
    }
    return QScrollArea::eventFilter(watched, event);
}

void DELSelectorWidget::delegate(DELBtnPtr btn, bool add, bool set)
{
    if (!btn)
        return;

    qDebug() << "DELSelectorWidget::delegate" << btn.get() << "index=" << btn->getIndex();

    // selection
    if (!add)
    {
        for (auto & btn : std::as_const(buttons))
        {
            btn->setSelection(false);
        }
    }
    btn->setSelection(set);

    // delegation
    if (delegatedButton.lock())
    {
        auto oldButton = delegatedButton.lock();
        if (set)
        {
            // setting new delegation, so removing new delegation
            oldButton->setDelegation(false);
        }
    }
    btn->setDelegation(set);
    delegatedButton = btn;
    ensureWidgetVisible(btn.get());
}

DELBtnPtr DELSelectorWidget::getButton(DesignElementPtr del)
{
    for (auto & btn : std::as_const(buttons))
    {
        if (del == btn->getDesignElement())
        {
            return btn;
        }
    }
    return delegatedButton.lock();
}

void DELSelectorWidget::tallyButtons()
{
    for (auto & btn : std::as_const(buttons))
    {
        auto del = btn->getDesignElement();
        QString str;
        if (protoMakerData->isSelected(del))
            str += "dS  ";

        if (protoMakerData->isHidden(MVD_DELEM,del))
            str += "H";
        else
            str += "V";
        btn->setTallyString(str);
        btn->tally();
    }
}

void DELSelectorWidget::getNextPosition(int index, int & row, int & col)
{
#if 0
    if (index > MAX_UNIQUE_TILE_INDEX)
    {
        qWarning() << "Max unique tile limit exceeded:" << index;
        return;
    }
#endif
    if (index & 1)
        col = 1;
    else
        col = 0;
    row = index >> 1;
}
