#include <QPushButton>
#include <QMenu>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>

#include "widgets/dlg_listselect.h"
#include "misc/fileservices.h"
#include "misc/pugixml.hpp"
#include "widgets/layout_sliderset.h"
#include "tile/placed_tile.h"

using namespace pugi;
using std::make_shared;

///////////////////////////////////////////////////////////////////
//
// DlgListSelect
//
///////////////////////////////////////////////////////////////////

DlgListSelect::DlgListSelect(QStringList files, QWidget *parent) : QDialog(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    list = new LoaderListWidget();
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    hbox = new QHBoxLayout;
    hbox->addWidget(list);

    QPushButton * cancelBtn = new QPushButton("Cancel");
    QPushButton * okBtn = new QPushButton("OK");
    okBtn->setDefault(true);

    QHBoxLayout * hBtnBox = new QHBoxLayout;
    hBtnBox->addWidget(cancelBtn);
    hBtnBox->addWidget(okBtn);
    hBtnBox->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addLayout(hBtnBox);
    setLayout(vbox);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,     &QPushButton::clicked, this, &QDialog::accept);

    for (auto & file : std::as_const(files))
    {
        QListWidgetItem * item = new QListWidgetItem(file);
        list->addItem(item);
    }
    list->setMinimumWidth(list->sizeHintForColumn(0) + 10);
    list->setMinimumHeight((list->sizeHintForRow(0) * files.size()) + 10);
    adjustSize();

    connect(list, &QListWidget::currentRowChanged,   this, &DlgListSelect::slot_currentRow);
    connect(list, &LoaderListWidget::leftDoubleClick,this, &DlgListSelect::slot_dclick);
}

void DlgListSelect::slot_currentRow(int row)
{
    if (row == -1)
    {
        selectedFile.clear();
    }
    else
    {
        QListWidgetItem * item = list->item(row);
        selectedFile = item->text();
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

GirihListSelect::GirihListSelect(QStringList names, QWidget * parent) : DlgListSelect (names, parent)
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
        QStringList ignore;
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
    QStringList results;
    bool used = isUsed(name,results);
    qDebug() << results;

    // display results
    QString resultStr = "<pre>";
    if (used)
    {
        for (int i=0; i < results.size(); i++)
        {
            QString str = results[i];
            resultStr  += str;
            resultStr += "<br>";
        }
    }
    else
    {
        resultStr += "<br><br>Girih shape NOT used<br><br>";
    }
    results += "</pre>";

    QMessageBox box;
    box.setWindowTitle(QString("Where tiling %1 is used ").arg(name));
    box.setText(resultStr);
    box.exec();
}

bool GirihListSelect::isUsed(QString girihname, QStringList & results)
{
    QStringList tilings = FileServices::getTilingFiles();
    for (auto & tile : std::as_const(tilings))
    {
        if (containsGirih(girihname,tile))
        {
            results << tile;
        }
    }

    return (results.size() > 0);
}

bool GirihListSelect::containsGirih(QString girihName, QString filename)
{
    xml_document doc;
    xml_parse_result result = doc.load_file(filename.toStdString().c_str());  // load file
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
    qDebug() << "file:" << selectedFile;

    frame->tile = make_shared<PlacedTile>();
    bool rv = frame->tile->loadFromGirihShape(selectedFile);
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
    EdgePoly ep = tile->getTileEdgePoly();
    QTransform tr = QTransform::fromTranslate(150,150);
    QTransform t2 = QTransform::fromScale(scale,scale);
    QTransform t3 = t2 * tr;
    ep.paint(&painter,t3);
}
