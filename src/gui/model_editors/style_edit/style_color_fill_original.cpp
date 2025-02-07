#include <QHeaderView>
#include "gui/model_editors/style_edit/style_color_fill_original.h"
#include "gui/model_editors/style_edit/style_editors.h"
#include "model/styles/filled.h"

StyleColorFillOriginal::StyleColorFillOriginal(FilledEditor * parent, FilledPtr style, QVBoxLayout * vbox)  : filled(style)
{
    this->parent        = parent;

    table = new QTableWidget();
    table->verticalHeader()->setVisible(false);
    table->setRowCount(filled->getWhiteColorSet()->size());
    table->setColumnCount(4);
    table->setRowCount(2);
    table->setColumnWidth(COL_SHOW,131);
    table->setColumnWidth(COL_COLORS,310);
    table->setColumnWidth(COL_EDIT,100);
    table->setColumnWidth(COL_NUMBER,120);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setMinimumHeight(501);

    QStringList qslH;
    qslH << "Show" <<  "Colors" << "Edit" << "Number";
    table->setHorizontalHeaderLabels(qslH);

    ColorMaker * cm = filled->getColorMaker();

    // whites
    int row = 0;
    outside_checkbox = new AQCheckBox("Inside (Whites)");   // is truth but does not match code
    outside_checkbox->setChecked(filled->getDrawOutsideWhites());
    table->setCellWidget(row,COL_SHOW,outside_checkbox);

    ColorSet * colorSetW    = filled->getWhiteColorSet();
    QWidget * widget        = colorSetW->createWidget();
    table->setCellWidget(row,COL_COLORS,widget);

    QPushButton * btnW = new QPushButton("Edit");
    table->setCellWidget(row,COL_EDIT,btnW);

    QTableWidgetItem * item = new QTableWidgetItem(QString("%1 faces").arg(cm->getWhiteFaces().size()));
    table->setItem(row,COL_NUMBER,item);

    row++;

    // blacks
    inside_checkbox = new AQCheckBox("Outside (Blacks)");   // is truth but does not match code
    inside_checkbox->setChecked(filled->getDrawInsideBlacks());
    table->setCellWidget(row,COL_SHOW,inside_checkbox);

    ColorSet * colorSetB    = filled->getBlackColorSet();
    widget                  = colorSetB->createWidget();
    table->setCellWidget(row,COL_COLORS,widget);

    QPushButton * btnB = new QPushButton("Edit");
    table->setCellWidget(row,COL_EDIT,btnB);

    item = new QTableWidgetItem(QString("%1 faces").arg(cm->getBlackFaces().size()));
    table->setItem(row,COL_NUMBER,item);

    connect(inside_checkbox,  &QCheckBox::clicked,      this,   &StyleColorFillOriginal::slot_insideChanged);
    connect(outside_checkbox, &QCheckBox::clicked,      this,   &StyleColorFillOriginal::slot_outsideChanged);
    connect(btnB,             &QPushButton::clicked,    parent, &FilledEditor::slot_editB);
    connect(btnW,             &QPushButton::clicked,    parent, &FilledEditor::slot_editW);

    vbox->addWidget(table);
}

void StyleColorFillOriginal::display()
{
}

void StyleColorFillOriginal::slot_insideChanged(bool checked)
{
    filled->setDrawInsideBlacks(checked);

    parent->slot_colorsChanged();
}

void StyleColorFillOriginal::slot_outsideChanged(bool checked)
{
    filled->setDrawOutsideWhites(checked);

    parent->slot_colorsChanged();
}
