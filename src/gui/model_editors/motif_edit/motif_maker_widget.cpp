#include "gui/model_editors/motif_edit/motif_editor_widget.h"
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "gui/model_editors/motif_edit/tile_motif_button.h"
#include "gui/top/system_view.h"
#include "gui/top/view_controller.h"
#include "model/makers/motif_maker.h"
#include "model/prototypes/prototype.h"
#include "model/prototypes/tile_motif.h"
#include "model/settings/configuration.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/transform.h"

typedef std::weak_ptr<Tiling> WeakTilingPtr;

MotifMakerWidget::MotifMakerWidget() : QWidget()
{
    qRegisterMetaType<TileMotifButton*>();

    setContentsMargins(0,0,0,0);

    buttonBox = new QScrollArea();
    buttonBox->setWidgetResizable(true);
    buttonBox->setFixedWidth(360);

    QWidget * buttonSelectionWidget = new QWidget();
    buttonSelectGrid = new QGridLayout;
    buttonSelectGrid->setSpacing(9);
    buttonSelectionWidget->setLayout(buttonSelectGrid);

    buttonBox->setWidget(buttonSelectionWidget);

    // larger slected feature button
    viewerBtn  = new TileMotifButton(-1);
    viewerBtn->setFixedSize(310,290);

    // top row
    QHBoxLayout * btnBox = new QHBoxLayout();
    btnBox->addWidget(buttonBox);
    btnBox->addSpacing(25);
    btnBox->addWidget(viewerBtn);
    btnBox->addStretch();

    // Editors
    motifEditorWidget  = new MotifEditorWidget(this);

    AQVBoxLayout * motifBox = new AQVBoxLayout;
    motifBox->addLayout(btnBox);
    motifBox->addWidget(motifEditorWidget);

    setLayout(motifBox);
    setMinimumWidth(610);

    connect(this,                      &MotifMakerWidget::sig_updateView,        Sys::systemViewController, &ViewController::slot_updateView);
    connect(this,                      &MotifMakerWidget::sig_scrollToDelegated, this, &MotifMakerWidget::slot_scrollToDelegated, Qt::QueuedConnection);
    connect(Sys::motifMakerView.get(), &MotifMakerView::sig_delegateTileMotif,   this, &MotifMakerWidget::slot_delegateTileMotif);
}

// called by button press on motif view
void MotifMakerWidget::slot_delegateTileMotif(TileMotifPtr timo)
{
    auto btn = getButton(timo);
    delegate(btn,true);
}

TileMotifBtnPtr MotifMakerWidget::getButton(TileMotifPtr timo)
{
    for (auto & btn : std::as_const(buttons))
    {
        if (timo == btn->getTileMotif())
        {
            return btn;
        }
    }
    return nullptr;
}

TileMotifBtnPtr MotifMakerWidget::getDelegatedButton()
{
    auto timo = Sys::motifMaker->motifData.getDelegatedTileMotif();
    auto btn  = getButton(timo);
    return btn;
}

// Runs queued (see constructor), after any pending show()/layout work in the same call chain -
// e.g. the hosting page's own show() called right after onRefresh() - so the button geometry
// ensureWidgetVisible() relies on is valid by the time this actually runs.
void MotifMakerWidget::slot_scrollToDelegated()
{
    auto btn = getDelegatedButton();
    if (!btn)
        return;

    buttonSelectGrid->activate();
    buttonBox->widget()->adjustSize();     // sync resize so the scroll area's scrollbar range is current
    buttonBox->ensureWidgetVisible(btn.get());
}

