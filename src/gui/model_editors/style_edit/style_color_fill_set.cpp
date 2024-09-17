#include <QHeaderView>
#include <QCheckBox>

#include "gui/model_editors/style_edit/style_color_fill_set.h"
#include "sys/sys.h"
#include "gui/top/view.h"
#include "model/styles/filled.h"

StyleColorFillSet::StyleColorFillSet(FilledPtr style, QVBoxLayout * vbox)  : filled(style)
{
    QGridLayout * grid = new QGridLayout;

    QPushButton * modBtn = new QPushButton("Modify");
    QPushButton * upBtn  = new QPushButton("Up");
    QPushButton * dwnBtn = new QPushButton("Down");
    QPushButton * rptBtn = new QPushButton("Repeat Color");
    QPushButton * cpyBtn = new QPushButton("Copy");
    QPushButton * pstBtn = new QPushButton("Paste");

    grid->addWidget(modBtn,0,1);
    grid->addWidget(cpyBtn,0,2);
    grid->addWidget(upBtn, 0,0);
    grid->addWidget(dwnBtn,1,0);
    grid->addWidget(rptBtn,1,1);
    grid->addWidget(pstBtn,1,2);
    vbox->addLayout(grid);

    connect(modBtn, &QPushButton::clicked, this, &StyleColorFillSet::modify);
    connect(upBtn,  &QPushButton::clicked, this, &StyleColorFillSet::up);
    connect(dwnBtn, &QPushButton::clicked, this, &StyleColorFillSet::down);
    connect(rptBtn, &QPushButton::clicked, this, &StyleColorFillSet::rptColor);
    connect(cpyBtn, &QPushButton::clicked, this, &StyleColorFillSet::copyColor);
    connect(pstBtn, &QPushButton::clicked, this, &StyleColorFillSet::pasteColor);

    fillSetTable = new QTableWidget();
    fillSetTable->verticalHeader()->setVisible(false);
    fillSetTable->setRowCount(filled->getWhiteColorSet()->size());
    fillSetTable->setColumnCount(8);
    fillSetTable->setColumnWidth(COL_ROW,40);
    fillSetTable->setColumnWidth(COL_FACES,40);
    fillSetTable->setColumnWidth(COL_SIDES,40);
    fillSetTable->setColumnWidth(COL_AREA,70);
    fillSetTable->setColumnWidth(COL_HIDE,80);
    fillSetTable->setColumnWidth(COL_SEL,40);
    fillSetTable->setSelectionMode(QAbstractItemView::SingleSelection);
    fillSetTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    fillSetTable->setMinimumHeight(501);

    QStringList qslH;
    qslH << "Row" <<  "Faces" << "Sides" << "Area" << "Hide" << "Select" << "Color" << "Color";
    fillSetTable->setHorizontalHeaderLabels(qslH);

    connect(fillSetTable,  &QTableWidget::cellClicked,        this,       &StyleColorFillSet::slot_click);
    connect(fillSetTable,  &QTableWidget::cellDoubleClicked,  this,       &StyleColorFillSet::slot_double_click);
    connect(this,          &StyleColorFillSet::sig_updateView, Sys::view, &View::slot_update);
    vbox->addWidget(fillSetTable);
}

void StyleColorFillSet::display()
{
    QModelIndex selected = fillSetTable->currentIndex();

    ColorMaker * cm = filled->getColorMaker();

    fillSetTable->clearContents();
    filled->getWhiteColorSet()->resetIndex();

    FaceGroups & faceGroups = cm->getFaceGroups();

    fillSetTable->setRowCount(faceGroups.size());

    int row = 0;
    for (const FaceSetPtr & faceSet : std::as_const(faceGroups))
    {
        QTableWidgetItem * item = new QTableWidgetItem(QString::number(row));
        fillSetTable->setItem(row,COL_ROW,item);

        item = new QTableWidgetItem(QString::number(faceSet->size()));
        fillSetTable->setItem(row,COL_FACES,item);

        item = new QTableWidgetItem(QString::number(faceSet->sides));
        fillSetTable->setItem(row,COL_SIDES,item);

        item = new QTableWidgetItem(QString::number(faceSet->area));
        fillSetTable->setItem(row,COL_AREA,item);

        // take color from whiteColorSet
        TPColor tpcolor  = filled->getWhiteColorSet()->getTPColor(row);
        QColor color     = tpcolor.color;
        QColor fullColor = color;
        fullColor.setAlpha(255);

        QLineEdit * le = new QLineEdit();
        fillSetTable->setCellWidget(row,COL_COLOR_TEXT,le);
        le->setText(color.name());
        connect(le, &QLineEdit::textEdited, this, [this,row] { colorChanged(row); });

        QLabel * label   = new QLabel;
        QVariant variant = fullColor;
        QString colcode  = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        fillSetTable->setCellWidget(row,COL_COLOR_PATCH,label);

        QCheckBox * cb = new AQCheckBox("Hide");
        fillSetTable->setCellWidget(row,COL_HIDE,cb);
        cb->setChecked(tpcolor.hidden);
        connect(cb, &QCheckBox::clicked, this, [this, row] { colorVisibilityChanged(row); });

        QString astring;
        if (faceSet->selected) astring = "X";
        item = new QTableWidgetItem(astring);
        fillSetTable->setItem(row,COL_SEL,item);

        row++;
    }
    //table->adjustTableSize();

    fillSetTable->setCurrentIndex(selected);
}

