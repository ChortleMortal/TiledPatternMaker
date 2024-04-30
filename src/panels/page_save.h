#pragma once
#ifndef PAGE_SAVE_H
#define PAGE_SAVE_H

#include "panels/panel_page.h"

class TilingMonitor;

class page_save : public panel_page
{
    Q_OBJECT

public:
    page_save(ControlPanel * cpanel);
    virtual ~page_save();

    void onEnter() override;
    void onExit() override {}
    void onRefresh() override;

signals:

public slots:
    void slot_saveTiling();
    void slot_saveImage();
    void slot_saveMenu();
    void slot_saveSvg();

private slots:
    void slot_saveMosaic();
    void slot_designSourceChanged();
    void slot_tilingSourceChanged();
    void setup();

protected:
    void createMosaicSave();
    void createTilingSave();
    void savePixmap(QPixmap & pixmap, QString name);

private:
    QTextEdit   * designNotes;
    QLineEdit   * leSaveXmlName;

    QLineEdit   * tile_name;
    QTextEdit   * tile_desc;
    QLineEdit   * tile_author;
    QLabel      * requiresSave;

    TilingMonitor * tilingMonitor;
};

#endif
