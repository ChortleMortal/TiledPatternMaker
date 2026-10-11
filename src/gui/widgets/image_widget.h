#pragma once

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
    virtual void keyPressEvent(QKeyEvent *k) override;

private:
    QPixmap _pixmap;
};


class ComparedImageWidget : public ImageWidget
{
    Q_OBJECT

public:
    ComparedImageWidget(QImage & imgA, QImage & imgB, QImage & imgDiff, QString titleA, QString titleB);

    virtual QString gettypename()  override { return "comp"; }

protected:
    virtual void keyPressEvent(QKeyEvent *k) override;

private:
    QPixmap pixA;
    QPixmap pixB;
    QPixmap pixDiff;
    QString titleA;
    QString titleB;

    bool isA;
    QString title;
    QString kbdInfo;
};
