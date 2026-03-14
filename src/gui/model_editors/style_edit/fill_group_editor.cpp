#include <QHeaderView>
#include <QCheckBox>
#include <QPushButton>
#include <QTableWidget>

#include "gui/model_editors/style_edit/fill_group_editor.h"
#include "gui/widgets/colorset_widget.h"
#include "gui/widgets/dlg_colorSet.h"
#include "gui/widgets/panel_misc.h"
#include "model/styles/colorset.h"

FillGroupEditor::FillGroupEditor(FilledEditor *parent, FilledPtr style, New3Coloring * cm, QVBoxLayout *vbox)
    : FilledSubTypeEditor(parent,style,cm)
{
    new3cm = cm;

    QPushButton * upBtn  = new QPushButton("Up");
    QPushButton * dwnBtn = new QPushButton("Down");
    QPushButton * rptBtn = new QPushButton("Repeat Set");
    QPushButton * cpyBtn = new QPushButton("Copy Set");
    QPushButton * pstBtn = new QPushButton("Paste Set");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(upBtn);
    hbox->addWidget(dwnBtn);
    hbox->addWidget(rptBtn);
    hbox->addWidget(cpyBtn);
    hbox->addWidget(pstBtn);
    vbox->addLayout(hbox);

    connect(upBtn,  &QPushButton::clicked, this, &FillGroupEditor::up);
    connect(dwnBtn, &QPushButton::clicked, this, &FillGroupEditor::down);
    connect(rptBtn, &QPushButton::clicked, this, &FillGroupEditor::rptSet);
    connect(cpyBtn, &QPushButton::clicked, this, &FillGroupEditor::copySet);
    connect(pstBtn, &QPushButton::clicked, this, &FillGroupEditor::pasteSet);

    table = new AQTableWidget();
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setColumnCount(8);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    table->setColumnWidth(COL_INDEX,35);
    table->setColumnWidth(COL_COUNT,40);
    table->setColumnWidth(COL_SIDES,40);
    table->setColumnWidth(COL_AREA,70);
    table->setColumnWidth(COL_HIDE,80);
    table->setColumnWidth(COL_SEL,40);
    //fillGroupTable->setColumnWidth(COL_BTN,70);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setMinimumHeight(501);

    QStringList qslH;
    qslH << "" << "Count" << "Sides" << "Area" << "Hide" << "Sel" << "Edit" << "Colors";
    table->setHorizontalHeaderLabels(qslH);

    create();

    connect(table,  &QTableWidget::cellClicked,       this, &FillGroupEditor::slot_click);
    connect(table,  &QTableWidget::cellDoubleClicked, this, &FillGroupEditor::slot_double_click);

    vbox->addWidget(table);
}

void FillGroupEditor::create()
{
    qDebug() << "FillGroupEditor::create";

    Q_ASSERT(table);

    table->clearContents();

    table->setRowCount(new3cm->faceGroup.size());

    int row = 0;
    for (const auto & faceSet : std::as_const(new3cm->faceGroup))
    {
        populateRow(faceSet, row);
        row++;
    }
}

void FillGroupEditor::populateRow(const FaceSetPtr & faceSet, int row)
{
    QTableWidgetItem * item = new QTableWidgetItem(QString::number(row));
    table->setItem(row,COL_INDEX,item);

    item = new QTableWidgetItem(QString::number(faceSet->size()));
    table->setItem(row,COL_COUNT,item);

    item = new QTableWidgetItem(QString::number(faceSet->sides));
    table->setItem(row,COL_SIDES,item);

    item = new QTableWidgetItem(QString::number(faceSet->area));
    table->setItem(row,COL_AREA,item);

    AQCheckBox * cb = new AQCheckBox("Hide");
    table->setCellWidget(row,COL_HIDE,cb);
    cb->setChecked(new3cm->colorGroup.isHidden(row));
    connect(cb, &QCheckBox::clicked, this, [this,row] { colorSetVisibilityChanged(row); });

    QPushButton * btn = new QPushButton("Edit");
    table->setCellWidget(row,COL_EDIT_BTN,btn);
    connect(btn, &QPushButton::clicked, this, [this,row] { edit(row); });

    ColorSet *  cset = new3cm->colorGroup.getColorSet(row);
    ColorSetWidget * widget = new ColorSetWidget(parent,cset);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    table->setCellWidget(row,COL_COLORS,widget);

    item = new QTableWidgetItem((faceSet->selected) ? "X": "");
    table->setItem(row,COL_SEL,item);
}

void FillGroupEditor::refresh()
{
    FaceGroup & faceGroup = new3cm->faceGroup;

    // update table
    int currentRows = table->rowCount();
    int neededRows  = faceGroup.size();

    if (currentRows != neededRows)
    {
        table->setRowCount(neededRows);

        // Update existing rows
        int minRows = std::min(currentRows, neededRows);
        for (int row = 0; row < minRows; ++row)
        {
            populateRow(faceGroup[row],row);
        }

        // Populate new rows (if any)
        for (int row = minRows; row < neededRows; ++row)
        {
            populateRow(faceGroup[row],row);
        }
    }

    for (int row = 0; row < neededRows; row++)
    {
        refreshRow(faceGroup[row],row);
    }

    //table->adjustTableSize();
}

