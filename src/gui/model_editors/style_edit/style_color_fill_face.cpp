#include <QHeaderView>
#include "gui/model_editors/style_edit/style_color_fill_face.h"
#include "gui/model_editors/style_edit/style_editors.h"
#include "gui/panels/panel_misc.h"
#include "gui/widgets/dlg_colorSet.h"
#include "model/styles/filled.h"

StyleColorFillFace::StyleColorFillFace(FilledEditor * parent, FilledPtr style, QVBoxLayout * vbox)  : filled(style)
{
    this->parent        = parent;
    iPaletteSelection   = -1;

    QPushButton * reset = new QPushButton("Reset color assignments");
    reset->setMinimumHeight(31);
    reset->setMinimumWidth(231);
    QWidget * widget    = new QWidget();
    QHBoxLayout * hbox  = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(reset);
    hbox->addStretch();
    widget->setLayout(hbox);

    table = new QTableWidget();
    table->verticalHeader()->setVisible(false);
    table->setRowCount(filled->getWhiteColorSet()->size());
    table->setColumnCount(3);
    table->setColumnWidth(COL_STATUS,131);
    table->setColumnWidth(COL_COLOR,210);
    table->setColumnWidth(COL_EDIT,120);

    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setMinimumHeight(501);

    QStringList qslH;
    qslH << "Selection" <<  "Color" << "Edit";
    table->setHorizontalHeaderLabels(qslH);

    connect(table,  &QTableWidget::cellClicked,          this, &StyleColorFillFace::slot_cellClicked);
    connect(reset,  &QPushButton::clicked,               this, &StyleColorFillFace::slot_reset);
    connect(this,   &StyleColorFillFace::sig_updateView, Sys::view, &View::slot_update);

    vbox->addWidget(widget);
    vbox->addWidget(table);
}

void StyleColorFillFace::display()
{
    ColorSet * colorPalette = filled->getWhiteColorSet();

    for (int row =0 ; row < colorPalette->size(); row++)
    {
        QTableWidgetItem * item = new QTableWidgetItem("Not selected");
        table->setItem(row,0,item);

        TPColor tpcolor = colorPalette->getTPColor(row);
        ClickableLabel * label   = new ClickableLabel;
        QVariant variant = tpcolor.color;
        QString colcode  = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        table->setCellWidget(row,1,label);

        QPushButton * btnW = new QPushButton("Edit");
        table->setCellWidget(row,2,btnW);

        connect(btnW,  &QPushButton::clicked,    this, &StyleColorFillFace::editPalette);
        connect(label, &ClickableLabel::clicked, this, [this,row] {iPaletteSelection = row;});
    }
    table->adjustSize();
}

void StyleColorFillFace::onRefresh()
{
    for (int row = 0; row < table->rowCount(); row++)
    {
        QTableWidgetItem * item = table->item(row,0);
        if (item)
        {
            if (iPaletteSelection == row)
            {
                item->setText("Selected");
            }
            else
            {
                item->setText("Not selected");
            }
        }
    }
}

void StyleColorFillFace::slot_cellClicked(int row,int col)
{
    Q_UNUSED(col);

    iPaletteSelection = row;
}

void StyleColorFillFace::slot_reset()
{
    filled->resetColorIndices();
    filled->resetStyleRepresentation();
    filled->createStyleRepresentation();
    emit sig_updateView();
}

void StyleColorFillFace::select(QPointF mpt,Qt::MouseButton btn)
{
    DCELPtr dcel    = filled->getPrototype()->getDCEL();
    if (!dcel) return;

    FaceSet & faces = dcel->getFaceSet();

    int faceIndex = 0;
    for (const FacePtr & face : std::as_const(faces))
    {
        if (face->getPolygon().containsPoint(mpt,Qt::OddEvenFill))
        {
            if (btn == Qt::LeftButton)
            {
                face->iPalette = iPaletteSelection;     // sets color in face
            }
            else
            {
                face->iPalette = -1;    // erases
            }
            filled->setColorIndex(faceIndex,face->iPalette);              // duplicates this info in indices
            return;
        }
        faceIndex++;
    }
}

void StyleColorFillFace::editPalette()
{
    DlgColorSet dlg(filled->getWhiteColorSet());

    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, parent, &FilledEditor::slot_colorsChanged, Qt::QueuedConnection);

    dlg.exec();
}
