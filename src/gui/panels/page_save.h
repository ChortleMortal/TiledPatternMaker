#pragma once
#ifndef PAGE_SAVE_H
#define PAGE_SAVE_H

#include "gui/panels/panel_page.h"

typedef std::shared_ptr<class Tiling> TilingPtr;

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
    void slot_print();

private slots:
    void slot_saveMosaic();
    void slot_tilingChanged(int idx);
    void setup();

protected:
    void createMosaicSave();
    void createTilingSave();
    void savePixmap(QPixmap & pixmap, QString name);
    void selectTiling(TilingPtr tiling);

private:
    QTextEdit   * designNotes;
    QLineEdit   * leSaveXmlName;

    QComboBox   * tile_names;
    QTextEdit   * tile_desc;
    QLineEdit   * tile_author;
    QLabel      * requiresSave;
};
#endif