void MotifMakerWidget::setupViewMode(MotifMakerView::ViewType viewMode)
{
    if (buttons.size() == 0)
        return; // nothing to do

    if (viewMode == MotifMakerView::ViewType::Undefined)
        viewMode = MotifMakerView::ViewType::Single;

    Sys::config->motifMakerViewType = viewMode;

    MotifViewData & data = Sys::motifMaker->motifData;
    if (data.numTileMotifs() == 0)
        return; // nothing to do

    TileMotifPtr    timo = data.getDelegatedTileMotif(); // could be null
    TileMotifBtnPtr delegatedButton;
    if (timo)
        delegatedButton = getButton(timo);              // could bu null

    if (!delegatedButton)
    {
        delegatedButton = buttons.first();
        timo = delegatedButton->getTileMotif();
        for (auto & timo2 : data.getTileMotifs())
        {
            data.hide(timo2);
        }
    }

    Sys::motifMakerView->setViewMode(viewMode);

    switch(viewMode)
    {
    case  MotifMakerView::ViewType::Single:
        for (auto & timo2 : data.getTileMotifs())
        {
            data.hide(timo2);
        }
        delegate(delegatedButton,true);
        break;

    case  MotifMakerView::ViewType::Multiple:
        delegate(delegatedButton,true);
        break;

    case  MotifMakerView::ViewType::All:
        for (auto & timo2 : data.getTileMotifs())
        {
            data.show(timo2);
        }
        delegate(delegatedButton,true);
        break;

    case  MotifMakerView::ViewType::Undefined:
        break;
    }
}

void MotifMakerWidget::refresh()
{
    if (_contentHasChanged())
    {
        auto proto  = Sys::motifMaker->motifData.getActivePrototype();
        Q_ASSERT(proto);

        // creates a new TileMotifButton for all TileMotifs in the prototype
        setup(proto);

        // sort out visibility and delegation
        setupViewMode(Sys::config->motifMakerViewType);
    }

    // Refresh everything driven by the timer
    // It would be better if this refresh was event drive by "doEmit" but
    // this is difficult becuase of the class hierarchy
    for (auto & btn : buttons)
    {
        btn->update();
    }
    motifEditorWidget->refresh();
    viewerBtn->update();
    QWidget::update();
}

// called by MotifMakerWidget and when the prototype. number of motifs, or tiling changes.
// also called when config->motifMakerViewType is changed
// when a TileMotif button is selected, this delgates the motif in both the motif maker and the motif editor
void MotifMakerWidget::delegate(TileMotifBtnPtr btn, bool modeswitch)
{
    // delegation of the TileMotif which the button represents
    if (!btn)
    {
        qWarning("No TileMotif button to delgate");
        return;
    }

    TileMotifPtr timo    = btn->getTileMotif(); // DAC taprats cloned here
    if (!timo) return;

    MotifMaker * maker   = Sys::motifMaker;
    MotifViewData & data = maker->motifData;

    if (!modeswitch && data.isDelegated(timo) && Sys::config->motifMakerViewType ==  MotifMakerView::ViewType::Multiple)
    {
        if (data.numVisibleTileMotifs() > 1)
        {
            data.hide(timo);

            // select another button to delegate
            for (auto & button : buttons)
            {
                auto timo2 = button->getTileMotif();
                if (timo2 != timo  && data.isVisible(timo2))
                {
                    btn  = button;
                    timo = timo2;
                    break;
                }
            }
        }
    }

    // TileMotif Viewer button - do this first
    viewerBtn->setTileMotif(timo);

    // Prototype Maker data
    Sys::motifMaker->motifData.setDelegatedTileMotif(timo);
    Sys::motifMaker->motifData.show(timo);

    // enlarge if in Single mode
    if (Sys::config->motifMakerViewType == MotifMakerView::ViewType::Single)
    {
        Sys::motifMakerView->enlarge(timo);
    }

    // TileMotif selector
    btn->update();

    // Motif Editor Widget
    motifEditorWidget->delegateComboAndLowerThird(timo);

    // this has already fully handled the motif switch - record it so the next poll tick's
    // _contentHasChanged() doesn't see it as a fresh change and redundantly redo this same
    // rebuild a second time (that double rebuild was the cause of a visible flash)
    lastDelegatedMotif = timo->getMotif();

    tallyButtons();

    emit sig_updateView();
    emit sig_scrollToDelegated();
}

