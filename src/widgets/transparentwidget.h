#ifndef TRANSPARENTWIDGET_H
#define TRANSPARENTWIDGET_H

#include <QLabel>
#include <QMouseEvent>

typedef std::shared_ptr<class Map>  MapPtr;

class ImageWidget : public QLabel
{
    Q_OBJECT

public:
    ImageWidget();

    void keyPressEvent(QKeyEvent *k) override;

public slots:
    void slot_closeMe();

};

class TransparentWidget : public ImageWidget
{
public:
    TransparentWidget(QString name);

protected:
    void paintEvent(QPaintEvent * event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *k) override;

private:
    QPoint  oldPos;
    bool    onTop;
    QString title;
};

#endif // TRANSPARENTWIDGET_H
