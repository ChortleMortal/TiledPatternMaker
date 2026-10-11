#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QGroupBox>
#include <QMessageBox>

#include "gui/widgets/dlg_rosette_tips.h"
#include "gui/model_editors/motif_edit/radial_motif_editors.h"

/////////////////////////////////////////////////////////////////////
///
/// DlgRosetteTips
///
/////////////////////////////////////////////////////////////////////

DlgRosetteTips::DlgRosetteTips(Rosette2Ptr rose, Rosette2Editor *parent) :  QDialog(parent)
{
    qDebug() << "DlgColorSet::DlgColorSet";

    this->rose   = rose;
    this->parent = parent;
    currentRow   = 0;
    tips         = rose->getTipTypes();  // local copy
    tipWidget    = nullptr;


    QPushButton * addBtn = new QPushButton("Add");
    QPushButton * delBtn = new QPushButton("Delete");
    QPushButton * upBtn  = new QPushButton("Up");
    QPushButton * dwnBtn = new QPushButton("Down");

    QHBoxLayout * tophbox = new QHBoxLayout();
    tophbox->addWidget(addBtn);
    tophbox->addWidget(delBtn);
    tophbox->addWidget(upBtn);
    tophbox->addWidget(dwnBtn);

    tipWidget = new QWidget();
    tgrid = new QGridLayout(tipWidget);

    QPushButton * okBtn  = new QPushButton("Done");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(okBtn);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(tophbox);
    vbox->addSpacing(11);
    vbox->addWidget(tipWidget);
    vbox->addStretch();
    vbox->addLayout(hbox);
    vbox->addSpacing(9);
    setLayout(vbox);

    setMinimumWidth(500);
    setMinimumHeight(300);

    connect(addBtn, &QPushButton::clicked, this, &DlgRosetteTips::add);
    connect(delBtn, &QPushButton::clicked, this, &DlgRosetteTips::del);
    connect(upBtn,  &QPushButton::clicked, this, &DlgRosetteTips::up);
    connect(dwnBtn, &QPushButton::clicked, this, &DlgRosetteTips::down);
    connect(okBtn,  &QPushButton::clicked, this, &DlgRosetteTips::slot_ok);

    displayTable();
}

void DlgRosetteTips::displayTable()
{
    // 1. Clear old widgets safely
    QLayoutItem *child;
    while ((child = tgrid->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    btns.clear();
    radios.clear();
    indepbtns.clear();

    for (int i = 0; i < tips.size(); i++)
    {
        // Radio Group Logic
        QButtonGroup * bg = new QButtonGroup(this);
        radios.push_back(bg);
        connect(bg, &QButtonGroup::buttonClicked, this, &DlgRosetteTips::selClicked);

        // PushButton Group Logic
        QButtonGroup * bg2 = new QButtonGroup(this);
        bg2->setExclusive(true); // Keep only ONE row selected at a time
        indepbtns.push_back(bg2);
        connect(bg2, &QButtonGroup::buttonClicked, this, &DlgRosetteTips::selectRow);

        // The Select Button
        QPushButton * btn = new QPushButton("Select");
        btn->setCheckable(true);
        btn->setProperty("row", i);
        // Use a stylesheet once to handle the "Green" state automatically
        btn->setStyleSheet("QPushButton:checked { background-color: LightGreen; color: black; }");

        if (i == currentRow)
            btn->setChecked(true); // Restore selection after redraw
        else
            btn->setChecked(false); // Restore selection after redraw

        tgrid->addWidget(btn, i, 0);
        btns.push_back(btn);
        bg2->addButton(btn);

        // The Radio Buttons
        static const TipType types[] = { TipType::OUTER, TipType::OUTER_TO_BOUNDARY,
                                         TipType::OUTER_POINT, TipType::OUTER_POINT_TO_BOUNDARY,
                                         TipType::INNER, TipType::FLIPPED };
        TipType currentType = tips[i];
        int col = 1;
        for (TipType type : types)
        {
            int itype         = static_cast<int>(type);
            QRadioButton * rb = new QRadioButton(sTipType[itype]);
            rb->setProperty("row",i);
            rb->setProperty("id",itype);
            tgrid->addWidget(rb,i,col++);
            bg->addButton(rb, itype);
            if(currentType == type) rb->setChecked(true);
        }
    }
}

void DlgRosetteTips::selectRow(QAbstractButton * btn)
{
    // Because bg2->setExclusive(true) is set, Qt handles unchecking
    // the previous button for you automatically.
    currentRow = btn->property("row").toInt();
    //qInfo() << "Selected Row:" << currentRow;
    for (int i=0; i < btns.size(); i++)
    {
        auto abtn = btns[i];
        if (abtn != btn)
        {
            abtn->setChecked(false);
            abtn->setStyleSheet("");
        }
        else
        {
            btn->setStyleSheet("QPushButton:checked { background-color: LightGreen; color: black; }");
        }
    }
}

void DlgRosetteTips::add()
{
    tips.push_back(TipType::OUTER);

    setTips();
}

void DlgRosetteTips::del()
{
    if (currentRow < 0 || currentRow >= tips.size())
    {
        QMessageBox box;
        box.setText("Select a tip type to delete");
        box.setIcon(QMessageBox::Information);
        box.exec();
        return;
    }

    if (btns.size() < 2)
    {
        QMessageBox box;
        box.setText("Cannot delete: there must be one tip type");
        box.setIcon(QMessageBox::Warning);
        box.exec();
        return;
    }

    tips.removeAt(currentRow);

    setTips();
}

void DlgRosetteTips::slot_ok()
{
    accept();
}

void DlgRosetteTips::up()
{
    if (currentRow < 1)
        return;

    TipType a = tips[currentRow];
    TipType b = tips[currentRow-1];

    tips[currentRow-1] = a;
    tips[currentRow]   = b;

    currentRow--;

    setTips();
}

void DlgRosetteTips::down()
{
    if ( (currentRow < 0) || (currentRow >= (tips.size()-1)) )
        return;

    TipType a = tips[currentRow];
    TipType b = tips[currentRow+1];

    tips[currentRow+1] = a;
    tips[currentRow]   = b;

    currentRow++;

    setTips();
}

void DlgRosetteTips::setTips()
{
    rose->setTipTypes(tips);
    parent->motifModified(rose);
    displayTable();
}

void DlgRosetteTips::selClicked(QAbstractButton * btn)
{
    int row =  btn->property("row").toInt();
    int id  =  btn->property("id").toInt();
    TipType tt = TipType(id);
    //qInfo() << "row" << row << "id" << id;
    tips[row] = tt;
    setTips();
}
