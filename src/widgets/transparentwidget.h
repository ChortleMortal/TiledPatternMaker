#pragma once
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
    virtual ~ImageWidget();

    void keyPressEvent(QKeyEvent *k) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

public slots:
    void slot_closeMe();
};

class TransparentWidget : public ImageWidget
{
public:
    TransparentWidget(QString name);
    virtual ~TransparentWidget(){}

protected:
    void paintEvent(QPaintEvent * event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *k) override;

private:
    QPoint  oldPos;
    bool    onTop;
    QString title;
    bool    dragging;
};

#endif // TRANSPARENTWIDGET_H
