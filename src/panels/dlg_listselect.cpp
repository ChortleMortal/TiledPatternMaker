#include "dlg_listselect.h"
#include "base/fileservices.h"
#include "base/pugixml.hpp"

using namespace pugi;

///////////////////////////////////////////////////////////////////
//
// DlgListSelect
//
///////////////////////////////////////////////////////////////////

DlgListSelect::DlgListSelect(QStringList files)
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

    for (auto it = files.begin(); it != files.end(); it++)
    {
        QString file = *it;
        QListWidgetItem * item = new QListWidgetItem(file);
        list->addItem(item);
    }
    list->setMinimumWidth(list->sizeHintForColumn(0) + 10);
    list->setMinimumHeight((list->sizeHintForRow(0) * files.size()) + 10);
    adjustSize();

    connect(list, &QListWidget::currentRowChanged, this, &DlgListSelect::slot_currentRow);
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

///////////////////////////////////////////////////////////////////
//
// GirihListSelect
//
///////////////////////////////////////////////////////////////////

GirihListSelect::GirihListSelect(QStringList names) : DlgListSelect (names)
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
    for (auto it = tilings.begin(); it != tilings.end(); it++)
    {
        QString tile = *it;
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
    for (auto it = alist.begin(); it != alist.end(); it++)
    {
        QListWidgetItem * qlwi = *it;
        QString name = qlwi->text();
        blist.push_back(name);
    }
    return blist;
}


void GirihListSelect::selectAction()
{
    qDebug() << "file:" << selectedFile;

    frame->feature = make_shared<PlacedFeature>();
    bool rv = frame->feature->loadFromGirihShape(selectedFile);
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
    painter.setRenderHint(QPainter::Antialiasing ,true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);

    painter.setPen(QPen(Qt::black,3));
    EdgePoly ep = feature->getFeatureEdgePoly();
    QTransform tr = QTransform::fromTranslate(150,150);
    QTransform t2 = QTransform::fromScale(scale,scale);
    QTransform t3 = t2 * tr;
    ep.paint(&painter,t3);
}
