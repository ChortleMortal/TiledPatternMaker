#pragma once
#ifndef IMAGE_WIDGET_H
#define IMAGE_WIDGET_H

#include <QLabel>
#include <QMouseEvent>

typedef std::shared_ptr<class Map>  MapPtr;

class ImageWidget : public QLabel
{
    Q_OBJECT

public:
    ImageWidget();
    virtual ~ImageWidget();

    virtual void setContentSize(QSize sz);

    static QPixmap removeAlphaChannel(const QPixmap &src);

    virtual QString gettypename() { return "image"; }

    void setPixmap(const QPixmap & pixmap);
    void setPixmap(const QPixmap & pixmap, qreal scale);
    QPixmap getPixmap() { return _pixmap; }

    void closeEvent(QCloseEvent * event) override;

signals:
    void sig_keyPressed(QKeyEvent *);
    void sig_closed(ImageWidget *);

public slots:
    void slot_closeMe();
    void slot_moveToPrimary();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *k) override;



private:
    QPixmap _pixmap;
};

#endif
