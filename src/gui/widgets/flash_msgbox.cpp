#include <QTimer>
#include "gui/widgets/flash_msgbox.h"

FlashMsgBox::FlashMsgBox(uint ms, QWidget * parent) : QMessageBox(parent)
{
    period = ms;
}

int FlashMsgBox::exec()
{
   QTimer::singleShot(std::chrono::milliseconds(period), this, [this]() { accept(); } );
   int result = QMessageBox::exec();
   return result;
}
