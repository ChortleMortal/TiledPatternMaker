#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>

#include "model/motifs/motif.h"
#include "gui/viewers/motif_maker_view.h"

typedef std::shared_ptr<class TileMotif>       TileMotifPtr;
typedef std::shared_ptr<class TileMotifButton> TileMotifBtnPtr;
typedef std::shared_ptr<class Motif>           MotifPtr;
typedef std::weak_ptr<class Motif>             WeakMotifPtr;
typedef std::shared_ptr<class Prototype>       ProtoPtr;
typedef std::weak_ptr<class Prototype>         WeakProtoPtr;

class MotifMaker;
class MotifEditorWidget;

// This widget is housed by page_motif_maker.
// It provides selection and display of TileMotifs
// and is driven by data in Motif Maker motifData

// This widget coordinates delegation/selection of
//  1. Motif View Button
//  2. Motif Selection Button
//  3. Lower Third Motif Editor
//  4. Motif Motif Maker View

class MotifMakerWidget : public QWidget
{
    Q_OBJECT

public:
    MotifMakerWidget();

    void            refresh();
    void            setupViewMode(MotifMakerView::ViewType viewMode);
    void            delegate(TileMotifBtnPtr btn, bool modeswitch);

    TileMotifBtnPtr getButton(TileMotifPtr timo);
    TileMotifBtnPtr getDelegatedButton();           // fetches from tileMotifSelector

    void            setup(ProtoPtr proto);

    bool            eventFilter(QObject *watched, QEvent *event) override;
    void            tallyButtons();

    const QVector<TileMotifBtnPtr> & getButtons() { return buttons; }

    void            scrollToDelegated() { emit sig_scrollToDelegated(); }   // for callers outside delegate(), e.g. page re-entry

signals:
    void            sig_updateView();
    void            sig_scrollToDelegated();        // queued: fires after this call chain's pending show()/layout work is done

public slots:
    void            slot_delegateTileMotif(TileMotifPtr timo);

private slots:
    void            slot_scrollToDelegated();

protected:
    void            getNextPosition(int index, int & row, int & col);
    TileMotifBtnPtr findBtn(TileMotifButton * fb);

private:
    bool                      _contentHasChanged();

    // tracks the delegated Motif that _contentHasChanged() last saw, so it can tell a
    // genuine backdoor motif swap (Map Editor push, type change) from a motif switch that
    // delegate() already just handled directly - see delegate() and _contentHasChanged()
    WeakMotifPtr              lastDelegatedMotif;

    QVector<TileMotifBtnPtr>  buttons;
    QScrollArea             * buttonBox;          // lhs top
    QGridLayout             * buttonSelectGrid;   // content of lhs top
    TileMotifButton         * viewerBtn;          // rhs top
    MotifEditorWidget       * motifEditorWidget;  // bottom
};
