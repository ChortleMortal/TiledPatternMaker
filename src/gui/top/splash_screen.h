#pragma once

#include <QStack>
#include <QWidget>

class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    SplashScreen();

    void display(QString & txt);
    void replace(QString & txt);
    void display(QString && txt);
    void remove();

    void disable(bool disable);

    void keyPressEvent(QKeyEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

protected:
    void draw();

    void paintEvent(QPaintEvent*) override;

private:
    QStack<QString>  msgStack;
    bool            _disable;

    QPixmap         m_pixmap{":/tpm.png"};
    QString         m_text;
};
