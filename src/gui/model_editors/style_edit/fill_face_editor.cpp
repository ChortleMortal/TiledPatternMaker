#include <QHeaderView>
#include <QMessageBox>

#include "gui/model_editors/style_edit/fill_face_editor.h"
#include "gui/model_editors/style_edit/style_editor.h"
#include "gui/widgets/panel_misc.h"
#include "gui/widgets/dlg_colorSet.h"

FillFaceEditor::FillFaceEditor(FilledEditor * parent, FilledPtr style, DirectColoring *cm, QVBoxLayout * vbox)
    : FilledSubTypeEditor(parent,style,cm)
{
    qInfo() << "StyleColorFillFace - constructor";

    dcm                 = cm;
    iPaletteSelection   = -1;

    QPushButton * addC    = new QPushButton("Add Color");
    QPushButton * modifyC = new QPushButton("Edit Color");
    QPushButton * deleteC = new QPushButton("Delete Color");
    QPushButton * resetC  = new QPushButton("Reset Faces");

    QWidget * widget    = new QWidget();
    QHBoxLayout * hbox  = new QHBoxLayout();
    hbox->addStretch(2);
    hbox->addWidget(addC);
    hbox->addWidget(modifyC);
    hbox->addWidget(deleteC);
    hbox->addStretch(3);
    hbox->addWidget(resetC);
    hbox->addStretch(2);
    widget->setLayout(hbox);

    table = new AQTableWidget();
    table->verticalHeader()->setVisible(false);
    table->setRowCount(cm->palette.size());
    table->setColumnCount(2);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    table->setMinimumHeight(301);

    QStringList qslH;
    qslH << "Selection" <<  "Color";
    table->setHorizontalHeaderLabels(qslH);

    connect(table,      &QTableWidget::cellClicked, this, &FillFaceEditor::slot_cellClicked);
    connect(addC,       &QPushButton::clicked,      this, &FillFaceEditor::slot_add);
    connect(modifyC,    &QPushButton::clicked,      this, &FillFaceEditor::slot_modify);
    connect(deleteC,    &QPushButton::clicked,      this, &FillFaceEditor::slot_delete);
    connect(resetC,     &QPushButton::clicked,      this, &FillFaceEditor::slot_resetColorMap);
    connect(this,       &FillFaceEditor::sig_updateView, Sys::viewController, &SystemViewController::slot_updateView);

    create();

    vbox->addWidget(widget);
    vbox->addWidget(table);
}

FillFaceEditor::~FillFaceEditor()
{
    qInfo() << "StyleColorFillFace - DESTRUCTOR";
}

void FillFaceEditor::create()
{
    ColorSet * colorPalette = &dcm->palette;

    table->clearContents();
    table->setRowCount(colorPalette->size());

    for (int row =0 ; row < colorPalette->size(); row++)
    {
        QTableWidgetItem * item = new QTableWidgetItem("Not selected");
        table->setItem(row,0,item);
        if (iPaletteSelection == row)
        {
            item->setText("Selected");
        }

        TPColor tpcolor  = colorPalette->getTPColor(row);
        QVariant variant = tpcolor.color;
        QString colcode  = variant.toString();

        ClickableLabel * label   = new ClickableLabel;
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        table->setCellWidget(row,1,label);

        connect(label, &ClickableLabel::clicked, this, [this,row] {iPaletteSelection = row; table->selectRow(row);} );
        connect(label, &ClickableLabel::clicked, this, [this,row] { modify(row);} );
    }

    table->updateGeometry();
    parent->updateGeometry();

    refresh();
}

void FillFaceEditor::refresh()
{
    ColorSet * colorPalette = &dcm->palette;
    if (table->rowCount() != colorPalette->size())
        return;

    for (int row =0 ; row < colorPalette->size(); row++)
    {
        QTableWidgetItem * item = table->item(row,0);
        if (item)
        {
            if (iPaletteSelection == row)
                item->setText("Selected");
            else
                item->setText("Not selected");
        }

        auto widget = table->cellWidget(row,1);
        if (widget)
        {
            ClickableLabel * label = static_cast<ClickableLabel *>(widget);

            TPColor tpcolor  = colorPalette->getTPColor(row);
            QVariant variant = tpcolor.color;
            QString colcode  = variant.toString();

            label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        }
    }
}

void FillFaceEditor::colorPick(QColor color)
{
    Q_UNUSED(color);
}

void FillFaceEditor::notify()
{
    emit parent->sig_updateView();
}

void FillFaceEditor::slot_cellClicked(int row,int col)
{
    Q_UNUSED(col);

    table->selectRow(row);
    iPaletteSelection = row;
}

void FillFaceEditor::slot_resetColorMap()
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Question);
    box.setText("Are You Sure?");
    box.setInformativeText("This erases all the associations of colors to faces.");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    auto rv = box.exec();
    if (rv == QMessageBox::Cancel)
        return;

    auto filled = wfilled.lock();
    if (filled)
    {
        filled->resetStyleRepresentation();     // erases the color map
        filled->createStyleRepresentation();
        emit sig_updateView();
    }
}

void FillFaceEditor::slot_add()
{
    AQColorDialog dlg(table);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    QColor newColor = dlg.selectedColor();
    if (newColor.isValid())
    {
        ColorSet * cset = &dcm->palette;
        cset->addColor(newColor);
        create();
    }
}

void FillFaceEditor::slot_modify()
{
    ColorSet * cset = &dcm->palette;
    auto row = table->currentRow();
    if (row < 0 || row >= cset->size())
    {
        QMessageBox box(parent);
        box.setText("Please seect a palette color first");
        box.exec();
        return;
    }

    modify(row);
}

void FillFaceEditor::modify(int row)
{
    ColorSet * cset = &dcm->palette;
    TPColor tpcolor = cset->getTPColor(row);

    AQColorDialog dlg(tpcolor.color,table);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    QColor color = dlg.selectedColor();
    if (color.isValid())
    {
        tpcolor.color = color;
        cset->setColor(row, tpcolor);

        refresh();
        emit sig_updateView();
    }
}

void FillFaceEditor::slot_delete()
{
    ColorSet * cset = &dcm->palette;
    int row = table->currentRow();

    if (row < 0 || row >= cset->size())
    {
        QMessageBox box(parent);
        box.setText("Please seect a palette color first");
        box.exec();
        return;
    }

    QMessageBox box(parent);
    box.setIcon(QMessageBox::Question);
    box.setText("Delete color:  Are you sure?");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    auto rv = box.exec();
    if (rv == QMessageBox::Cancel)
        return;

    cset->removeTPColor(row);

    create();
    emit sig_updateView();
}

void FillFaceEditor::mousePressed(QPointF mpt,Qt::MouseButton btn)
{
    auto filled = wfilled.lock();
    if (!filled) return;

    DCELPtr dcel  = filled->getPrototype()->getDCEL();
    if (!dcel) return;

    if (iPaletteSelection == -1)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("First, use the color palette to select a color");
        box.exec();
        return;
    }

    const FaceSet & faces = dcel->getFaceSet();
    for (const FacePtr & face : faces)
    {
        if (face->getPolygon().containsPoint(mpt,Qt::OddEvenFill))
        {
            if (face->outer)
            {
                continue;
            }
            if (btn == Qt::LeftButton)
            {
                face->iPalette = iPaletteSelection;     // sets color in face
                dcm->faceMap.insert(PointKey{face->center()},face);
            }
            else
            {
                face->iPalette = -1;    // erases
                dcm->faceMap.remove(PointKey{face->center()});
            }
            return;
        }
    }
}