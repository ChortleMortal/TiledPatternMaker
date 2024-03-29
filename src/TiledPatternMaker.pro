QT += core gui widgets svg concurrent
TEMPLATE = app
CONFIG += c++20

DEFINES += QT_DEPRECATED_WARNINGS

msvc:QMAKE_LFLAGS_WINDOWS += /ignore:4099
win32:RC_ICONS += dac1.ico

win32:QMAKE_LFLAGS += /STACK:32000000

# Input
SOURCES += \
    engine/compare_bmp_engine.cpp \
    engine/image_engine.cpp \
    engine/mosaic_bmp_engine.cpp \
    engine/mosaic_stepper.cpp \
    engine/png_stepper.cpp \
    engine/stepping_engine.cpp \
    engine/tiling_bmp_engine.cpp \
    engine/tiling_stepper.cpp \
    engine/version_stepper.cpp \
    engine/worklist_stepper.cpp \
    enums/eborder.cpp \
    enums/ecyclemode.cpp \
    enums/edesign.cpp \
    enums/edgetype.cpp \
    enums/ekeyboardmode.cpp \
    enums/emapeditor.cpp \
    enums/emotiftype.cpp \
    enums/estatemachineevent.cpp \
    enums/etilingmakermousemode.cpp \
    enums/eviewtype.cpp \
    geometry/arcdata.cpp \
    geometry/belowandaboveedge.cpp \
    geometry/bounds.cpp \
    geometry/circle.cpp \
    geometry/colormaker.cpp \
    geometry/crop.cpp \
    geometry/dcel.cpp \
    geometry/edge.cpp \
    geometry/edgepoly.cpp \
    geometry/faces.cpp \
    geometry/fill_region.cpp \
    geometry/geo.cpp \
    geometry/intersect.cpp \
    geometry/loose.cpp \
    geometry/map.cpp \
    geometry/map_cleanse.cpp \
    geometry/map_verify.cpp \
    geometry/measurement.cpp \
    geometry/neighbour_map.cpp \
    geometry/neighbours.cpp \
    geometry/threads.cpp \
    geometry/transform.cpp \
    geometry/vertex.cpp \
    geometry/xform.cpp \
    legacy/design.cpp \
    legacy/design_maker.cpp \
    legacy/designs.cpp \
    legacy/legacy_border.cpp \
    legacy/legacy_tile.cpp \
    legacy/patterns.cpp \
    legacy/shapefactory.cpp \
    legacy/shapes.cpp \
    main.cpp \
    makers/crop_maker/crop_maker.cpp \
    makers/crop_maker/mouse_edit_border.cpp \
    makers/crop_maker/mouse_edit_crop.cpp \
    makers/map_editor/map_editor.cpp \
    makers/map_editor/map_editor_db.cpp \
    makers/map_editor/map_editor_map_loader.cpp \
    makers/map_editor/map_editor_map_writer.cpp \
    makers/map_editor/map_editor_mouseactions.cpp \
    makers/map_editor/map_editor_selection.cpp \
    makers/map_editor/map_editor_stash.cpp \
    makers/map_editor/map_selection.cpp \
    makers/mosaic_maker/mosaic_maker.cpp \
    makers/mosaic_maker/style_color_fill_group.cpp \
    makers/mosaic_maker/style_color_fill_set.cpp \
    makers/mosaic_maker/style_editors.cpp \
    makers/motif_maker/design_element_button.cpp \
    makers/motif_maker/design_element_selector.cpp \
    makers/motif_maker/irregular_motif_editors.cpp \
    makers/motif_maker/motif_editor_widget.cpp \
    makers/motif_maker/motif_maker_widget.cpp \
    makers/motif_maker/motif_maker_widgets.cpp \
    makers/motif_maker/regular_motif_editors.cpp \
    makers/prototype_maker/prototype.cpp \
    makers/prototype_maker/prototype_data.cpp \
    makers/prototype_maker/prototype_maker.cpp \
    makers/tiling_maker/tile_selection.cpp \
    makers/tiling_maker/tiling_maker.cpp \
    makers/tiling_maker/tiling_monitor.cpp \
    makers/tiling_maker/tiling_mouseactions.cpp \
    misc/border.cpp \
    misc/colorset.cpp \
    misc/fileservices.cpp \
    misc/geo_graphics.cpp \
    misc/layer.cpp \
    misc/layer_controller.cpp \
    misc/mark_x.cpp \
    misc/pugixml.cpp \
    misc/qtapplog.cpp \
    misc/shortcuts.cpp \
    misc/sys.cpp \
    misc/timers.cpp \
    misc/tpmsplash.cpp \
    misc/utilities.cpp \
    mosaic/design_element.cpp \
    mosaic/legacy_loader.cpp \
    mosaic/mosaic.cpp \
    mosaic/mosaic_manager.cpp \
    mosaic/mosaic_reader.cpp \
    mosaic/mosaic_reader_base.cpp \
    mosaic/mosaic_writer.cpp \
    mosaic/mosaic_writer_base.cpp \
    motifs/explicit_map_motif.cpp \
    motifs/extended_boundary.cpp \
    motifs/extended_rosette.cpp \
    motifs/extended_star.cpp \
    motifs/extender.cpp \
    motifs/girih_motif.cpp \
    motifs/hourglass_motif.cpp \
    motifs/inferred_motif.cpp \
    motifs/intersect_motif.cpp \
    motifs/irregular_girih_branches.cpp \
    motifs/irregular_motif.cpp \
    motifs/irregular_rosette.cpp \
    motifs/irregular_star.cpp \
    motifs/irregular_star_branches.cpp \
    motifs/irregular_tools.cpp \
    motifs/motif.cpp \
    motifs/motif_connector.cpp \
    motifs/radial_motif.cpp \
    motifs/rosette.cpp \
    motifs/rosette2.cpp \
    motifs/rosette_connect.cpp \
    motifs/star.cpp \
    motifs/star2.cpp \
    motifs/star_connect.cpp \
    motifs/tile_motif.cpp \
    panels/controlpanel.cpp \
    panels/page_backgrounds.cpp \
    panels/page_borders.cpp \
    panels/page_config.cpp \
    panels/page_crop_maker.cpp \
    panels/page_debug.cpp \
    panels/page_grid.cpp \
    panels/page_image_tools.cpp \
    panels/page_layers.cpp \
    panels/page_loaders.cpp \
    panels/page_log.cpp \
    panels/page_map_editor.cpp \
    panels/page_modelSettings.cpp \
    panels/page_mosaic_info.cpp \
    panels/page_mosaic_maker.cpp \
    panels/page_motif_maker.cpp \
    panels/page_prototype_info.cpp \
    panels/page_save.cpp \
    panels/page_system_info.cpp \
    panels/page_tiling_maker.cpp \
    panels/panel_misc.cpp \
    panels/panel_page.cpp \
    panels/panel_page_controller.cpp \
    panels/panel_page_list_widget.cpp \
    panels/panel_pages_widget.cpp \
    panels/panel_status.cpp \
    panels/panel_view_select.cpp \
    panels/splitscreen.cpp \
    settings/canvas.cpp \
    settings/canvas_settings.cpp \
    settings/configuration.cpp \
    settings/filldata.cpp \
    style/colored.cpp \
    style/emboss.cpp \
    style/filled.cpp \
    style/interlace.cpp \
    style/outline.cpp \
    style/plain.cpp \
    style/sketch.cpp \
    style/style.cpp \
    style/thick.cpp \
    style/tile_colors.cpp \
    tile/backgroundimage.cpp \
    tile/placed_tile.cpp \
    tile/tile.cpp \
    tile/tile_reader.cpp \
    tile/tile_writer.cpp \
    tile/tiling.cpp \
    tile/tiling_data.cpp \
    tile/tiling_manager.cpp \
    tile/tiling_reader.cpp \
    tile/tiling_writer.cpp \
    tiledpatternmaker.cpp \
    viewers/backgroundimageview.cpp \
    viewers/border_view.cpp \
    viewers/crop_view.cpp \
    viewers/grid_view.cpp \
    viewers/map_editor_view.cpp \
    viewers/measure_view.cpp \
    viewers/motif_view.cpp \
    viewers/prototype_view.cpp \
    viewers/shape_view.cpp \
    viewers/tiling_maker_view.cpp \
    viewers/view.cpp \
    viewers/view_controller.cpp \
    viewers/viewerbase.cpp \
    widgets/crop_widget.cpp \
    widgets/dlg_cleanse.cpp \
    widgets/dlg_colorSet.cpp \
    widgets/dlg_edgepoly_edit.cpp \
    widgets/dlg_line_edit.cpp \
    widgets/dlg_listnameselect.cpp \
    widgets/dlg_listselect.cpp \
    widgets/dlg_magnitude.cpp \
    widgets/dlg_name.cpp \
    widgets/dlg_push_select.cpp \
    widgets/dlg_rebase.cpp \
    widgets/dlg_rename.cpp \
    widgets/dlg_textedit.cpp \
    widgets/dlg_trim.cpp \
    widgets/dlg_wlist_create.cpp \
    widgets/image_layer.cpp \
    widgets/image_widget.cpp \
    widgets/layout_qrectf.cpp \
    widgets/layout_sliderset.cpp \
    widgets/layout_transform.cpp \
    widgets/memory_combo.cpp \
    widgets/mouse_mode_widget.cpp \
    widgets/rounded_polygon.cpp \
    widgets/transparent_widget.cpp \
    widgets/version_dialog.cpp \
    widgets/versioned_list_widget.cpp \
    widgets/worklist_widget.cpp

