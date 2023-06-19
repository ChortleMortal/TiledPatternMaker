#include <QtWidgets>
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/motif_maker/motif_selector.h"
#include "makers/motif_maker/motif_editor_widget.h"
#include "makers/motif_maker/design_element_button.h"
#include "makers/prototype_maker/prototype.h"
#include "style/style.h"
#include "tile/tiling.h"
#include "mosaic/design_element.h"
#include "settings/configuration.h"

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

DELSelectorWidget::DELSelectorWidget()
{
    qRegisterMetaType<DesignElementButton*>();

    config        = Configuration::getInstance();
    motifViewData = PrototypeMaker::getInstance()->getProtoMakerData();

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
        setCurrentButton(dummy,false);
        return;
    }

    QVector<DesignElementPtr> & dels = proto->getDesignElements();
    populateMotifButtons(dels);

    Q_ASSERT(buttons.size());
    setCurrentButton(buttons[0],false);
}

void DELSelectorWidget::populateMotifButtons(QVector<DesignElementPtr> & dels)
{
    buttons.clear();
    _currentButton.reset();

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
        DesignElementButton * fb = dynamic_cast<DesignElementButton*>(watched);
        if (fb)
        {
            qDebug() << "MotifSelector::eventFilter - found button";
            for (auto it = buttons.begin(); it != buttons.end(); it++)
            {
                DELBtnPtr fbp = *it;
                if (fbp.get() == fb)
                {
                    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
                    if (kms == Qt::SHIFT || config->motifMultiView)
                        setCurrentButton(fbp,true);
                    else
                        setCurrentButton(fbp,false);
                }
            }
        }
    }
    return QScrollArea::eventFilter(watched, event);
}

void DELSelectorWidget::setCurrentButton(DELBtnPtr btn, bool add)
{
    if (btn)
        qDebug() << "MotifSelector::setCurrent" << btn.get() << "index=" << btn->getIndex();

    _currentButton = btn;

    emit sig_launcherButton(btn,add);
}

void DELSelectorWidget::tallyButtons()
{
    auto dels =  motifViewData->getSelectedDELs(MVD_DELEM);
    for (auto btn : buttons)
    {
        auto del = btn->getDesignElement();
        btn->tally(dels.contains(del));
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
