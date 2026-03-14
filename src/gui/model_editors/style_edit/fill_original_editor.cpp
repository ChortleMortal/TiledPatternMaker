#include <QHeaderView>
#include <QTableWidget>
#include <QPushButton>

#include "gui/widgets/panel_misc.h"
#include "gui/model_editors/style_edit/fill_original_editor.h"
#include "gui/widgets/colorset_widget.h"
#include "gui/widgets/dlg_colorSet.h"

///////////////////////////////////////////////////////////////////////
//
//  StyleColorFillOriginal
//
///////////////////////////////////////////////////////////////////////

FillOriginalEditor::FillOriginalEditor(FilledEditor * parent, FilledPtr filled, OriginalColoring * cm, QVBoxLayout * vbox)
    : FilledSubTypeEditor(parent,filled,cm)
{
    ocm = cm;

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

    ColorSet * colorSetW    = &ocm->whiteColorSet;
    cswW = new ColorSetWidget(parent,colorSetW);
    table->setCellWidget(row,COL_COLORS,cswW);

    QPushButton * btnW = new QPushButton("Edit");
    table->setCellWidget(row,COL_EDIT,btnW);

    infoW = new QTableWidgetItem(QString("%1 faces").arg(ocm->whiteFaces.size()));
    table->setItem(row,COL_NUMBER,infoW);

    row++;

    // blacks
    inside_checkbox = new AQCheckBox("Inside (Blacks)");   // is truth but does not match code
    table->setCellWidget(row,COL_SHOW,inside_checkbox);

    ColorSet * colorSetB    = &ocm->blackColorSet;
    cswB = new ColorSetWidget(parent,colorSetB);
    table->setCellWidget(row,COL_COLORS,cswB);

    QPushButton * btnB = new QPushButton("Edit");
    table->setCellWidget(row,COL_EDIT,btnB);

    infoB = new QTableWidgetItem(QString("%1 faces").arg(ocm->blackFaces.size()));
    table->setItem(row,COL_NUMBER,infoB);

    connect(inside_checkbox,  &QCheckBox::clicked,      this,   &FillOriginalEditor::slot_insideChanged);
    connect(outside_checkbox, &QCheckBox::clicked,      this,   &FillOriginalEditor::slot_outsideChanged);
    connect(btnB,             &QPushButton::clicked,    this,   &FillOriginalEditor::slot_editB);
    connect(btnW,             &QPushButton::clicked,    this,   &FillOriginalEditor::slot_editW);
    connect(cswW,             &ColorSetWidget::sig_updateView, Sys::viewController, &SystemViewController::slot_updateView);

    vbox->addWidget(table);
}

void FillOriginalEditor::refresh()
{
    inside_checkbox->setChecked( ocm->draw_inside_blacks);
    outside_checkbox->setChecked(ocm->draw_outside_whites);

    cswB->updateFromColorSet();
    cswW->updateFromColorSet();

    infoW->setText(QString("%1 faces").arg(ocm->whiteFaces.size()));
    infoB->setText(QString("%1 faces").arg(ocm->blackFaces.size()));
}

void FillOriginalEditor::colorPick(QColor color)
{
    Q_UNUSED(color);
}

void FillOriginalEditor::notify()
{
    emit parent->sig_updateView();
}

void FillOriginalEditor::mousePressed(QPointF mpt, Qt::MouseButton btn)
{
    Q_UNUSED(btn);
    const FaceSet & fsetW = ocm->whiteFaces;
    for (auto & face : fsetW)
    {
        if (face->contains(mpt))
        {
            qDebug() << "Face is white 0";
            return;
        }
    }

    const FaceSet & fsetB = ocm->blackFaces;
    for (auto & face : fsetB)
    {
        if (face->contains(mpt))
        {
            qDebug() << "Face is black 0";
            return;
        }
    }
    qDebug() << "Face unknown 0";

}

void FillOriginalEditor::slot_insideChanged(bool checked)
{
    ocm->draw_inside_blacks = checked;

    emit parent->sig_updateView();
}

void FillOriginalEditor::slot_outsideChanged(bool checked)
{
    ocm->draw_outside_whites = checked;

    emit parent->sig_updateView();
}

void FillOriginalEditor::slot_editB()
{
    DlgColorSet dlg(&ocm->blackColorSet,table);

    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, parent, &FilledEditor::slot_colorsChanged, Qt::QueuedConnection);

    dlg.exec();

    emit parent->sig_updateView();
}

void FillOriginalEditor::slot_editW()
{
    DlgColorSet dlg(&ocm->whiteColorSet,table);

    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, parent, &FilledEditor::slot_colorsChanged, Qt::QueuedConnection);

    dlg.exec();

    emit parent->sig_updateView();
}
