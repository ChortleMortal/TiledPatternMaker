#include "viewers/style_set_view.h"

StyleSetView * StyleSetView::mpThis = nullptr;

StyleSetView * StyleSetView::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new StyleSetView;
    }
    return mpThis;
}

StyleSetView::StyleSetView() : Layer("Style Set",LTYPE_VIEW)
{
}
