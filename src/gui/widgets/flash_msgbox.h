#pragma once

#include <QMessageBox>

class FlashMsgBox : public QMessageBox
{
public:
    FlashMsgBox(uint ms, QWidget * parent = nullptr);

    int exec() override;

private:
    uint period;   // milliseconds
};
