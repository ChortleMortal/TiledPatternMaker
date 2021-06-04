#ifndef STYLESETVIEW_H
#define STYLESETVIEW_H

#include "base/layer.h"

class StyleSetView : public Layer
{
public:
    static StyleSetView * getInstance();

private:
    StyleSetView();

    static StyleSetView * mpThis;
};

#endif // STYLESETVIEW_H
