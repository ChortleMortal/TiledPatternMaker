#include <QHeaderView>
#include "makers/mosaic_maker/style_color_fill_face.h"
#include "makers/mosaic_maker/style_editors.h"
#include "style/filled.h"

StyleColorFillFace::StyleColorFillFace(FilledEditor * parent, FilledPtr style, QVBoxLayout * vbox)  : filled(style)
{
    this->parent        = parent;
    iPaletteSelection   = -1;

    table = new QTableWidget();
    table->verticalHeader()->setVisible(false);
    table->setRowCount(filled->getWhiteColorSet()->size());
    table->setColumnCount(3);
    table->setColumnWidth(COL_STATUS,131);
    table->setColumnWidth(COL_COLOR,410);
    table->setColumnWidth(COL_EDIT,120);

    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setMinimumHeight(501);

    QStringList qslH;
    qslH << "Selection" <<  "Color" << "Edit";
    table->setHorizontalHeaderLabels(qslH);

    connect(table,  &QTableWidget::cellClicked, this, &StyleColorFillFace::slot_cellClicked);

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

        connect(btnW,  &QPushButton::clicked,    parent, &FilledEditor::slot_editW);
        connect(label, &ClickableLabel::clicked, this, [this,row] {iPaletteSelection = row;});
    }
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

void StyleColorFillFace::select(QPointF mpt,Qt::MouseButton btn)
{
    ColorMaker & cm = filled->getColorMaker();
    DCELPtr dcel    = cm.getDCEL();
    if (!dcel) return;

    FaceSet & faces = dcel->getFaceSet();

    int faceIndex = 0;
    for (const FacePtr & face : std::as_const(faces))
    {
        faceIndex++;
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
            cm.setColorIndex(faceIndex,face->iPalette);              // duplicates this info in indices
            return;
        }
    }
}
