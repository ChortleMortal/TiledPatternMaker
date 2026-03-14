#include <QHeaderView>
#include <QComboBox>
#include <QCheckBox>

#include "gui/model_editors/style_edit/fill_editor.h"
#include "gui/model_editors/style_edit/fill_face_editor.h"
#include "gui/model_editors/style_edit/fill_group_editor.h"
#include "gui/model_editors/style_edit/fill_new1_editor.h"
#include "gui/model_editors/style_edit/fill_original_editor.h"
#include "gui/model_editors/style_edit/fill_set_editor.h"
#include "gui/widgets/colorset_widget.h"
#include "gui/widgets/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "model/styles/fill_color_maker.h"
#include "model/styles/filled.h"
#include "sys/engine/image_engine.h"
#include "sys/sys.h"

#define ROW_HEIGHT 39

////////////////////////////////////////////////////////////////////////////
///
///  Filled Editor
///
////////////////////////////////////////////////////////////////////////////
///
FilledEditor::FilledEditor(StylePtr style, eStyleType user) : StyleEditor(style, user)
{
    auto filled  = std::dynamic_pointer_cast<Filled>(style);
    wfilled      = filled;
    Q_ASSERT(wfilled.lock());

    currentEditor = nullptr;
    panel         = Sys::controlPanel;

    QLabel * label = new QLabel("Algorithm");
    label->setMinimumHeight(31);

    algoBox = new QComboBox();
    algoBox->setMinimumHeight(31);
    algoBox->addItem(sFillType[FILL_ORIGINAL],                  FILL_ORIGINAL);
    algoBox->addItem(sFillType[FILL_TWO_FACE],                  FILL_TWO_FACE);
    algoBox->addItem(sFillType[FILL_MULTI_FACE],                FILL_MULTI_FACE);
    algoBox->addItem(sFillType[FILL_MULTI_FACE_MULTI_COLORS],   FILL_MULTI_FACE_MULTI_COLORS);
    algoBox->addItem(sFillType[FILL_FACE_DIRECT],               FILL_FACE_DIRECT);

    hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(label);
    hbox->addWidget(algoBox);
    hbox->addStretch();

    vbox = new QVBoxLayout();

    QVBoxLayout * vbox2 = new QVBoxLayout();
    vbox2->addLayout(hbox);
    vbox2->addLayout(vbox);
    this->setLayout(vbox2);

    eFillType algo  = filled->getAlgorithm();
    int index       = algoBox->findData(algo);
    if (index != 1)
    {
        algoBox->setCurrentIndex(index);
    }

    createSubTypeEditor(algo);

    connect(algoBox, &QComboBox::currentIndexChanged, this, [this] { slot_algo((eFillType)algoBox->currentData().toInt());} );
}

FilledEditor::~FilledEditor()
{
    delete currentEditor;
    panel->restorePageStatus();
}

void FilledEditor::createSubTypeEditor(eFillType algo)
{
    qDebug() << "FilledEditor::createSubType" << algo;

    panel_misc::eraseLayout(dynamic_cast<QLayout*>(vbox));

    if (currentEditor)
    {
        delete currentEditor;
        currentEditor = nullptr;
    }

    FilledPtr filled = wfilled.lock();
    if (!filled) return;

    int index = algoBox->findData(algo);
    if (index != -1)
    {
        algoBox->blockSignals(true);
        algoBox->setCurrentIndex(index);
        algoBox->blockSignals(false);

        switch (algo)
        {
        case FILL_ORIGINAL:
            panel->restorePageStatus();
            currentEditor = new FillOriginalEditor(this, filled, &filled->original, vbox);
            break;

        case FILL_TWO_FACE:
            panel->restorePageStatus();
            currentEditor = new FillNew1Editor(this, filled, &filled->new1, vbox);
            break;

        case FILL_MULTI_FACE:
            panel->restorePageStatus();
            currentEditor = new FillSetEditor(this,filled, &filled->new2, vbox);
            break;

        case FILL_MULTI_FACE_MULTI_COLORS:
            panel->overridePagelStatus("Click on face in image to select row in table. Double-click to edit directly");
            currentEditor = new FillGroupEditor(this, filled, &filled->new3, vbox);
            break;

        case FILL_FACE_DIRECT:
            panel->overridePagelStatus("Select palette color in table, then left-click mosaic faces (right-click to erase)");
            currentEditor = new FillFaceEditor(this, filled, & filled->direct, vbox);
            break;

        case DEPRECATED_FILL_FACE_DIRECT:
            break;
        }
    }

    if (currentEditor)
    {
        currentEditor->refresh();
        updateGeometry();
    }
}

void FilledEditor::slot_algo(eFillType algo)
{
    qDebug() << "FilledEditor::slot_algo" << algo;
    auto filled = wfilled.lock();
    if (!filled) return;

    Q_ASSERT(algo != DEPRECATED_FILL_FACE_DIRECT);

    eFillType old = filled->getAlgorithm();
    filled->setAlgorithm(algo);
    filled->initAlgorithmFrom(old);         // signals a change
    filled->resetStyleRepresentation();

    createSubTypeEditor(algo);

    emit sig_reconstructView();
}

void FilledEditor::slot_colorsChanged()
{
    emit sig_updateView();
}

void FilledEditor::onEnter()
{
    connect(Sys::sysview,     &SystemView::sig_mousePressed,   this, &FilledEditor::slot_mousePressed);
    connect(Sys::imageEngine, &ImageEngine::sig_colorPick,     this, &FilledEditor::slot_colorPick, Qt::QueuedConnection);
}

void FilledEditor::onExit()
{
    disconnect(Sys::sysview,     &SystemView::sig_mousePressed, this, &FilledEditor::slot_mousePressed);
    disconnect(Sys::imageEngine, &ImageEngine::sig_colorPick,   this, &FilledEditor::slot_colorPick);
}

void FilledEditor::onRefresh()
{
    if (currentEditor)
    {
        currentEditor->refresh();
    }
    else
    {
        FilledPtr filled = wfilled.lock();
        if (!filled) return;
        auto algo = filled->getAlgorithm();
        createSubTypeEditor(algo);
    }
}

void FilledEditor::notify()
{
    if (currentEditor)
    {
        currentEditor->notify();
    }
}

void FilledEditor::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    auto filled = wfilled.lock();
    if (!filled) return;

    QPointF mpt = filled->screenToModel(spt);
    qDebug() << "FilledEditor" << spt << mpt;

    if (currentEditor)
    {
        currentEditor->mousePressed(mpt,btn);
    }
}

// called by mouse click on a image tools popup menu
void FilledEditor::slot_colorPick(QColor color)
{
    qDebug() << "slot_colorPick" << color;

    if (currentEditor)
    {
        currentEditor->colorPick(color);
        emit sig_updateView();
    }
}

FilledSubTypeEditor::FilledSubTypeEditor(FilledEditor * parent, FilledPtr filled, ColorMaker * cm)
{
    this->parent  = parent;
    this->wfilled = filled;
    this->cm      = cm;
}

FilledSubTypeEditor::~FilledSubTypeEditor()
{}


