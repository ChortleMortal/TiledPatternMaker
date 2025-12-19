#ifndef DLG_PRINT_H
#define DLG_PRINT_H

#include <QObject>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>

class DlgPrint : public QPrintPreviewDialog
{
    Q_OBJECT

public:
    DlgPrint(QPrinter * printer, QWidget *widgetToPrint);

protected slots:
    void slot_updatePreview();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QPrintPreviewWidget * previewWidget;
    QTimer * timer;
};

#endif // DLG_PRINT_H