HEADERS += \
    engine/compare_bmp_engine.h \
    engine/image_engine.h \
    engine/mosaic_bmp_engine.h \
    engine/mosaic_stepper.h \
    engine/png_stepper.h \
    engine/stepping_engine.h \
    engine/tiling_bmp_engine.h \
    engine/tiling_stepper.h \
    engine/version_stepper.h \
    engine/worklist_stepper.h \
    enums/eborder.h \
    enums/ecyclemode.h \
    enums/edesign.h \
    enums/edgetype.h \
    enums/efilesystem.h \
    enums/ekeyboardmode.h \
    enums/elogmode.h \
    enums/emapeditor.h \
    enums/emotiftype.h \
    enums/emousemode.h \
    enums/epanelpage.h \
    enums/estatemachineevent.h \
    enums/estyletype.h \
    enums/etilingmakermousemode.h \
    enums/eviewtype.h \
    geometry/arcdata.h \
    geometry/belowandaboveedge.h \
    geometry/circle.h \
    geometry/colormaker.h \
    geometry/crop.h \
    geometry/dcel.h \
    geometry/edge.h \
    geometry/edgepoly.h \
    geometry/faces.h \
    geometry/geo.h \
    geometry/fill_region.h \
    geometry/intersect.h \
    geometry/loose.h \
    geometry/map.h \
    geometry/measurement.h \
    geometry/neighbour_map.h \
    geometry/neighbours.h \
    geometry/threads.h \
    geometry/transform.h \
    geometry/vertex.h \
    geometry/xform.h \
    legacy/design.h \
    legacy/design_maker.h \
    legacy/designs.h \
    legacy/legacy_border.h \
    legacy/legacy_tile.h \
    legacy/patterns.h \
    legacy/shapefactory.h \
    legacy/shapes.h \
    makers/crop_maker/crop_maker.h \
    makers/crop_maker/mouse_edit_border.h \
    makers/crop_maker/mouse_edit_crop.h \
    makers/map_editor/map_editor.h \
    makers/map_editor/map_editor_db.h \
    makers/map_editor/map_editor_map_loader.h \
    makers/map_editor/map_editor_map_writer.h \
    makers/map_editor/map_editor_mouseactions.h \
    makers/map_editor/map_editor_selection.h \
    makers/map_editor/map_editor_stash.h \
    makers/map_editor/map_selection.h \
    makers/mosaic_maker/mosaic_maker.h \
    makers/mosaic_maker/style_color_fill_group.h \
    makers/mosaic_maker/style_color_fill_set.h \
    makers/mosaic_maker/style_editors.h \
    makers/motif_maker/design_element_button.h \
    makers/motif_maker/design_element_selector.h \
    makers/motif_maker/irregular_motif_editors.h \
    makers/motif_maker/motif_editor_widget.h \
    makers/motif_maker/motif_maker_widget.h \
    makers/motif_maker/motif_maker_widgets.h \
    makers/motif_maker/regular_motif_editors.h \
    makers/prototype_maker/prototype.h \
    makers/prototype_maker/prototype_data.h \
    makers/prototype_maker/prototype_maker.h \
    makers/tiling_maker/tile_selection.h \
    makers/tiling_maker/tiling_maker.h \
    makers/tiling_maker/tiling_monitor.h \
    makers/tiling_maker/tiling_mouseactions.h \
    misc/border.h \
    misc/colorset.h \
    misc/fileservices.h \
    misc/geo_graphics.h \
    misc/layer.h \
    misc/layer_controller.h \
    misc/mark_x.h \
    misc/pugiconfig.hpp \
    misc/pugixml.hpp \
    misc/qtapplog.h \
    misc/shortcuts.h \
    misc/signal_blocker.h \
    misc/sys.h \
    misc/tile_color_defs.h \
    misc/timers.h \
    misc/tpm_io.h \
    misc/tpmsplash.h \
    misc/unique_qvector.h \
    misc/utilities.h \
    misc/version.h \
    mosaic/design_element.h \
    mosaic/legacy_loader.h \
    mosaic/mosaic.h \
    mosaic/mosaic_manager.h \
    mosaic/mosaic_reader.h \
    mosaic/mosaic_reader_base.h \
    mosaic/mosaic_writer.h \
    mosaic/mosaic_writer_base.h \
    motifs/explicit_map_motif.h \
    motifs/extended_boundary.h \
    motifs/extended_rosette.h \
    motifs/extended_star.h \
    motifs/extender.h \
    motifs/girih_motif.h \
    motifs/hourglass_motif.h \
    motifs/inferred_motif.h \
    motifs/intersect_motif.h \
    motifs/irregular_girih_branches.h \
    motifs/irregular_motif.h \
    motifs/irregular_rosette.h \
    motifs/irregular_star.h \
    motifs/irregular_star_branches.h \
    motifs/irregular_tools.h \
    motifs/motif.h \
    motifs/motif_connector.h \
    motifs/radial_motif.h \
    motifs/rosette.h \
    motifs/rosette2.h \
    motifs/rosette_connect.h \
    motifs/star.h \
    motifs/star2.h \
    motifs/star_connect.h \
    motifs/tile_motif.h \
    panels/controlpanel.h \
    panels/page_backgrounds.h \
    panels/page_borders.h \
    panels/page_config.h \
    panels/page_crop_maker.h \
    panels/page_debug.h \
    panels/page_grid.h \
    panels/page_image_tools.h \
    panels/page_layers.h \
    panels/page_loaders.h \
    panels/page_log.h \
    panels/page_map_editor.h \
    panels/page_modelSettings.h \
    panels/page_mosaic_info.h \
    panels/page_mosaic_maker.h \
    panels/page_motif_maker.h \
    panels/page_prototype_info.h \
    panels/page_save.h \
    panels/page_system_info.h \
    panels/page_tiling_maker.h \
    panels/panel_misc.h \
    panels/panel_page.h \
    panels/panel_page_controller.h \
    panels/panel_page_list_widget.h \
    panels/panel_pages_widget.h \
    panels/panel_status.h \
    panels/panel_view_select.h \
    panels/splitscreen.h \
    settings/canvas.h \
    settings/canvas_settings.h \
    settings/configuration.h \
    settings/filldata.h \
    settings/tristate.h \
    style/colored.h \
    style/emboss.h \
    style/filled.h \
    style/interlace.h \
    style/outline.h \
    style/plain.h \
    style/sketch.h \
    style/style.h \
    style/thick.h \
    style/tile_colors.h \
    tile/backgroundimage.h \
    tile/placed_tile.h \
    tile/tile.h \
    tile/tile_reader.h \
    tile/tile_writer.h \
    tile/tiling.h \
    tile/tiling_data.h \
    tile/tiling_manager.h \
    tile/tiling_reader.h \
    tile/tiling_writer.h \
    tiledpatternmaker.h \
    viewers/backgroundimageview.h \
    viewers/border_view.h \
    viewers/crop_view.h \
    viewers/grid_view.h \
    viewers/map_editor_view.h \
    viewers/measure_view.h \
    viewers/motif_view.h \
    viewers/prototype_view.h \
    viewers/shape_view.h \
    viewers/tiling_maker_view.h \
    viewers/view.h \
    viewers/view_controller.h \
    viewers/viewerbase.h \
    widgets/crop_widget.h \
    widgets/dlg_cleanse.h \
    widgets/dlg_colorSet.h \
    widgets/dlg_edgepoly_edit.h \
    widgets/dlg_line_edit.h \
    widgets/dlg_listnameselect.h \
    widgets/dlg_listselect.h \
    widgets/dlg_magnitude.h \
    widgets/dlg_name.h \
    widgets/dlg_push_select.h \
    widgets/dlg_rebase.h \
    widgets/dlg_rename.h \
    widgets/dlg_textedit.h \
    widgets/dlg_trim.h \
    widgets/dlg_wlist_create.h \
    widgets/image_layer.h \
    widgets/image_widget.h \
    widgets/layout_qrectf.h \
    widgets/layout_sliderset.h \
    widgets/layout_transform.h \
    widgets/memory_combo.h \
    widgets/mouse_mode_widget.h \
    widgets/rounded_polygon.h \
    widgets/transparent_widget.h \
    widgets/version_dialog.h \
    widgets/versioned_list_widget.h \
    widgets/worklist_widget.h

