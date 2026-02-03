#pragma once
#ifndef MOTIF_SELECTOR
#define MOTIF_SELECTOR

#include <QVector>
#include <QScrollArea>

typedef std::shared_ptr<class DesignElementButton>  DELBtnPtr;
typedef std::weak_ptr<  class DesignElementButton>  WeakDELBtnPtr;
typedef std::shared_ptr<class Prototype>            ProtoPtr;
typedef std::shared_ptr<class DesignElement>        DELPtr;

class MotifMakerWidget;
class QGridLayout;
class Configuration;

////////////////////////////////////////////////////////////////////////////
//
// FeatureLauncher.java
//
// A repository for a collection of DesignElementButtons, kind of like a
// radio group.  Manages the currently active button and enforces
// mutual exclusion (or not).  Exports a signal for telling other objects when
// the active selection changes (MotifMaker uses this to change what's
// being edited).
//
// This class also contains some code to automatically decide what the
// initial figure should be for ea4ch feature in a tiling.  This is probably
// a bad idea -- the tiling (or some outside client) should tell you what
// it wants to have by default.  But since I'm controlling the possible
// tilings for now, it's not a big deal, and I can always add the
// functionality later.

class DELSelectorWidget : public QScrollArea
{
public:
    DELSelectorWidget(MotifMakerWidget * makerWidget);

    void            setup(ProtoPtr proto);

    bool            eventFilter(QObject *watched, QEvent *event) override;

    void            delegate(DELBtnPtr btn, bool add, bool set);
    DELBtnPtr       getDelegated() {return delegatedButton.lock(); }

    void            tallyButtons();

    DELBtnPtr       getButton(DELPtr del);

    void            update();

protected:
    void            getNextPosition(int index, int & row, int & col);

private:
    Configuration          * config;
    MotifMakerWidget       * maker;

    QWidget                * widget;
    QGridLayout            * grid;

    QVector<DELBtnPtr>       buttons;
    WeakDELBtnPtr            delegatedButton;
};

#endif

