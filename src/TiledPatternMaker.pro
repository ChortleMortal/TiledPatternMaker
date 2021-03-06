######################################################################
# Automatically generated by qmake (3.0) Fri Jun 17 14:09:09 2016
######################################################################
QT += core gui widgets svg
TEMPLATE = app
CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS

msvc:QMAKE_LFLAGS_WINDOWS += /ignore:4099
win32:RC_ICONS += dac1.ico

win32:QMAKE_LFLAGS += /STACK:32000000

# Input
SOURCES += \
    base/border.cpp \
    base/colorset.cpp \
    base/cycler.cpp \
    base/fileservices.cpp \
    base/geo_graphics.cpp \
    base/layer.cpp \
    base/main.cpp \
    base/misc.cpp \
    base/mosaic.cpp \
    base/mosaic_loader.cpp \
    base/mosaic_manager.cpp \
    base/mosaic_writer.cpp \
    base/pugixml.cpp \
    base/qtapplog.cpp \
    base/shortcuts.cpp \
    base/tiledpatternmaker.cpp \
    base/tpmsplash.cpp \
    base/transparentwidget.cpp \
    base/utilities.cpp \
    designs/design.cpp \
    designs/design_maker.cpp \
    designs/designs.cpp \
    designs/patterns.cpp \
    designs/shapefactory.cpp \
    designs/shapes.cpp \
    designs/tile.cpp \
    enums/ecyclemode.cpp \
    enums/edesign.cpp \
    enums/efigtype.cpp \
    enums/ekeyboardmode.cpp \
    enums/estatemachineevent.cpp \
    enums/etilingmakermousemode.cpp \
    enums/eviewtype.cpp \
    geometry/bounds.cpp \
    geometry/circle.cpp \
    geometry/colormaker.cpp \
    geometry/crop.cpp \
    geometry/dcel.cpp \
    geometry/edge.cpp \
    geometry/edgepoly.cpp \
    geometry/faces.cpp \
    geometry/fill_region.cpp \
    geometry/intersect.cpp \
    geometry/loose.cpp \
    geometry/map.cpp \
    geometry/map_cleanse.cpp \
    geometry/map_verify.cpp \
    geometry/neighbours.cpp \
    geometry/point.cpp \
    geometry/threads.cpp \
    geometry/transform.cpp \
    geometry/vertex.cpp \
    geometry/xform.cpp \
    makers/decoration_maker/decoration_maker.cpp \
    makers/decoration_maker/style_color_fill_group.cpp \
    makers/decoration_maker/style_color_fill_set.cpp \
    makers/decoration_maker/style_editors.cpp \
    makers/map_editor/map_editor.cpp \
    makers/map_editor/map_editor_selection.cpp \
    makers/map_editor/map_editor_stash.cpp \
    makers/map_editor/map_mouseactions.cpp \
    makers/map_editor/map_selection.cpp \
    makers/motif_maker/explicit_figure_editors.cpp \
    makers/motif_maker/feature_button.cpp \
    makers/motif_maker/feature_launcher.cpp \
    makers/motif_maker/figure_editors.cpp \
    makers/motif_maker/master_figure_editor.cpp \
    makers/motif_maker/motif_maker.cpp \
    makers/tiling_maker/feature_selection.cpp \
    makers/tiling_maker/tiling_maker.cpp \
    makers/tiling_maker/tiling_mouseactions.cpp \
    panels/dlg_colorSet.cpp \
    panels/dlg_crop.cpp \
    panels/dlg_edgepoly_edit.cpp \
    panels/dlg_line_edit.cpp \
    panels/dlg_listnameselect.cpp \
    panels/dlg_listselect.cpp \
    panels/dlg_magnitude.cpp \
    panels/dlg_name.cpp \
    panels/dlg_rebase.cpp \
    panels/dlg_rename.cpp \
    panels/dlg_textedit.cpp \
    panels/dlg_trim.cpp \
    panels/layout_qrectf.cpp \
    panels/layout_sliderset.cpp \
    panels/layout_transform.cpp \
    panels/motif_display_widget.cpp \
    panels/page_background_image.cpp \
    panels/page_borders.cpp \
    panels/page_config.cpp \
    panels/page_debug.cpp \
    panels/page_decoration_maker.cpp \
    panels/page_grid.cpp \
    panels/page_image_tools.cpp \
    panels/page_layers.cpp \
    panels/page_loaders.cpp \
    panels/page_log.cpp \
    panels/page_map_editor.cpp \
    panels/page_modelSettings.cpp \
    panels/page_motif_maker.cpp \
    panels/page_prototype_info.cpp \
    panels/page_save.cpp \
    panels/page_style_figure_info.cpp \
    panels/page_system_info.cpp \
    panels/page_tiling_maker.cpp \
    panels/panel.cpp \
    panels/panel_list_widget.cpp \
    panels/panel_misc.cpp \
    panels/panel_page.cpp \
    panels/panel_pagesWidget.cpp \
    panels/panel_status.cpp \
    panels/splitscreen.cpp \
    panels/versioned_list_widget.cpp \
    panels/view_panel.cpp \
    settings/configuration.cpp \
    settings/filldata.cpp \
    settings/frame_settings.cpp \
    settings/model_settings.cpp \
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
    tapp/design_element.cpp \
    tapp/explicit_figure.cpp \
    tapp/extended_rosette.cpp \
    tapp/extended_star.cpp \
    tapp/figure.cpp \
    tapp/figure_connector.cpp \
    tapp/infer.cpp \
    tapp/prototype.cpp \
    tapp/radial_figure.cpp \
    tapp/rosette.cpp \
    tapp/rosette_connect_figure.cpp \
    tapp/star.cpp \
    tapp/star_connect_figure.cpp \
    tile/backgroundimage.cpp \
    tile/feature.cpp \
    tile/feature_reader.cpp \
    tile/feature_writer.cpp \
    tile/placed_feature.cpp \
    tile/tiling.cpp \
    tile/tiling_loader.cpp \
    tile/tiling_manager.cpp \
    tile/tiling_writer.cpp \
    viewers/figure_view.cpp \
    viewers/grid.cpp \
    viewers/map_editor_view.cpp \
    viewers/prototype_view.cpp \
    viewers/shape_view.cpp \
    viewers/tiling_maker_view.cpp \
    viewers/tiling_view.cpp \
    viewers/view.cpp \
    viewers/viewcontrol.cpp \
    viewers/viewerbase.cpp

