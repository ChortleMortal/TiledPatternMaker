#include <QHeaderView>
#include <QTableWidget>

#include "gui/model_editors/style_edit/fill_new1_editor.h"
#include "gui/model_editors/style_edit/style_editor.h"
#include "gui/widgets/colorset_widget.h"
#include "gui/widgets/dlg_colorSet.h"
#include "gui/widgets/panel_misc.h"

///////////////////////////////////////////////////////////////////////
//
//  StyleColorFillNew1
//
///////////////////////////////////////////////////////////////////////

FillNew1Editor::FillNew1Editor(FilledEditor * parent, FilledPtr filled, New1Coloring * cm, QVBoxLayout * vbox)
    : FilledSubTypeEditor(parent,filled,cm)
{
    n1cm         = cm;

    table = new QTableWidget();
    table->verticalHeader()->setVisible(false);
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

    // whites
    int row = 0;
    outside_checkbox = new AQCheckBox("Outside (Whites)");   // is truth but does not match code
    table->setCellWidget(row,COL_SHOW,outside_checkbox);

    ColorSet * colorSetW    = &cm->whiteColorSet;
    cswW = new ColorSetWidget(parent,colorSetW);
    table->setCellWidget(row,COL_COLORS,cswW);

    QPushButton * btnW = new QPushButton("Edit");
    table->setCellWidget(row,COL_EDIT,btnW);

    infoW = new QTableWidgetItem(QString("%1 faces").arg(n1cm->whiteFaces.size()));
    table->setItem(row,COL_NUMBER,infoW);

    row++;

    // blacks
    inside_checkbox = new AQCheckBox("Inside (Blacks)");   // is truth but does not match code
    table->setCellWidget(row,COL_SHOW,inside_checkbox);

    ColorSet * colorSetB    = &cm->blackColorSet;
    cswB = new ColorSetWidget(parent,colorSetB);
    table->setCellWidget(row,COL_COLORS,cswB);

    QPushButton * btnB = new QPushButton("Edit");
    table->setCellWidget(row,COL_EDIT,btnB);

    infoB = new QTableWidgetItem(QString("%1 faces").arg(n1cm->blackFaces.size()));
    table->setItem(row,COL_NUMBER,infoB);

    connect(inside_checkbox,  &QCheckBox::clicked,      this,   &FillNew1Editor::slot_insideChanged);
    connect(outside_checkbox, &QCheckBox::clicked,      this,   &FillNew1Editor::slot_outsideChanged);
    connect(btnB,             &QPushButton::clicked,    this,   &FillNew1Editor::slot_editB);
    connect(btnW,             &QPushButton::clicked,    this,   &FillNew1Editor::slot_editW);
    connect(cswW,             &ColorSetWidget::sig_updateView, Sys::viewController, &SystemViewController::slot_updateView);

    vbox->addWidget(table);
}

void FillNew1Editor::refresh()
{
    inside_checkbox->setChecked( n1cm->draw_inside_blacks);
    outside_checkbox->setChecked(n1cm->draw_outside_whites);

    cswB->updateFromColorSet();
    cswW->updateFromColorSet();

    infoW->setText(QString("%1 faces").arg(n1cm->whiteFaces.size()));
    infoB->setText(QString("%1 faces").arg(n1cm->blackFaces.size()));
}

void FillNew1Editor::colorPick(QColor color)
{
    Q_UNUSED(color);
}

void FillNew1Editor::notify()
{
    emit parent->sig_updateView();
}

void FillNew1Editor::mousePressed(QPointF mpt, Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    const FaceSet & fsetW = n1cm->whiteFaces;
    for (auto & face : fsetW)
    {
        if (face->contains(mpt))
        {
            qDebug() << "Face is white 1";
            return;
        }
    }

    const FaceSet & fsetB = n1cm->blackFaces;
    for (auto & face : fsetB)
    {
        if (face->contains(mpt))
        {
            qDebug() << "Face is black 1";
            return;
        }
    }
    qDebug() << "Face unknown 1";
}

void FillNew1Editor::slot_insideChanged(bool checked)
{
    n1cm->draw_inside_blacks = checked;

    emit parent->sig_updateView();
}

void FillNew1Editor::slot_outsideChanged(bool checked)
{
    n1cm->draw_outside_whites = checked;

    emit parent->sig_updateView();
}

void FillNew1Editor::slot_editB()
{
    DlgColorSet dlg(&n1cm->blackColorSet,table);

    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, parent, &FilledEditor::slot_colorsChanged, Qt::QueuedConnection);

    dlg.exec();

    emit parent->sig_updateView();
}

void FillNew1Editor::slot_editW()
{
    DlgColorSet dlg(&n1cm->whiteColorSet,table);

    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, parent, &FilledEditor::slot_colorsChanged, Qt::QueuedConnection);

    dlg.exec();

    emit parent->sig_updateView();
}
