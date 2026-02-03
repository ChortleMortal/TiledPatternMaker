#include <QHeaderView>
#include <QMessageBox>

#include "gui/model_editors/style_edit/style_color_fill_face.h"
#include "gui/model_editors/style_edit/style_editors.h"
#include "gui/panels/panel_misc.h"
#include "gui/widgets/dlg_colorSet.h"
#include "model/styles/filled.h"

StyleColorFillFace::StyleColorFillFace(FilledEditor * parent, FilledPtr style, QVBoxLayout * vbox)  : filled(style)
{
    this->parent        = parent;
    iPaletteSelection   = -1;

    QPushButton * addC    = new QPushButton("Add color");
    QPushButton * modifyC = new QPushButton("Modify color");
    QPushButton * deleteC = new QPushButton("Delete color");
    QPushButton * resetC  = new QPushButton("Reset colors");


    QWidget * widget    = new QWidget();
    QHBoxLayout * hbox  = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(addC);
    hbox->addWidget(modifyC);
    hbox->addWidget(deleteC);
    hbox->addWidget(resetC);
    hbox->addStretch();
    widget->setLayout(hbox);

    table = new QTableWidget();
    table->verticalHeader()->setVisible(false);
    table->setRowCount(filled->getWhiteColorSet()->size());
    table->setColumnCount(2);
    table->setMinimumHeight(301);
    QStringList qslH;
    qslH << "Selection" <<  "Color";
    table->setHorizontalHeaderLabels(qslH);

    connect(table,      &QTableWidget::cellClicked, this, &StyleColorFillFace::slot_cellClicked);
    connect(addC,       &QPushButton::clicked,      this, &StyleColorFillFace::slot_add);
    connect(modifyC,    &QPushButton::clicked,      this, &StyleColorFillFace::slot_modify);
    connect(deleteC,    &QPushButton::clicked,      this, &StyleColorFillFace::slot_delete);
    connect(resetC,     &QPushButton::clicked,      this, &StyleColorFillFace::slot_reset);
    connect(this,       &StyleColorFillFace::sig_updateView, Sys::viewController, &SystemViewController::slot_updateView);

    vbox->addWidget(widget);
    vbox->addWidget(table);
}

void StyleColorFillFace::display()
{
    ColorSet * colorPalette = filled->getWhiteColorSet();
    table->clearContents();
    table->setRowCount(colorPalette->size());
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

        connect(label, &ClickableLabel::clicked, this, [this,row] {iPaletteSelection = row; table->selectRow(row);} );
    }
    table->updateGeometry();
    parent->updateGeometry();
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

    table->selectRow(row);
    iPaletteSelection = row;
}

void StyleColorFillFace::slot_reset()
{
    filled->resetColorIndices();
    filled->resetStyleRepresentation();
    filled->createStyleRepresentation();
    emit sig_updateView();
}

void StyleColorFillFace::slot_add()
{
    AQColorDialog dlg(table);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    QColor newColor = dlg.selectedColor();
    if (newColor.isValid())
    {
        ColorSet * cset = filled->getWhiteColorSet();
        cset->addColor(newColor);
        display();
    }
}

void StyleColorFillFace::slot_modify()
{
    ColorSet * cset = filled->getWhiteColorSet();
    auto row = table->currentRow();
    if (row < 0 || row >= cset->size())
    {
        QMessageBox box(parent);
        box.setText("Please seect a palette color first");
        box.exec();
        return;
    }

    TPColor tpcolor = cset->getTPColor(row);

    AQColorDialog dlg(tpcolor.color,table);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    QColor color = dlg.selectedColor();
    if (color.isValid())
    {
        tpcolor.color = color;
        cset->setColor(row, tpcolor);

        display();
        filled->resetStyleRepresentation();
        filled->createStyleRepresentation();
        emit sig_updateView();
    }
}
void StyleColorFillFace::slot_delete()
{

    ColorSet * cset = filled->getWhiteColorSet();
    int row = table->currentRow();
    if (row < 0 || row >= cset->size())
    {
        QMessageBox box(parent);
        box.setText("Please seect a palette color first");
        box.exec();
        return;
    }

    QMessageBox box(parent);
    box.setText("Delete color:  Are you sure?");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    auto rv = box.exec();
    if (rv == QMessageBox::Cancel)
        return;

    cset->removeTPColor(row);
    filled->removeColorIndex(row);

    display();
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