void FillGroupEditor::refreshRow(const FaceSetPtr & faceSet, int row)
{
    QTableWidgetItem * item;
    QWidget          * cellWidget;

    item = table->item(row,COL_COUNT);
    item->setText(QString::number(row));

    item = table->item(row,COL_COUNT);
    item->setText(QString::number(faceSet->size()));

    item = table->item(row,COL_SIDES);
    item->setText(QString::number(faceSet->sides));

    item = table->item(row,COL_AREA);
    item->setText(QString::number(faceSet->area));

    cellWidget = table->cellWidget(row,COL_HIDE);
    AQCheckBox * cb = static_cast<AQCheckBox*>(cellWidget);
    cb->setChecked(new3cm->colorGroup.isHidden(row));

    cellWidget = table->cellWidget(row,COL_COLORS);
    ColorSetWidget * csw = static_cast<ColorSetWidget*>(cellWidget);
    csw->updateFromColorSet();

    item = table->item(row,COL_SEL);
    item->setText((faceSet->selected) ? "X" : "");
}

void FillGroupEditor::edit(int row)
{
    if (row < 0 || row >= new3cm->colorGroup.size())
    {
        return;
    }

    ColorSet * colorSet = new3cm->colorGroup.getColorSet(row);

    DlgColorSet dlg(colorSet,table);
    dlg.setWindowTitle(QString("Row = %1").arg(QString::number(row)));
    dlg.exec();

    emit parent->sig_updateView();
}


void FillGroupEditor::up()
{
    int currentRow = table->currentRow();
    if (currentRow < 1)
        return;

    ColorSet a = *new3cm->colorGroup.getColorSet(currentRow);
    ColorSet b = *new3cm->colorGroup.getColorSet(currentRow-1);

    new3cm->colorGroup.setColorSet(currentRow-1, a);
    new3cm->colorGroup.setColorSet(currentRow  , b);

    emit parent->sig_updateView();
}

void FillGroupEditor::down()
{
    int currentRow = table->currentRow();
    if (currentRow >= (new3cm->colorGroup.size()-1))
        return;

    ColorSet a = *new3cm->colorGroup.getColorSet(currentRow);
    ColorSet b = *new3cm->colorGroup.getColorSet(currentRow+1);

    new3cm->colorGroup.setColorSet(currentRow+1, a);
    new3cm->colorGroup.setColorSet(currentRow  , b);

    emit parent->sig_updateView();
}

void FillGroupEditor::rptSet()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= new3cm->colorGroup.size())
        return;

    copyPasteSet = *new3cm->colorGroup.getColorSet(currentRow);
    new3cm->colorGroup.resize(table->rowCount());
    for (int i = currentRow + 1; i < table->rowCount(); i++)
    {
        new3cm->colorGroup.setColorSet(i,copyPasteSet);
    }

    emit parent->sig_updateView();
}

void FillGroupEditor::copySet()
{
    qDebug() << "FillGroupEditor::copySet()";
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= new3cm->colorGroup.size())
        return;

    copyPasteSet = *new3cm->colorGroup.getColorSet(currentRow);
}

void FillGroupEditor::pasteSet()
{
    qDebug() << "FillGroupEditor::pasteSet()";
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= new3cm->colorGroup.size())
        return;

    new3cm->colorGroup.setColorSet(currentRow,copyPasteSet);

    emit parent->sig_updateView();
}

void FillGroupEditor::colorPick(QColor color)
{
    qDebug() << "FillGroupEditor::setColor" << color;

    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= new3cm->colorGroup.size())
        return;

    // this adds a color to the face set
    ColorSet * cset = new3cm->colorGroup.getColorSet(currentRow);
    cset->addColor(color,false);
    new3cm->colorGroup.hide(currentRow, false);

    emit parent->sig_updateView();
}

void FillGroupEditor::notify()
{
    // Changing the colors in the group can require the Filled style to rebuild its
    // representation (faceGroup/colorGroup bindings) so the view shows the new
    // colors immediately.
    if (auto filled = parent->getFilled())
    {
        filled->resetStyleRepresentation();
    }
    emit parent->sig_reconstructView();
}


void FillGroupEditor::colorSetVisibilityChanged(int row)
{
    qDebug() << "colorVisibilityChanged row=" << row;

    QCheckBox * cb  = dynamic_cast<QCheckBox*>(table->cellWidget(row,COL_HIDE));
    bool hide       = cb->isChecked();
    qDebug() << "group hide state="  << hide;

    new3cm->colorGroup.hide(row, hide);

    emit parent->sig_updateView();

    qDebug() << "colorVisibilityChanged: done";
}

void FillGroupEditor::slot_click(int row, int col)
{
    if (col == COL_SEL)
    {
        if (!new3cm->faceGroup.isSelected(row))
        {
            new3cm->faceGroup.select(row);
        }
        else
        {
            new3cm->faceGroup.deselect(row);
        }
    }
    else
    {
        new3cm->faceGroup.deselect();
    }

    emit parent->sig_updateView();

    refresh();
}

void FillGroupEditor::slot_double_click(int row, int col)
{
    Q_UNUSED(col);

    edit(row);
}

void FillGroupEditor::mousePressed(QPointF mpt, Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    const FaceGroup & faceGroup = new3cm->faceGroup;
    int row = 0;
    for (const FaceSetPtr & faceSet : std::as_const(faceGroup))
    {
        for (const FacePtr & face : std::as_const(*faceSet))
        {
            if (face->getPolygon().containsPoint(mpt,Qt::OddEvenFill))
            {
                table->setCurrentCell(row,0);
                return;
            }
        }
        row++;
    }
}

