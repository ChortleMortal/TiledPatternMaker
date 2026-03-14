#include <QHeaderView>
#include <QCheckBox>

#include "gui/widgets/panel_misc.h"
#include "gui/model_editors/style_edit/fill_set_editor.h"
#include "model/styles/filled.h"

FillSetEditor::FillSetEditor(FilledEditor * parent, FilledPtr filled, New2Coloring *cm, QVBoxLayout * vbox)
    : FilledSubTypeEditor(parent,filled,cm)
{
    new2cm = cm;

    QPushButton * upBtn  = new QPushButton("Up");
    QPushButton * dwnBtn = new QPushButton("Down");
    QPushButton * rptBtn = new QPushButton("Repeat Color");
    QPushButton * cpyBtn = new QPushButton("Copy");
    QPushButton * pstBtn = new QPushButton("Paste");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(upBtn);
    hbox->addWidget(dwnBtn);
    hbox->addWidget(rptBtn);
    hbox->addWidget(cpyBtn);
    hbox->addWidget(pstBtn);
    vbox->addLayout(hbox);

    connect(upBtn,  &QPushButton::clicked, this, &FillSetEditor::up);
    connect(dwnBtn, &QPushButton::clicked, this, &FillSetEditor::down);
    connect(rptBtn, &QPushButton::clicked, this, &FillSetEditor::rptColor);
    connect(cpyBtn, &QPushButton::clicked, this, &FillSetEditor::copyColor);
    connect(pstBtn, &QPushButton::clicked, this, &FillSetEditor::pasteColor);

    table = new AQTableWidget();
    table->verticalHeader()->setVisible(false);
    table->setRowCount(cm->colorSet.size());
    table->setColumnCount(9);
    table->setColumnWidth(COL_ROW,40);
    table->setColumnWidth(COL_FACES,40);
    table->setColumnWidth(COL_SIDES,40);
    table->setColumnWidth(COL_AREA,70);
    table->setColumnWidth(COL_HIDE,80);
    table->setColumnWidth(COL_SEL,40);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setMinimumHeight(501);

    new TableRowSelector(table);     // install helper managed by the table

    QStringList qslH;
    qslH << "Row" <<  "Faces" << "Sides" << "Area" << "Hide" << "Select" << "Color" << "Color" << "Edit";
    table->setHorizontalHeaderLabels(qslH);

    connect(table,  &QTableWidget::cellClicked,         this,       &FillSetEditor::slot_cellClick);
    connect(table,  &QTableWidget::cellDoubleClicked,   this,       &FillSetEditor::slot_double_click);

    create();

    vbox->addWidget(table);
}

void FillSetEditor::create()
{
    table->clearContents();

    FaceGroup & faceGroups = new2cm->faceGroup;

    table->setRowCount(faceGroups.size());

    int row = 0;
    for (const FaceSetPtr & faceSet : std::as_const(faceGroups))
    {
        populateRow(faceSet,row);
        row++;
    }
}

void FillSetEditor::populateRow(const FaceSetPtr & faceSet, int row)
{
    QTableWidgetItem * item = new QTableWidgetItem(QString::number(row));
    table->setItem(row,COL_ROW,item);

    item = new QTableWidgetItem(QString::number(faceSet->size()));
    table->setItem(row,COL_FACES,item);

    item = new QTableWidgetItem(QString::number(faceSet->sides));
    table->setItem(row,COL_SIDES,item);

    item = new QTableWidgetItem(QString::number(faceSet->area));
    table->setItem(row,COL_AREA,item);

    item = new QTableWidgetItem((faceSet->selected) ? "X" : "");
    table->setItem(row,COL_SEL,item);

    // take color from whiteColorSet
    TPColor tpcolor  = new2cm->colorSet[row];
    QColor color     = tpcolor.color;
    QColor fullColor = color;
    fullColor.setAlpha(255);

    AQLineEdit * le = new AQLineEdit();
    table->setCellWidget(row,COL_COLOR_TEXT,le);
    le->setText(color.name());
    connect(le, &AQLineEdit::editingFinished, this, [this,row] { colorChanged(row); });

    QPushButton * btn = new QPushButton;
    btn->setText("");
    QVariant variant = fullColor;
    QString colcode  = variant.toString();
    btn->setStyleSheet("QPushButton { background-color :"+colcode+" ;}");
    table->setCellWidget(row,COL_COLOR_PATCH,btn);
    connect(btn,&QPushButton::clicked, this, &FillSetEditor::btnClicked);

    AQCheckBox * cb = new AQCheckBox("Hide");
    table->setCellWidget(row,COL_HIDE,cb);
    cb->setChecked(tpcolor.hidden);
    connect(cb, &AQCheckBox::clicked, this, [this, row] { colorVisibilityChanged(row); });

    btn = new QPushButton("Edit");
    table->setCellWidget(row,COL_EDIT_BTN,btn);
    connect(btn, &QPushButton::clicked, this, [this,row] { edit(row); });
}