FORMS +=

DISTFILES += \
    ../etc/windows/aliases.txt \
    ../etc/windows/binfo \
    ../etc/windows/branches \
    ../etc/windows/bstate \
    ../etc/windows/build-install.md \
    ../etc/windows/build-install-qt6.md \
    ../etc/windows/configure-local.bat \
    ../etc/windows/qt5.bat \
    ../etc/windows/qt6.bat \
    ../etc/linux/aliases.txt \
    ../etc/linux/binfo \
    ../etc/linux/branches \
    ../etc/linux/bstate \
    ../etc/linux/build-install.md \
    ../etc/linux/build-install-qt6.md \
    ../etc/linux/configure-local \
    ../etc/TiledPatternMaker.nsi \
    ../etc/models/Components.qmodel \
    ../etc/models/designeditor.qmodel \
    ../etc/models/Designs.qmodel \
    ../etc/models/figure_editors.qmodel \
    ../etc/models/figures.qmodel \
    ../etc/models/makers.qmodel \
    ../etc/models/makers2.qmodel \
    ../etc/models/mosaic.qmodel \
    ../etc/models/shapes.qmodel \
    ../etc/models/styles.qmodel \
    ../etc/models/Viewers.qmodel \
    ../release-notes.txt

RESOURCES += tpm.qrc
