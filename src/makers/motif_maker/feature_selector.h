#ifndef FEATURE_LAUNCHER
#define FEATURE_LAUNCHER

#include <QVector>
#include <QScrollArea>

typedef std::shared_ptr<class FeatureButton>       FeatureBtnPtr;
typedef std::weak_ptr<class FeatureButton>         WeakFeatureBtnPtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;

class QGridLayout;
class Configuration;
class MotifMaker;

////////////////////////////////////////////////////////////////////////////
//
// FeatureLauncher.java
//
// A repository for a collection of FeatureButtons, kind of like a
// radio group.  Manages the currently active button and enforces
// mutual exclusion.  Exports a signal for telling other objects when
// the active selection changes (FigureMaker uses this to change what's
// being edited).
//
// This class also contains some code to automatically decide what the
// initial figure should be for each feature in a tiling.  This is probably
// a bad idea -- the tiling (or some outside client) should tell you what
// it wants to have by default.  But since I'm controlling the possible
// tilings for now, it's not a big deal, and I can always add the
// functionality later.

class FeatureSelector : public QScrollArea
{
    Q_OBJECT

public:
    FeatureSelector();

    void            setup(PrototypePtr proto);
    FeatureBtnPtr   getCurrentButton() {return _currentButton.lock(); }
    bool            eventFilter(QObject *watched, QEvent *event) override;
    void            tallyButtons();

signals:
    void            sig_launcherButton(FeatureBtnPtr fb, bool add);

public slots:
    void            setCurrentButton(FeatureBtnPtr btn, bool add);

protected:
    void            populateFeatureButtons(QVector<DesignElementPtr> & dels);
    void            getNextPosition(int index, int & row, int & col);

private:
    MotifMaker             * motifMaker;
    QWidget                * widget;
    QGridLayout            * grid;
    QVector<FeatureBtnPtr>	buttons;
    WeakFeatureBtnPtr       _currentButton;

    Configuration          * config;
};

#endif

