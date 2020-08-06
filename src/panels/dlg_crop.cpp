#include "dlg_crop.h"

#include <QtWidgets>

DlgCrop::DlgCrop(MapEditor * me, QWidget *parent) : QDialog(parent)
{
    mapeditor = me;
    entered   = false;

    setWindowTitle("Crop Mask Parameters");

    QPushButton * okBtn      = new QPushButton("Crop");
    QPushButton * cancelBtn  = new QPushButton("Cancel");

    QLabel * label1 = new QLabel("width");
    QLabel * label2 = new QLabel("height");
    QLabel * label3 = new QLabel("top-left-X");
    QLabel * label4 = new QLabel("top-left-Y");

    width  = new QDoubleSpinBox;
    height = new QDoubleSpinBox;
    startX = new QDoubleSpinBox;
    startY = new QDoubleSpinBox;

    width->setRange(1.0,50.0);
    height->setRange(1.0,50.0);
    startX->setRange(-50.0,50.0);
    startY->setRange(-50.0,50.0);

    widthS  = new QDoubleSpinBox;
    heightS = new QDoubleSpinBox;
    startXS = new QDoubleSpinBox;
    startYS = new QDoubleSpinBox;

    widthS->setRange(1.0,4096.0);
    heightS->setRange(1.0,2040.0);
    startXS->setRange(-4096.0,4096.0);
    startYS->setRange(-2160.0,2160.0);

    QGridLayout * grid = new QGridLayout();
    int row = 0;
    grid->addWidget(label1,row,0);
    grid->addWidget(width,row,1);
    grid->addWidget(widthS,row,2);

    row++;
    grid->addWidget(label2,row,0);
    grid->addWidget(height,row,1);
    grid->addWidget(heightS,row,2);

    row++;
    grid->addWidget(label3,row,0);
    grid->addWidget(startX,row,1);
    grid->addWidget(startXS,row,2);

    row++;
    grid->addWidget(label4,row,0);
    grid->addWidget(startY,row,1);
    grid->addWidget(startYS,row,2);

    row++;
    row++;
    grid->addWidget(okBtn,row,1);
    grid->addWidget(cancelBtn,row,2);


    setLayout(grid);

    QTimer * timer = new QTimer();

    connect(timer,     &QTimer::timeout,      this, &DlgCrop::slot_timeout);
    connect(okBtn,     &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(width,     SIGNAL(valueChanged(qreal)), this, SLOT(slot_setCrop(qreal)));
    connect(height,     SIGNAL(valueChanged(qreal)), this, SLOT(slot_setCrop(qreal)));
    connect(startX,     SIGNAL(valueChanged(qreal)), this, SLOT(slot_setCrop(qreal)));
    connect(startY,     SIGNAL(valueChanged(qreal)), this, SLOT(slot_setCrop(qreal)));

    timer->start(100);
}

void DlgCrop::slot_timeout()
{
    if (entered)
    {
        return;
    }

    QRectF rect  = mapeditor->getCropRect();
    if (rect.isEmpty())
    {
        return;
    }

    QTransform t = mapeditor->getLayerTransform();
    QRectF rectS = t.mapRect(rect);

    blockSignals(true);
    width->setValue(rect.width());
    height->setValue(rect.height());
    startX->setValue(rect.topLeft().x());
    startY->setValue(rect.topLeft().y());

    widthS->setValue(rectS.width());
    heightS->setValue(rectS.height());
    startXS->setValue(rectS.topLeft().x());
    startYS->setValue(rectS.topLeft().y());
    blockSignals(false);
}

void DlgCrop::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    entered = true;
}

void DlgCrop::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    entered = false;
}

void DlgCrop::slot_setCrop(qreal)
{
    QRectF rect;
    QPointF pos = QPointF(startX->value(), startY->value());
    rect.setTopLeft(pos);
    rect.setWidth(width->value());
    rect.setHeight(height->value());
    mapeditor->setCropRect(rect);
    mapeditor->forceRedraw();
}
