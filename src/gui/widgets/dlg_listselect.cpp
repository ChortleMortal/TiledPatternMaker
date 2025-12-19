#include <QPushButton>
#include <QMenu>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>

#include "gui/widgets/dlg_listselect.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "sys/sys/fileservices.h"
#include "sys/sys/pugixml.hpp"
#include "gui/widgets/layout_sliderset.h"

using namespace pugi;
using std::make_shared;

///////////////////////////////////////////////////////////////////
//
// DlgListSelect
//
///////////////////////////////////////////////////////////////////

DlgListSelect::DlgListSelect(VersionFileList & xfiles, QWidget *parent) : QDialog(parent)
{
    list = new LoaderListWidget();
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    list->setFixedHeight(600);

    QPushButton * cancelBtn = new QPushButton("Cancel");
    QPushButton * okBtn = new QPushButton("OK");
    okBtn->setDefault(true);

    QHBoxLayout * hBtnBox = new QHBoxLayout;
    hBtnBox->addStretch();
    hBtnBox->addWidget(cancelBtn);
    hBtnBox->addWidget(okBtn);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(list);
    vbox->addLayout(hBtnBox);

    hbox = new QHBoxLayout;
    hbox->addLayout(vbox);

    setLayout(hbox);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,     &QPushButton::clicked, this, &QDialog::accept);
    connect(list, &QListWidget::currentRowChanged,   this, &DlgListSelect::slot_currentRow);
    connect(list, &LoaderListWidget::leftDoubleClick,this, &DlgListSelect::slot_dclick);

    load(xfiles);
}

void DlgListSelect::load(VersionFileList & xfiles)
{
    _xfiles = xfiles;

    list->clear();

    for (VersionedFile & xfile : xfiles)
    {
        QListWidgetItem * item = new QListWidgetItem(xfile.getVersionedName().get());
        list->addItem(item);
    }

    list->setMinimumWidth(list->sizeHintForColumn(0) + 10);
    adjustSize();
}

void DlgListSelect::slot_currentRow(int row)
{
    if (row == -1)
    {
        selected.clear();
    }
    else
    {
        selected = _xfiles.at(row);
    }

    selectAction();
}

void DlgListSelect::slot_dclick(QPoint pos)
{
    Q_UNUSED(pos);

    int row = list->currentRow();
    slot_currentRow(row);
    accept();
}

///////////////////////////////////////////////////////////////////
//
// GirihListSelect
//
///////////////////////////////////////////////////////////////////

GirihListSelect::GirihListSelect(VersionFileList xfiles, QWidget * parent) : DlgListSelect (xfiles, parent)
{
    QVBoxLayout * vbox = new QVBoxLayout;
    frame = new AQFrame();
    frame->setFixedSize(300,300);
    frame->setStyleSheet("background-color: white;");
    frame->setFrameStyle(QFrame::Box | QFrame::Plain);
    frame->setLineWidth(1); 
    vbox->addWidget(frame);

    magSlider = new SliderSet("Size",20,10,100);
    magSlider->setRange(10,100);
    vbox->addLayout(magSlider);

    hbox->addLayout(vbox);

    setMouseTracking(true);
    list->setSelectionMode(QAbstractItemView::ExtendedSelection);

    frame->scale = 20;  // default

    connect(list, &LoaderListWidget::rightClick, this, &GirihListSelect::slot_rightClick);
    connect(magSlider,  &SliderSet::valueChanged, this, &GirihListSelect::magChanged);

    for (int i=0; i < list->count(); i++)
    {
        VersionFileList ignore;
        QListWidgetItem * qitem = list->item(i);
        if (isUsed(qitem->text(),ignore))
        {
            qitem->setForeground(Qt::red);
        }
    }
}

void GirihListSelect::slot_rightClick(QPoint pos)
{
    QMenu myMenu;
    myMenu.addAction("Where used",  this, SLOT(whereUsed()));
    myMenu.exec(list->mapToGlobal(pos));
}

void GirihListSelect::whereUsed()
{
    QString name = list->currentItem()->text();
    VersionFileList results;
    bool used = isUsed(name,results);

    // display results
    QString resultStr = "<pre>";
    if (used)
    {
        for (int i=0; i < results.size(); i++)
        {
            QString str = results.at(i).getVersionedName().get();
            resultStr  += str;
            resultStr += "<br>";
        }
    }
    else
    {
        resultStr += "<br><br>Girih shape NOT used<br><br>";
    }
    resultStr += "</pre>";

    QMessageBox box;
    box.setWindowTitle(QString("Where tiling %1 is used ").arg(name));
    box.setText(resultStr);
    box.exec();
}

bool GirihListSelect::isUsed(QString girihname, VersionFileList & results)
{
    VersionFileList tilings = FileServices::getFiles(FILE_TILING);
    for (const VersionedFile & tile : std::as_const(tilings))
    {
        if (containsGirih(girihname,tile))
        {
            results.add(tile);
        }
    }

    return (results.size() > 0);
}

bool GirihListSelect::containsGirih(QString girihName, VersionedFile file)
{
    xml_document doc;
    xml_parse_result result = doc.load_file(file.getPathedName().toStdString().c_str());  // load file
    if (result == false)
    {
        qWarning() << result.description();
        return false;
    }

    xml_node node1 = doc.child("Tiling");
    if (node1)
    {
        for (xml_node node2 = node1.child("Feature"); node2; node2 = node2.next_sibling("Feature"))
        {
            xml_attribute attr = node2.attribute("type");
            if (attr)
            {
                if (QString(attr.value()) == "girih")
                {
                    xml_attribute attr2 = node2.attribute("name");
                    if (QString(attr2.value()) == girihName)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

QStringList GirihListSelect::getSelected()
{
    QList<QListWidgetItem *> alist = list->selectedItems();
    QStringList blist;
    for (auto qlwi : std::as_const(alist))
    {
        QString name = qlwi->text();
        blist.push_back(name);
    }
    return blist;
}


void GirihListSelect::selectAction()
{
    Q_ASSERT(frame);

    qDebug() << "file:" << selected.getVersionedName().get();

    frame->setPlacedTile(make_shared<PlacedTile>());
    bool rv = frame->placedTile()->loadFromGirihShape(selected.getVersionedName());
    Q_ASSERT(rv);

    frame->update();
}


void GirihListSelect::magChanged(int mag)
{
    //frame->scale = magSlider->value();
    frame->scale = mag;
    frame->update();
}


void AQFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    qDebug() << "paint";

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    painter.setPen(QPen(Qt::black,3));
    TilePtr tp    = placedTile()->getTile();
    EdgePoly ep   = tp->getEdgePoly();
    QTransform tr = QTransform::fromTranslate(150,150);
    QTransform t2 = QTransform::fromScale(scale,scale);
    QTransform t3 = t2 * tr;
    ep.paint(&painter,t3);
}
