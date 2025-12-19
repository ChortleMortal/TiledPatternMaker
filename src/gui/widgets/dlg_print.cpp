#include <QTimer>
#include "gui/top/system_view.h"
#include "gui/widgets/dlg_print.h"
#include "sys/sys.h"


DlgPrint::DlgPrint(QPrinter * printer, QWidget * widgetToPrint) : QPrintPreviewDialog(printer, widgetToPrint)
{
    timer = nullptr;
    setFixedSize(700,800);
    move(0,0);

    connect(this, &QPrintPreviewDialog::paintRequested,  this, [](QPrinter * printer) {Sys::sysview->render(printer);} );

    previewWidget = findChild<QPrintPreviewWidget*>();
    if (previewWidget)
    {
        timer = new QTimer(this);
        QObject::connect(timer, &QTimer::timeout, this, &DlgPrint::slot_updatePreview);

        timer->setInterval(250);
        timer->start();
    }
}

void DlgPrint::slot_updatePreview()
{
    previewWidget->updatePreview();
}

void DlgPrint::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    if (timer)
    {
        timer->stop();
        delete timer;
    }
}

#if 0
    // Connect the paintRequested signal to a slot that handles drawing
    connect(&previewDialog, &QPrintPreviewDialog::paintRequested, this, &MainWindow::renderPrintableContent);

    void renderPrintableContent(QPrinter *printer)
    {
        QPainter painter(printer); // Create a painter for the printer

        // Scale the content to fit the page if necessary
        double xscale = printer->pageRect().width() / static_cast<double>(myPrintableWidget->width());
        double yscale = printer->pageRect().height() / static_cast<double>(myPrintableWidget->height());
        double scale = qMin(xscale, yscale);

        painter.translate(printer->paperRect().x() + printer->pageRect().width() / 2,
                          printer->paperRect().y() + printer->pageRect().height() / 2);
        painter.scale(scale, scale);
        painter.translate(-myPrintableWidget->width() / 2, -myPrintableWidget->height() / 2);

        myPrintableWidget->render(&painter); // Render the widget onto the printer
    }
#endif