void StyleColorFillSet::modify()
{
    qDebug().noquote() << "before" << filled->getWhiteColorSet()->colorsString();

    int currentRow = fillSetTable->currentRow();
    if (currentRow < 0 || currentRow >= filled->getWhiteColorSet()->size())
        return;

    TPColor tpcolor = filled->getWhiteColorSet()->getTPColor(currentRow);
    QColor color    = tpcolor.color;

    AQColorDialog dlg(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        filled->getWhiteColorSet()->setColor(currentRow, color);
        qDebug().noquote() << "after" << filled->getWhiteColorSet()->colorsString();

        emit sig_colorsChanged();
    }
}

void StyleColorFillSet::up()
{
    int currentRow = fillSetTable->currentRow();
    if (currentRow < 1)
        return;

    TPColor a = filled->getWhiteColorSet()->getTPColor(currentRow);
    TPColor b = filled->getWhiteColorSet()->getTPColor(currentRow-1);

    filled->getWhiteColorSet()->setColor(currentRow-1, a);
    filled->getWhiteColorSet()->setColor(currentRow  , b);

    currentRow--;

    emit sig_colorsChanged();
}

void StyleColorFillSet::down()
{
    int currentRow = fillSetTable->currentRow();
    if (currentRow < 0 || currentRow >= (filled->getWhiteColorSet()->size()-1))
        return;

    TPColor a = filled->getWhiteColorSet()->getTPColor(currentRow);
    TPColor b = filled->getWhiteColorSet()->getTPColor(currentRow+1);

    filled->getWhiteColorSet()->setColor(currentRow+1, a);
    filled->getWhiteColorSet()->setColor(currentRow,   b);

    currentRow++;

    emit sig_colorsChanged();
}

void StyleColorFillSet::rptColor()
{
    int currentRow = fillSetTable->currentRow();
    if (currentRow < 0 || currentRow >= filled->getWhiteColorSet()->size())
        return;

    TPColor a = filled->getWhiteColorSet()->getTPColor(currentRow);
    filled->getWhiteColorSet()->resize(fillSetTable->rowCount());
    for (int i = currentRow + 1; i < fillSetTable->rowCount(); i++)
    {
        filled->getWhiteColorSet()->setColor(i,a);
    }

    emit sig_colorsChanged();
}

void StyleColorFillSet::copyColor()
{
    int currentRow = fillSetTable->currentRow();
    if (currentRow < 0 || currentRow >= filled->getWhiteColorSet()->size())
        return;

    copyPasteColor = filled->getWhiteColorSet()->getTPColor(currentRow);
}

void StyleColorFillSet::pasteColor()
{
    int currentRow = fillSetTable->currentRow();
    if (currentRow < 0 || currentRow >= filled->getWhiteColorSet()->size())
        return;

    filled->getWhiteColorSet()->setColor(currentRow,copyPasteColor);

    emit sig_colorsChanged();
}

void StyleColorFillSet::setColor(QColor color)
{
    int currentRow = fillSetTable->currentRow();
    if (currentRow < 0 || currentRow >= filled->getWhiteColorSet()->size())
        return;

    qDebug() << "row" << currentRow  << "color" << color;
    filled->getWhiteColorSet()->setColor(currentRow,color);

    emit sig_colorsChanged();
}

void StyleColorFillSet::colorVisibilityChanged(int row)
{
    qDebug() << "colorVisibilityChanged row=" << row;

    TPColor tpcolor = filled->getWhiteColorSet()->getTPColor(row);
    QCheckBox * cb  = dynamic_cast<QCheckBox*>(fillSetTable->cellWidget(row,COL_HIDE));
    bool hide       = cb->isChecked();
    tpcolor.hidden  = hide;
    filled->getWhiteColorSet()->setColor(row, tpcolor);
    qDebug() << "set hide state="  << hide;

    emit sig_updateView();

    qDebug() << "colorVisibilityChanged: done";
}

void StyleColorFillSet::colorChanged(int row)
{
    qDebug() << "colorChanged row=" << row;

    QLineEdit *  le = dynamic_cast<QLineEdit*>(fillSetTable->cellWidget(row,COL_COLOR_TEXT));

    QString  sColor = le->text();
    if (sColor.size() != 7)
    {
        return;     // wait for complete color
    }

    QColor color(sColor);
    if (color.isValid())
    {
        filled->getWhiteColorSet()->setColor(row, color);
        emit sig_colorsChanged();
    }
}

void StyleColorFillSet::slot_click(int row, int col)
{
    ColorMaker * cm = filled->getColorMaker();

    if (col == COL_SEL)
    {
        if (!cm->getFaceGroups().isSelected(row))
        {
            cm->getFaceGroups().select(row);
        }
        else
        {
            cm->getFaceGroups().deselect(row);
        }
    }
    else
    {
        cm->getFaceGroups().deselect();
    }

    emit sig_updateView();

    display();
}

void StyleColorFillSet::slot_double_click(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);

    modify();
}

void StyleColorFillSet::select(QPointF mpt)
{
    ColorMaker * cm = filled->getColorMaker();

    const FaceGroups & faceGroups = cm->getFaceGroups();

    int row = 0;
    for (const FaceSetPtr & faceSet : std::as_const(faceGroups))
    {
        for (const FacePtr & face : std::as_const(*faceSet))
        {
            if (face->getPolygon().containsPoint(mpt,Qt::OddEvenFill))
            {
                qDebug() << "select row" << row;
                fillSetTable->setCurrentCell(row,0);
                return;
            }
        }
        row++;
    }
}