bool MotifMakerWidget::_contentHasChanged()
{
    static WeakProtoPtr     wLastProto;
    static WeakTilingPtr    wLastTiling;
    static int              lastBtnCount = 0;

    auto lastProto  = wLastProto.lock();
    auto lastTiling = wLastTiling.lock();
    auto lastMotif  = lastDelegatedMotif.lock();

    auto pmaker     = Sys::motifMaker;
    auto proto      = pmaker->motifData.getActivePrototype();
    Q_ASSERT(proto);
    auto tiling     = proto->getTiling();

    auto delegated       = pmaker->motifData.getDelegatedTileMotif();
    auto delegatedMotif  = delegated ? delegated->getMotif() : nullptr;

    if (      proto
        && (    pmaker->forceWidgetRefresh()
            || (lastProto    != proto)
            || (lastTiling   != tiling)
            || (lastBtnCount != proto->numTileMotifs())
            || (lastMotif    != delegatedMotif)))         // catches an in-place motif swap (e.g. Map Editor push, type change)
    {
        // change detected: differen proto or num DELs or different tiling
        pmaker->setForceWidgetRefresh(false);
        wLastProto         = proto;
        wLastTiling        = tiling;
        lastBtnCount       = proto->numTileMotifs();
        lastDelegatedMotif = delegatedMotif;
        return true;
    }
    return false;
}

void MotifMakerWidget::setup(ProtoPtr proto)
{
    if (!proto || !proto->getTiling() || proto->getTiling()->unit().numIncluded() == 0 || proto->numTileMotifs() == 0)
    {
        buttons.clear();
        return;
    }

    // creates a new TileMotifButton for all TileMotifs in the prototype
    QVector<TileMotifPtr> & timos = proto->getTileMotifs();
    auto tiling = proto->getTiling();

    buttons.clear();
    //delegatedButton.reset();

    int idx = 0;
    auto rit = timos.constEnd();

    // reverse iterate
    while(rit != timos.constBegin())
    {
        rit--;

        // only use rotation from first placement, since scale and position are backed out
        // in the tile motif button which tries to center the del in its window
        TileMotifPtr timo = *rit;
        QTransform t       = tiling->unit().getFirstPlacement(timo->getTile());
        qreal rotation     = Transform::rotation(t);

        TileMotifBtnPtr fb   = std::make_shared<TileMotifButton>(idx,timo,Transform::rotate(rotation));
        buttons.push_back(fb);
        fb->setFixedSize( QSize( 130, 130 ) );
        fb->QFrame::installEventFilter(this);

        int row = 0;
        int col = 0;
        getNextPosition(idx,row,col);
        buttonSelectGrid->addWidget(fb.get(),row,col,Qt::AlignTop);

        idx++;
    }
}

TileMotifBtnPtr MotifMakerWidget::findBtn(TileMotifButton * fb)
{
    TileMotifBtnPtr del;
    for (auto & btn : buttons)
    {
        if (btn.get() == fb)
        {
            del = btn;
            break;
        }
    }
    return del;
}

bool MotifMakerWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        qDebug() << "MotifMakerWidget::eventFilter - button mouse button press";
        TileMotifButton * fb = dynamic_cast<TileMotifButton*>(watched);
        TileMotifBtnPtr btn = findBtn(fb);
        if (btn)
        {
            delegate(btn,false);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MotifMakerWidget::tallyButtons()
{
    for (auto & btn : std::as_const(buttons))
    {
        auto timo = btn->getTileMotif();
        QString str;
        if (Sys::motifMaker->motifData.isDelegated(timo))
            str += "Dlg ";

        if (Sys::motifMaker->motifData.isHidden(timo))
            str += "Hid";
        else
            str += "Vis";
        btn->setTallyString(str);
        btn->tally();
    }
}

void MotifMakerWidget::getNextPosition(int index, int & row, int & col)
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
