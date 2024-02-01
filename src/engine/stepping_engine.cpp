#include "engine/stepping_engine.h"
#include "engine/image_engine.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"

int  SteppingEngine::runCount = 0;
bool SteppingEngine::paused   = false;

SteppingEngine::SteppingEngine(ImageEngine * parent)
{
    this->parent = parent;

    config  = Configuration::getInstance();
    panel   = ControlPanel::getInstance();

    started = false;
    paused  = false;

    connect(parent, &ImageEngine::sig_tick, this,  [this]() { tick(); });
    connect(parent, &ImageEngine::sig_next, this,  [this]() { next(); });
    connect(parent, &ImageEngine::sig_end,  this,  [this]() { end(); });
    connect(parent, &ImageEngine::sig_pause,this,  []()     { pause(); });
}

void SteppingEngine::start(bool enb)
{
    if (enb == true && !started)
    {
        started = true;
        runCount++;
    }
    else if (enb == false && started)
    {
        started = false;
        runCount--;
    }
}

void SteppingEngine::finish(QString name)
{
    QMessageBox box(ControlPanel::getInstance());
    box.setIcon(QMessageBox::Information);
    box.setText(QString("%1 completed").arg(name));
    box.exec();

    emit parent->sig_closeAllImageViewers();
}
