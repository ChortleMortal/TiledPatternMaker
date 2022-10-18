#ifndef MOTIF_SELECTOR
#define MOTIF_SELECTOR

#include <QVector>
#include <QScrollArea>

typedef std::shared_ptr<class MotifButton>      MotifBtnPtr;
typedef std::weak_ptr<class MotifButton>        WeakMotifBtnPtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;

class QGridLayout;
class Configuration;
class MotifMaker;

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

class MotifSelector : public QScrollArea
{
    Q_OBJECT

public:
    MotifSelector();

    void            setup(PrototypePtr proto);
    MotifBtnPtr     getCurrentButton() {return _currentButton.lock(); }
    bool            eventFilter(QObject *watched, QEvent *event) override;
    void            tallyButtons();

signals:
    void            sig_launcherButton(MotifBtnPtr fb, bool add);

public slots:
    void            setCurrentButton(MotifBtnPtr btn, bool add);

protected:
    void            populateMotifButtons(QVector<DesignElementPtr> & dels);
    void            getNextPosition(int index, int & row, int & col);

private:
    MotifMaker             * motifMaker;
    QWidget                * widget;
    QGridLayout            * grid;
    QVector<MotifBtnPtr>	buttons;
    WeakMotifBtnPtr       _currentButton;

    Configuration          * config;
};

#endif