void FillSetEditor::refresh()
{
    FaceGroup & faceGroup = new2cm->faceGroup;

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

void FillSetEditor::refreshRow(const FaceSetPtr & faceSet, int row)
{
    QTableWidgetItem * item;
    QWidget          * cellWidget;

    item = table->item(row,COL_ROW);
    item->setText(QString::number(row));

    item = table->item(row,COL_FACES);
    item->setText(QString::number(faceSet->size()));

    item = table->item(row,COL_SIDES);
    item->setText(QString::number(faceSet->sides));

    item = table->item(row,COL_AREA);
    item->setText(QString::number(faceSet->area));

    item = table->item(row,COL_SEL);
    item->setText((faceSet->selected) ? "X" : "");

    // take color from whiteColorSet
    TPColor tpcolor  = new2cm->colorSet[row];
    QColor color     = tpcolor.color;
    QColor fullColor = color;
    fullColor.setAlpha(255);

    cellWidget = table->cellWidget(row,COL_COLOR_TEXT);
    AQLineEdit * le = static_cast<AQLineEdit*>(cellWidget);
    le->setText(color.name());

    QVariant variant = fullColor;
    QString colcode  = variant.toString();

    cellWidget = table->cellWidget(row,COL_COLOR_PATCH);
    QPushButton * btn = static_cast<QPushButton*>(cellWidget);
    btn->setStyleSheet("QPushButton { background-color :"+colcode+" ;}");

    cellWidget = table->cellWidget(row,COL_HIDE);
    AQCheckBox * cb = static_cast<AQCheckBox*>(cellWidget);
    cb->setChecked(tpcolor.hidden);
}

void FillSetEditor::edit(int row)
{
    qDebug().noquote() << "edit row =" << row << "before" << new2cm->colorSet.colorsString();

    if (row < 0 || row >= new2cm->colorSet.size())
        return;

    TPColor tpcolor = new2cm->colorSet[row];
    QColor color    = tpcolor.color;

    AQColorDialog dlg(color, parent);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        tpcolor.color = color;
        new2cm->colorSet[row] = tpcolor;

        qDebug().noquote() << "after" << new2cm->colorSet.colorsString();
        emit parent->sig_updateView();
    }
}

void FillSetEditor::up()
{
    int currentRow = table->currentRow();
    if (currentRow < 1)
        return;

    TPColor a = new2cm->colorSet[currentRow];
    TPColor b = new2cm->colorSet[currentRow-1];

    new2cm->colorSet[currentRow-1] = a;
    new2cm->colorSet[currentRow]   = b;

    table->setCurrentCell(--currentRow,0);

    emit parent->sig_updateView();
}

void FillSetEditor::down()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= (new2cm->colorSet.size()-1))
        return;

    TPColor a = new2cm->colorSet[currentRow];
    TPColor b = new2cm->colorSet[currentRow+1];

    new2cm->colorSet[currentRow+1] = a;
    new2cm->colorSet[currentRow]   = b;

    table->setCurrentCell(++currentRow,0);

    emit parent->sig_updateView();
}

void FillSetEditor::rptColor()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= new2cm->colorSet.size())
        return;

    TPColor a = new2cm->colorSet[currentRow];
    for (int i = currentRow + 1; i < new2cm->colorSet.size(); i++)
    {
        new2cm->colorSet[i] = a;
    }

    emit parent->sig_updateView();
}

