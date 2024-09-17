#include <QHeaderView>
#include <QCheckBox>

#include "gui/model_editors/style_edit/style_color_fill_group.h"
#include "gui/widgets/dlg_colorSet.h"
#include "sys/sys.h"
#include "gui/top/view.h"
#include "model/styles/filled.h"

StyleColorFillGroup::StyleColorFillGroup(FilledPtr style, QVBoxLayout *vbox) : filled(style)
{
    QGridLayout * grid = new QGridLayout;

    QPushButton * modBtn = new QPushButton("Modify");
    QPushButton * upBtn  = new QPushButton("Up");
    QPushButton * dwnBtn = new QPushButton("Down");
    QPushButton * rptBtn = new QPushButton("Repeat Set");
    QPushButton * cpyBtn = new QPushButton("Copy Set");
    QPushButton * pstBtn = new QPushButton("Paste Set");

    grid->addWidget(modBtn,0,1);
    grid->addWidget(cpyBtn,0,3);
    grid->addWidget(upBtn, 1,0);
    grid->addWidget(dwnBtn,1,1);
    grid->addWidget(rptBtn,1,2);
    grid->addWidget(pstBtn,1,3);
    vbox->addLayout(grid);

    connect(modBtn, &QPushButton::clicked, this, &StyleColorFillGroup::modify);
    connect(upBtn,  &QPushButton::clicked, this, &StyleColorFillGroup::up);
    connect(dwnBtn, &QPushButton::clicked, this, &StyleColorFillGroup::down);
    connect(rptBtn, &QPushButton::clicked, this, &StyleColorFillGroup::rptSet);
    connect(cpyBtn, &QPushButton::clicked, this, &StyleColorFillGroup::copySet);
    connect(pstBtn, &QPushButton::clicked, this, &StyleColorFillGroup::pasteSet);

    fillGroupTable = new QTableWidget();
    fillGroupTable->verticalHeader()->setVisible(false);
    fillGroupTable->setColumnCount(8);
    fillGroupTable->setColumnWidth(COL_INDEX,35);
    fillGroupTable->setColumnWidth(COL_COUNT,40);
    fillGroupTable->setColumnWidth(COL_SIDES,40);
    fillGroupTable->setColumnWidth(COL_AREA,70);
    fillGroupTable->setColumnWidth(COL_HIDE,80);
    fillGroupTable->setColumnWidth(COL_SEL,40);
    fillGroupTable->setColumnWidth(COL_BTN,70);
    fillGroupTable->setSelectionMode(QAbstractItemView::SingleSelection);
    fillGroupTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    fillGroupTable->setMinimumHeight(501);

    QStringList qslH;
    qslH << "" << "Count" << "Sides" << "Area" << "Hide" << "Sel" << "Edit" << "Colors";
    fillGroupTable->setHorizontalHeaderLabels(qslH);

    connect(fillGroupTable,  &QTableWidget::cellClicked,       this, &StyleColorFillGroup::slot_click);
    connect(fillGroupTable,  &QTableWidget::cellDoubleClicked, this, &StyleColorFillGroup::slot_double_click);

    vbox->addWidget(fillGroupTable);

    connect(this, &StyleColorFillGroup::sig_updateView, Sys::view, &View::slot_update);
}

void StyleColorFillGroup::display()
{
    Q_ASSERT(fillGroupTable);
    qDebug() << "table is" << fillGroupTable;

    ColorMaker * cm = filled->getColorMaker();
    //QModelIndex selected = fillGroupTable->currentIndex();
    int crow = fillGroupTable->currentRow();

    fillGroupTable->clearContents();

    fillGroupTable->setRowCount(cm->getFaceGroups().size());

    int row = 0;
    for (const auto & face : std::as_const(cm->getFaceGroups()))
    {
        QTableWidgetItem * item = new QTableWidgetItem(QString::number(row));
        fillGroupTable->setItem(row,COL_INDEX,item);

        item = new QTableWidgetItem(QString::number(face->size()));
        fillGroupTable->setItem(row,COL_COUNT,item);

        item = new QTableWidgetItem(QString::number(face->sides));
        fillGroupTable->setItem(row,COL_SIDES,item);

        item = new QTableWidgetItem(QString::number(face->area));
        fillGroupTable->setItem(row,COL_AREA,item);

        QCheckBox * cb = new AQCheckBox("Hide");
        fillGroupTable->setCellWidget(row,COL_HIDE,cb);
        cb->setChecked(filled->getColorGroup()->isHidden(row));
        connect(cb, &QCheckBox::toggled, this, [this,row] { colorSetVisibilityChanged(row); });

        QPushButton * btn = new QPushButton("Edit");
        btn->setFixedWidth(40);
        fillGroupTable->setCellWidget(row,COL_BTN,btn);
        connect(btn, &QPushButton::clicked, this, [this,row] { edit(row); });

        ColorSet *  cset = filled->getColorGroup()->getColorSet(row);
        QWidget * widget = cset->createWidget();
        widget->setContentsMargins(0,0,0,0);
        fillGroupTable->setCellWidget(row,COL_COLORS,widget);

        QString astring;
        if (face->selected) astring = "X";
        item = new QTableWidgetItem(astring);
        fillGroupTable->setItem(row,COL_SEL,item);
        row++;
    }

    fillGroupTable->setColumnWidth(COL_BTN,90);
    fillGroupTable->resizeColumnToContents(5);
    fillGroupTable->resizeColumnToContents(6);

    if (crow < 0)
    {
        crow = 0;
    }
    fillGroupTable->setCurrentCell(crow,0);
}

