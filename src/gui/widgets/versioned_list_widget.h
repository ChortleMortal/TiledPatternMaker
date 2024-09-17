#pragma once
#ifndef VERSIONED_LIST_WIDGET_H
#define VERSIONED_LIST_WIDGET_H

#include <QListWidget>
#include "sys/sys/versioning.h"

class LoaderListWidget : public QListWidget
{
    Q_OBJECT

public:
    LoaderListWidget(QWidget *parent = nullptr);

    void addItemList(QStringList &list);
    bool selectItemByName(QString name);
    bool selectItemByValue(QVariant val);

signals:
    void rightClick(QPoint pos);
    void leftDoubleClick(QPoint pos);
    void listEnter();

private:
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
};

class VersionedListWidget : public LoaderListWidget
{
public:
    VersionedListWidget();

    void addItemList(VersionFileList & list);
};

#endif
