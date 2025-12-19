#ifndef FLASH_MSGBOX_H
#define FLASH_MSGBOX_H

#include <QMessageBox>

class FlashMsgBox : public QMessageBox
{
public:
    FlashMsgBox(uint ms, QWidget * parent = nullptr);

    int exec() override;

private:
    uint period;   // milliseconds
};

#endif // FLASH_MSGBOX_H
