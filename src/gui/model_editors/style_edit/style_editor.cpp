#include "gui/model_editors/style_edit/style_editor.h"
#include "gui/model_editors/style_edit/fill_editor.h"
#include "model/styles/style.h"

StyleEditor::StyleEditor(StylePtr style, eStyleType user) : QWidget()
{
    wStyle     = style;
    this->user = user;
    setable    = nullptr;

    setContentsMargins(0,0,0,0);

    SystemViewController * vcontrol = Sys::viewController;
    
    connect(this, &StyleEditor::sig_reconstructView, vcontrol, &SystemViewController::slot_reconstructView);
    connect(this, &StyleEditor::sig_updateView,      vcontrol, &SystemViewController::slot_updateView);
}

void StyleEditor::notify()
{
    switch (user)
    {
    case STYLE_PLAIN:
    case STYLE_SKETCHED:
    case STYLE_THICK:
        emit sig_updateView();
        break;

    case STYLE_INTERLACED:
    case STYLE_EMBOSSED:
    case STYLE_OUTLINED:
    case STYLE_TILECOLORS:
        if (auto style = wStyle.lock())
            style->resetStyleRepresentation();
        emit sig_reconstructView();
        break;

        // overloaded
        break;

    case STYLE_FILLED:
    {
        FilledEditor * filledEditor = static_cast<FilledEditor*>(this);
        filledEditor->notify();
    }   break;

    case STYLE_BORDER:
    case STYLE_NONE:
        break;
    }
}