void StyleColorFillGroup::edit(int row)
{
    if (row < 0 || row >= filled->getColorGroup()->size())
    {
        return;
    }

    ColorSet * colorSet = filled->getColorGroup()->getColorSet(row);

    DlgColorSet dlg(colorSet);

    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, this, &StyleColorFillGroup::sig_colorsChanged);

    dlg.exec();
}

void StyleColorFillGroup::modify()
{
    int row = fillGroupTable->currentRow();
    edit(row);
}

void StyleColorFillGroup::up()
{
    int currentRow = fillGroupTable->currentRow();
    if (currentRow < 1)
        return;

    ColorSet a = *filled->getColorGroup()->getColorSet(currentRow);
    ColorSet b = *filled->getColorGroup()->getColorSet(currentRow-1);

    filled->getColorGroup()->setColorSet(currentRow-1, a);
    filled->getColorGroup()->setColorSet(currentRow  , b);

    emit sig_colorsChanged();
}

void StyleColorFillGroup::down()
{
    int currentRow = fillGroupTable->currentRow();
    if (currentRow >= (filled->getColorGroup()->size()-1))
        return;

    ColorSet a = *filled->getColorGroup()->getColorSet(currentRow);
    ColorSet b = *filled->getColorGroup()->getColorSet(currentRow+1);

    filled->getColorGroup()->setColorSet(currentRow+1, a);
    filled->getColorGroup()->setColorSet(currentRow  , b);

    emit sig_colorsChanged();
}

void StyleColorFillGroup::rptSet()
{
    int currentRow = fillGroupTable->currentRow();
    if (currentRow < 0 || currentRow >= filled->getColorGroup()->size())
        return;

    copyPasteSet = *filled->getColorGroup()->getColorSet(currentRow);
    filled->getColorGroup()->resize(fillGroupTable->rowCount());
    for (int i = currentRow + 1; i < fillGroupTable->rowCount(); i++)
    {
        filled->getColorGroup()->setColorSet(i,copyPasteSet);
    }

    emit sig_colorsChanged();
}

void StyleColorFillGroup::copySet()
{
    qDebug() << "StyleColorFillGroup::copySet()";
    int currentRow = fillGroupTable->currentRow();
    if (currentRow < 0 || currentRow >= filled->getColorGroup()->size())
        return;

    copyPasteSet = *filled->getColorGroup()->getColorSet(currentRow);
}

void StyleColorFillGroup::pasteSet()
{
    qDebug() << "StyleColorFillGroup::pasteSet()";
    int currentRow = fillGroupTable->currentRow();
    if (currentRow < 0 || currentRow >= filled->getColorGroup()->size())
        return;

    filled->getColorGroup()->setColorSet(currentRow,copyPasteSet);

    emit sig_colorsChanged();
}

void StyleColorFillGroup::colorSetVisibilityChanged(int row)
{
    qDebug() << "colorVisibilityChanged row=" << row;

    QCheckBox * cb  = dynamic_cast<QCheckBox*>(fillGroupTable->cellWidget(row,COL_HIDE));
    bool hide       = cb->isChecked();
    qDebug() << "group hide state="  << hide;

    filled->getColorGroup()->hide(row, hide);

    emit sig_updateView();

    qDebug() << "colorVisibilityChanged: done";
}

void StyleColorFillGroup::slot_click(int row, int col)
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

void StyleColorFillGroup::slot_double_click(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);

    modify();
}

void StyleColorFillGroup::select(QPointF mpt)
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
                fillGroupTable->setCurrentCell(row,0);
                return;
            }
        }
        row++;
    }
}

void StyleColorFillGroup::setColor(QColor color)
{
    qDebug() << "StyleColorFillGroup::setColor" << color;

    int currentRow = fillGroupTable->currentRow();
    if (currentRow < 0 || currentRow >= filled->getColorGroup()->size())
        return;

    // this adds a color to the face set
    ColorSet * cset = filled->getColorGroup()->getColorSet(currentRow);
    cset->addColor(color,false);
    filled->getColorGroup()->hide(currentRow, false);

    emit sig_colorsChanged();
}