void FillSetEditor::copyColor()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= new2cm->colorSet.size())
        return;

    copyPasteColor = new2cm->colorSet[currentRow];
}

void FillSetEditor::pasteColor()
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= new2cm->colorSet.size())
        return;

    new2cm->colorSet[currentRow] = copyPasteColor;

    emit parent->sig_updateView();
}

void FillSetEditor::colorPick(QColor color)
{
    int currentRow = table->currentRow();
    if (currentRow < 0 || currentRow >= new2cm->colorSet.size())
        return;

    qDebug() << " color pick row" << currentRow  << "color" << color;
    new2cm->colorSet[currentRow] = color;

    emit parent->sig_updateView();
}

void FillSetEditor::notify()
{
    emit parent->sig_updateView();
}

void FillSetEditor::colorVisibilityChanged(int row)
{
    qDebug() << "colorVisibilityChanged row=" << row;

    TPColor tpcolor = new2cm->colorSet[row];

    QCheckBox * cb  = static_cast<QCheckBox*>(table->cellWidget(row,COL_HIDE));
    tpcolor.hidden  = cb->isChecked();
    new2cm->colorSet[row] = tpcolor;
    qDebug() << "set hide state="  << tpcolor.hidden;

    emit parent->sig_updateView();

    qDebug() << "colorVisibilityChanged: done";
}

void FillSetEditor::colorChanged(int row)
{
    qDebug() << "colorChanged row=" << row;

    QLineEdit *  le = static_cast<QLineEdit*>(table->cellWidget(row,COL_COLOR_TEXT));

    QString  sColor = le->text();
    if (sColor.size() != 7)
    {
        return;     // wait for complete color
    }

    QColor color(sColor);
    if (color.isValid())
    {
        new2cm->colorSet[row] = color;

        emit parent->sig_updateView();
    }
}

void FillSetEditor::slot_cellClick(int row, int col)
{
    if (col == COL_SEL)
    {
        if (!new2cm->faceGroup.isSelected(row))
        {
            new2cm->faceGroup.select(row);
        }
        else
        {
            new2cm->faceGroup.deselect(row);
        }
    }
    else
    {
        new2cm->faceGroup.deselect();
    }

    emit parent->sig_updateView();
}

void FillSetEditor::slot_double_click(int row, int col)
{
    Q_UNUSED(col);

    edit(row);
}

void FillSetEditor::mousePressed(QPointF mpt, Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    const FaceGroup & faceGroup = new2cm->faceGroup;

    int row = 0;
    for (const FaceSetPtr & faceSet : std::as_const(faceGroup))
    {
        for (const FacePtr & face : std::as_const(*faceSet))
        {
            if (face->getPolygon().containsPoint(mpt,Qt::OddEvenFill))
            {
                //qDebug() << "select row" << row;
                table->setCurrentCell(row,0);
                return;
            }
        }
        row++;
    }
}

void FillSetEditor::btnClicked()
{
    auto *label = qobject_cast<QWidget*>(sender());
    QWidget *cellWidget = findCellWidget(label, table);

    if (!cellWidget)
    {
        qDebug() << "Could not find cell widget";
        return;
    }

    QPoint center = cellWidget->geometry().center();  // already in viewport coords
    int row       = table->indexAt(center).row();

    qDebug() << "Row is:" << row;   // use this instead of currentRow()

    TPColor tpc = new2cm->colorSet[row];

    // FIXME this came from pickColor and maybe should be there?
    AQColorDialog dlg(tpc.color,parent);
    dlg.setCurrentColor(tpc.color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    QColor acolor = dlg.selectedColor();
    if (acolor.isValid())
    {
        tpc.color = acolor;
        new2cm->colorSet[row] = tpc;
        emit parent->sig_updateView();
    }
}

QWidget* FillSetEditor::findCellWidget(QWidget *w, QTableWidget *table)
{
    QWidget *p = w;
    while (p && p->parentWidget() != table->viewport())
        p = p->parentWidget();
    return p;
}
