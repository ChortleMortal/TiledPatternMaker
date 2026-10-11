#pragma once

#include <QCheckBox>
#include <QVBoxLayout>
#include "model/motifs/extender.h"

class SliderSet;
class SpinSet;
class DoubleSliderSet;
class MotifEditor;

typedef std::weak_ptr<class Extender>       WeakExtenderPtr;

class ExtenderEditor : public QWidget
{
    Q_OBJECT;

public:
    ExtenderEditor(ExtenderPtr extender)  { wextender = extender; }

    QVBoxLayout *   createExtenderLayout(MotifEditor * parent);
    void            editorToExtender();
    void            refresh();

private:
    SpinSet         * boundarySides;
    DoubleSliderSet	* boundaryScale;
    DoubleSliderSet	* boundaryRotate;

    QCheckBox       * extendRaysChk;
    QCheckBox       * extendTipsToBoundChk;
    QCheckBox       * extendTipsToTileChk;
    QCheckBox       * embedBoundaryChk;
    QCheckBox       * embedTileChk;
    SpinSet         * connectRaysBox;
    
    SpinSet         * stellationBox;
    QCheckBox       * fitChk;

    MotifEditor     * editor;
    WeakExtenderPtr   wextender;
};

