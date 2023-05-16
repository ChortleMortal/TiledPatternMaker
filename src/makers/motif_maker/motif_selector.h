#pragma once
#ifndef MOTIF_SELECTOR
#define MOTIF_SELECTOR

#include <QVector>
#include <QScrollArea>

typedef std::shared_ptr<class DesignElementButton>  DELBtnPtr;
typedef std::weak_ptr<  class DesignElementButton>  WeakDELBtnPtr;
typedef std::shared_ptr<class Prototype>            ProtoPtr;
typedef std::shared_ptr<class DesignElement>        DesignElementPtr;

class QGridLayout;
class PrototypeData;
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
    Q_OBJECT

public:
    DELSelectorWidget();

    void            setup(ProtoPtr proto);
    DELBtnPtr       getCurrentButton() {return _currentButton.lock(); }
    bool            eventFilter(QObject *watched, QEvent *event) override;
    void            tallyButtons();

signals:
    void            sig_launcherButton(DELBtnPtr fb, bool add);

public slots:
    void            setCurrentButton(DELBtnPtr btn, bool add);

protected:
    void            populateMotifButtons(QVector<DesignElementPtr> & dels);
    void            getNextPosition(int index, int & row, int & col);

private:
    Configuration          * config;
    PrototypeData          * motifViewData;

    QWidget                * widget;
    QGridLayout            * grid;

    QVector<DELBtnPtr>       buttons;
    WeakDELBtnPtr           _currentButton;
};

#endif

