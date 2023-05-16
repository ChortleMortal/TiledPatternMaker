#pragma once
#ifndef PAGE_SAVE_H
#define PAGE_SAVE_H

class QTextEdit;
class QLineEdit;
class QLabel;
class QCheckBox;

#include "widgets/panel_page.h"

class page_save : public panel_page
{
    Q_OBJECT

public:
    page_save(ControlPanel * cpanel);

    void onEnter() override;
    void onExit() override {}
    void onRefresh() override;

signals:
    void sig_saveMosaic(QString name);
    void sig_saveTiling(QString name);

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
};

#endif