HEADERS += \
    base/border.h \
    base/colorset.h \
    base/cycler.h \
    base/fileservices.h \
    base/geo_graphics.h \
    base/layer.h \
    base/misc.h \
    base/mosaic.h \
    base/mosaic_loader.h \
    base/mosaic_manager.h \
    base/mosaic_writer.h \
    base/pugiconfig.hpp \
    base/pugixml.hpp \
    base/qtapplog.h \
    base/shared.h \
    base/shortcuts.h \
    base/tiledpatternmaker.h \
    base/tpmsplash.h \
    base/transparentwidget.h \
    base/utilities.h \
    base/version.h \
    designs/design.h \
    designs/design_maker.h \
    designs/designs.h \
    designs/patterns.h \
    designs/shapefactory.h \
    designs/shapes.h \
    designs/tile.h \
    enums/ecyclemode.h \
    enums/edesign.h \
    enums/efigtype.h \
    enums/ekeyboardmode.h \
    enums/emousemode.h \
    enums/estatemachineevent.h \
    enums/estyletype.h \
    enums/etilingmakermousemode.h \
    enums/eviewtype.h \
    geometry/circle.h \
    geometry/colormaker.h \
    geometry/crop.h \
    geometry/dcel.h \
    geometry/edge.h \
    geometry/edgepoly.h \
    geometry/faces.h \
    geometry/fill_region.h \
    geometry/intersect.h \
    geometry/loose.h \
    geometry/map.h \
    geometry/neighbours.h \
    geometry/point.h \
    geometry/threads.h \
    geometry/transform.h \
    geometry/vertex.h \
    geometry/xform.h \
    makers/decoration_maker/decoration_maker.h \
    makers/decoration_maker/style_color_fill_group.h \
    makers/decoration_maker/style_color_fill_set.h \
    makers/decoration_maker/style_editors.h \
    makers/map_editor/map_editor.h \
    makers/map_editor/map_editor_selection.h \
    makers/map_editor/map_editor_stash.h \
    makers/map_editor/map_mouseactions.h \
    makers/map_editor/map_selection.h \
    makers/motif_maker/explicit_figure_editors.h \
    makers/motif_maker/feature_button.h \
    makers/motif_maker/feature_launcher.h \
    makers/motif_maker/figure_editors.h \
    makers/motif_maker/master_figure_editor.h \
    makers/motif_maker/motif_maker.h \
    makers/tiling_maker/feature_selection.h \
    makers/tiling_maker/tiling_maker.h \
    makers/tiling_maker/tiling_mouseactions.h \
    panels/dlg_colorSet.h \
    panels/dlg_crop.h \
    panels/dlg_edgepoly_edit.h \
    panels/dlg_line_edit.h \
    panels/dlg_listnameselect.h \
    panels/dlg_listselect.h \
    panels/dlg_magnitude.h \
    panels/dlg_name.h \
    panels/dlg_rebase.h \
    panels/dlg_rename.h \
    panels/dlg_textedit.h \
    panels/dlg_trim.h \
    panels/layout_qrectf.h \
    panels/layout_sliderset.h \
    panels/layout_transform.h \
    panels/motif_display_widget.h \
    panels/page_background_image.h \
    panels/page_borders.h \
    panels/page_config.h \
    panels/page_debug.h \
    panels/page_decoration_maker.h \
    panels/page_grid.h \
    panels/page_image_tools.h \
    panels/page_layers.h \
    panels/page_loaders.h \
    panels/page_log.h \
    panels/page_map_editor.h \
    panels/page_modelSettings.h \
    panels/page_motif_maker.h \
    panels/page_prototype_info.h \
    panels/page_save.h \
    panels/page_style_figure_info.h \
    panels/page_system_info.h \
    panels/page_tiling_maker.h \
    panels/panel.h \
    panels/panel_list_widget.h \
    panels/panel_misc.h \
    panels/panel_page.h \
    panels/panel_pagesWidget.h \
    panels/panel_status.h \
    panels/splitscreen.h \
    panels/versioned_list_widget.h \
    panels/view_panel.h \
    settings/configuration.h \
    settings/filldata.h \
    settings/frame_settings.h \
    settings/model_settings.h \
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
    tapp/design_element.h \
    tapp/explicit_figure.h \
    tapp/extended_rosette.h \
    tapp/extended_star.h \
    tapp/figure.h \
    tapp/figure_connector.h \
    tapp/infer.h \
    tapp/prototype.h \
    tapp/radial_figure.h \
    tapp/rosette.h \
    tapp/rosette_connect_figure.h \
    tapp/star.h \
    tapp/star_connect_figure.h \
    tile/backgroundimage.h \
    tile/feature.h \
    tile/feature_reader.h \
    tile/feature_writer.h \
    tile/placed_feature.h \
    tile/tiling.h \
    tile/tiling_loader.h \
    tile/tiling_manager.h \
    tile/tiling_writer.h \
    viewers/figure_view.h \
    viewers/grid.h \
    viewers/map_editor_view.h \
    viewers/prototype_view.h \
    viewers/shape_view.h \
    viewers/tiling_maker_view.h \
    viewers/tiling_view.h \
    viewers/view.h \
    viewers/viewcontrol.h \
    viewers/viewerbase.h

FORMS +=

DISTFILES += \
    ../README.md \
    ../etc/TIledPatternMaker.nsi \
    ../etc/build-install.md \
    ../etc/docs/BrougImplemented.md \
    ../etc/docs/ViewersAndMakers.xlsx \
    ../etc/docs/bugs.md \
    ../etc/docs/design_notes.md \
    ../etc/docs/makers.md \
    ../etc/docs/mod_dates.txt \
    ../etc/docs/sample.md \
    ../etc/docs/taprats_notes.md \
    ../etc/docs/taprats_readme.txt \
    ../etc/fixdropbox.py \
    ../etc/models/Components.qmodel \
    ../etc/models/Designs.qmodel \
    ../etc/models/Style.qmodel \
    ../etc/models/Viewers.qmodel \
    ../etc/models/designeditor.qmodel \
    ../etc/models/figure_editors.qmodel \
    ../etc/models/figures.qmodel \
    ../etc/models/makers.qmodel \
    ../etc/models/makers2.qmodel \
    ../etc/models/shapes.qmodel \
    ../etc/models/styles.qmodel \
    ../etc/prune.sh \
    ../etc/qt5.bat

RESOURCES += \
    tpm.qrc
