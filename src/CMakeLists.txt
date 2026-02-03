cmake_minimum_required(VERSION 3.21)

project(TiledPatternMaker LANGUAGES CXX)

# Enable Qt features
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Unity builds globally
set(CMAKE_UNITY_BUILD ON)

# Try to find ccache (with fallback path hint)
find_program(CCACHE_PROGRAM ccache  PATHS "C:/ProgramData/chocolatey/bin")

if(CCACHE_PROGRAM)
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
else()
    message(WARNING "ccache not found, continuing without compiler cache.")
endif()

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg Concurrent PrintSupport)

# Project sources
set(PROJECT_SOURCES
    gui/map_editor/map_editor.cpp
    gui/map_editor/map_editor.h
    gui/map_editor/map_editor_db.cpp
    gui/map_editor/map_editor_db.h
    gui/map_editor/map_editor_map_loader.cpp
    gui/map_editor/map_editor_map_loader.h
    gui/map_editor/map_editor_map_writer.cpp
    gui/map_editor/map_editor_map_writer.h
    gui/map_editor/map_editor_mouseactions.cpp
    gui/map_editor/map_editor_mouseactions.h
    gui/map_editor/map_editor_selection.cpp
    gui/map_editor/map_editor_selection.h
    gui/map_editor/map_editor_stash.cpp
    gui/map_editor/map_editor_stash.h
    gui/map_editor/map_selection.cpp
    gui/map_editor/map_selection.h
    gui/model_editors/border_edit/mouse_edit_border.cpp
    gui/model_editors/border_edit/mouse_edit_border.h
    gui/model_editors/crop_edit/mouse_edit_crop.cpp
    gui/model_editors/crop_edit/mouse_edit_crop.h
    gui/model_editors/motif_edit/design_element_button.cpp
    gui/model_editors/motif_edit/design_element_button.h
    gui/model_editors/motif_edit/design_element_selector.cpp
    gui/model_editors/motif_edit/design_element_selector.h
    gui/model_editors/motif_edit/irregular_motif_editors.cpp
    gui/model_editors/motif_edit/irregular_motif_editors.h
    gui/model_editors/motif_edit/motif_editor_widget.cpp
    gui/model_editors/motif_edit/motif_editor_widget.h
    gui/model_editors/motif_edit/motif_editors.cpp
    gui/model_editors/motif_edit/motif_editors.h
    gui/model_editors/motif_edit/motif_maker_widget.cpp
    gui/model_editors/motif_edit/motif_maker_widget.h
    gui/model_editors/motif_edit/regular_motif_editors.cpp
    gui/model_editors/motif_edit/regular_motif_editors.h
    gui/model_editors/style_edit/style_color_fill_face.cpp
    gui/model_editors/style_edit/style_color_fill_face.h
    gui/model_editors/style_edit/style_color_fill_group.cpp
    gui/model_editors/style_edit/style_color_fill_group.h
    gui/model_editors/style_edit/style_color_fill_original.cpp
    gui/model_editors/style_edit/style_color_fill_original.h
    gui/model_editors/style_edit/style_color_fill_set.cpp
    gui/model_editors/style_edit/style_color_fill_set.h
    gui/model_editors/style_edit/style_editors.cpp
    gui/model_editors/style_edit/style_editors.h
    gui/model_editors/tiling_edit/tile_selection.cpp
    gui/model_editors/tiling_edit/tile_selection.h
    gui/model_editors/tiling_edit/tiling_mouseactions.cpp
    gui/model_editors/tiling_edit/tiling_mouseactions.h
    gui/panels/page_backgrounds.cpp
    gui/panels/page_backgrounds.h
    gui/panels/page_borders.cpp
    gui/panels/page_borders.h
    gui/panels/page_config.cpp
    gui/panels/page_config.h
    gui/panels/page_crop_maker.cpp
    gui/panels/page_crop_maker.h
    gui/panels/page_debug.cpp
    gui/panels/page_debug.h
    gui/panels/page_grid.cpp
    gui/panels/page_grid.h
    gui/panels/page_image_tools.cpp
    gui/panels/page_image_tools.h
    gui/panels/page_layers.cpp
    gui/panels/page_layers.h
    gui/panels/page_layer_alignmentl.cpp
    gui/panels/page_layer_alignmentl.h
    gui/panels/page_loaders.cpp
    gui/panels/page_loaders.h
    gui/panels/panel_worker.cpp
    gui/panels/panel_worker.h
    gui/panels/page_log.cpp
    gui/panels/page_log.h
    gui/panels/page_map_editor.cpp
    gui/panels/page_map_editor.h
    gui/panels/page_modelSettings.cpp
    gui/panels/page_modelSettings.h
    gui/panels/page_mosaic_info.cpp
    gui/panels/page_mosaic_info.h
    gui/panels/page_mosaic_maker.cpp
    gui/panels/page_mosaic_maker.h
    gui/panels/page_motif_maker.cpp
    gui/panels/page_motif_maker.h
    gui/panels/page_prototype_info.cpp
    gui/panels/page_prototype_info.h
    gui/panels/page_save.cpp
    gui/panels/page_save.h
    gui/panels/page_system_info.cpp
    gui/panels/page_system_info.h
    gui/panels/page_tiling_maker.cpp
    gui/panels/page_tiling_maker.h
    gui/panels/panel_misc.cpp
    gui/panels/panel_misc.h
    gui/panels/panel_page.cpp
    gui/panels/panel_page.h
    gui/panels/panel_page_controller.cpp
    gui/panels/panel_page_controller.h
    gui/panels/panel_page_list_widget.cpp
    gui/panels/panel_page_list_widget.h
    gui/panels/panel_pages_widget.cpp
    gui/panels/panel_pages_widget.h
    gui/panels/panel_status.cpp
    gui/panels/panel_status.h
    gui/panels/panel_view_select.cpp
    gui/panels/panel_view_select.h
    gui/panels/shortcuts.cpp
    gui/panels/shortcuts.h
    gui/top/controlpanel.cpp
    gui/top/controlpanel.h
    gui/top/splash_screen.cpp
    gui/top/splash_screen.h
    gui/top/split_screen.cpp
    gui/top/split_screen.h
    gui/top/system_view.cpp
    gui/top/system_view.h
    gui/top/system_view_accessor.cpp
    gui/top/system_view_accessor.h
    gui/top/system_view_controller.cpp
    gui/top/system_view_controller.h
    gui/viewers/crop_maker_view.cpp
    gui/viewers/crop_maker_view.h
    gui/viewers/debug_view.cpp
    gui/viewers/debug_view.h
    gui/viewers/geo_graphics.cpp
    gui/viewers/geo_graphics.h
    gui/viewers/grid_view.cpp
    gui/viewers/grid_view.h

    gui/viewers/image_view.cpp
    gui/viewers/image_view.cpp
    gui/viewers/image_view.h
    gui/viewers/image_view.h
    gui/viewers/layer.cpp
    gui/viewers/layer.h
    gui/viewers/layer_controller.cpp
    gui/viewers/layer_controller.h
    gui/viewers/map_editor_view.cpp
    gui/viewers/map_editor_view.h
    gui/viewers/motif_maker_view.cpp
    gui/viewers/motif_maker_view.h
    gui/viewers/prototype_view.cpp
    gui/viewers/prototype_view.h
    gui/viewers/shape_view.cpp
    gui/viewers/shape_view.h
    gui/viewers/tiling_maker_view.cpp
    gui/viewers/tiling_maker_view.h
    gui/viewers/viewer_services.cpp
    gui/viewers/viewer_services.h
    gui/widgets/crop_widget.cpp
    gui/widgets/crop_widget.h
    gui/widgets/dlg_cleanse.cpp
    gui/widgets/dlg_cleanse.h
    gui/widgets/dlg_colorSet.cpp
    gui/widgets/dlg_colorSet.h
    gui/widgets/dlg_edgepoly_edit.cpp
    gui/widgets/dlg_edgepoly_edit.h
    gui/widgets/dlg_listnameselect.cpp
    gui/widgets/dlg_listnameselect.h
    gui/widgets/dlg_listselect.cpp
    gui/widgets/dlg_listselect.h
    gui/widgets/dlg_magnitude.cpp
    gui/widgets/dlg_magnitude.h
    gui/widgets/dlg_name.cpp
    gui/widgets/dlg_name.h
    gui/widgets/dlg_point_edit.cpp
    gui/widgets/dlg_point_edit.h
    gui/widgets/dlg_print.cpp
    gui/widgets/dlg_print.h
    gui/widgets/dlg_push_select.cpp
    gui/widgets/dlg_push_select.h
    gui/widgets/dlg_rebase.cpp
    gui/widgets/dlg_rebase.h
    gui/widgets/dlg_rename.cpp
    gui/widgets/dlg_rename.h
    gui/widgets/dlg_savename.cpp
    gui/widgets/dlg_savename.h
    gui/widgets/dlg_textedit.cpp
    gui/widgets/dlg_textedit.h
    gui/widgets/dlg_trim.cpp
    gui/widgets/dlg_trim.h
    gui/widgets/dlg_wlist_create.cpp
    gui/widgets/dlg_wlist_create.h
    gui/widgets/flash_msgbox.h
    gui/widgets/flash_msgbox.cpp
    gui/widgets/floatable_tab.cpp
    gui/widgets/floatable_tab.h
    gui/widgets/image_widget.cpp
    gui/widgets/image_widget.h
    gui/widgets/layout_qrectf.cpp
    gui/widgets/layout_qrectf.h
    gui/widgets/layout_sliderset.cpp
    gui/widgets/layout_sliderset.h
    gui/widgets/layout_transform.cpp
    gui/widgets/layout_transform.h
    gui/widgets/memory_combo.cpp
    gui/widgets/memory_combo.h
    gui/widgets/mouse_mode_widget.cpp
    gui/widgets/mouse_mode_widget.h
    gui/widgets/rounded_polygon.cpp
    gui/widgets/rounded_polygon.h
    gui/widgets/smx_widget.cpp
    gui/widgets/smx_widget.h
    gui/widgets/transparent_widget.cpp
    gui/widgets/transparent_widget.h
    gui/widgets/version_dialog.cpp
    gui/widgets/version_dialog.h
    gui/widgets/versioned_list_widget.cpp
    gui/widgets/versioned_list_widget.h
    gui/widgets/worklist_widget.cpp
    gui/widgets/worklist_widget.h

    legacy/design.cpp
    legacy/design.h
    legacy/design_maker.cpp
    legacy/design_maker.h
    legacy/designs.cpp
    legacy/designs.h
    legacy/legacy_border.cpp
    legacy/legacy_border.h
    legacy/legacy_tile.cpp
    legacy/legacy_tile.h
    legacy/patterns.cpp
    legacy/patterns.h
    legacy/shapefactory.cpp
    legacy/shapefactory.h
    legacy/shapes.cpp
    legacy/shapes.h

    model/makers/crop_maker.cpp
    model/makers/crop_maker.h
    model/makers/mosaic_maker.cpp
    model/makers/mosaic_maker.h
    model/makers/prototype_maker.cpp
    model/makers/prototype_maker.h
    model/makers/prototype_maker_data.cpp
    model/makers/prototype_maker_data.h
    model/makers/tiling_maker.cpp
    model/makers/tiling_maker.h
    model/mosaics/border.cpp
    model/mosaics/border.h
    model/mosaics/legacy_loader.cpp
    model/mosaics/legacy_loader.h
    model/mosaics/mosaic.cpp
    model/mosaics/mosaic.h
    model/mosaics/mosaic_manager.cpp
    model/mosaics/mosaic_manager.h
    model/mosaics/mosaic_reader.cpp
    model/mosaics/mosaic_reader.h
    model/mosaics/mosaic_writer.cpp
    model/mosaics/mosaic_writer.h
    model/mosaics/reader_base.cpp
    model/mosaics/reader_base.h
    model/mosaics/writer_base.cpp
    model/mosaics/writer_base.h
    model/motifs/explicit_map_motif.cpp
    model/motifs/explicit_map_motif.h
    model/motifs/extended_boundary.cpp
    model/motifs/extended_boundary.h
    model/motifs/extender.cpp
    model/motifs/extender.h
    model/motifs/girih_motif.cpp
    model/motifs/girih_motif.h
    model/motifs/hourglass_motif.cpp
    model/motifs/hourglass_motif.h
    model/motifs/inferred_motif.cpp
    model/motifs/inferred_motif.h
    model/motifs/intersect_motif.cpp
    model/motifs/intersect_motif.h
    model/motifs/irregular_girih_branches.cpp
    model/motifs/irregular_girih_branches.h
    model/motifs/irregular_motif.cpp
    model/motifs/irregular_motif.h
    model/motifs/irregular_rosette.cpp
    model/motifs/irregular_rosette.h
    model/motifs/irregular_star.cpp
    model/motifs/irregular_star.h
    model/motifs/irregular_star_branches.cpp
    model/motifs/irregular_star_branches.h
    model/motifs/irregular_tools.cpp
    model/motifs/irregular_tools.h
    model/motifs/motif.cpp
    model/motifs/motif.h
    model/motifs/motif_connector.cpp
    model/motifs/motif_connector.h
    model/motifs/radial_motif.cpp
    model/motifs/radial_motif.h
    model/motifs/radial_ray.cpp
    model/motifs/radial_ray.h
    model/motifs/rosette.cpp
    model/motifs/rosette.h
    model/motifs/rosette2.cpp
    model/motifs/rosette2.h
    model/motifs/star.cpp
    model/motifs/star.h
    model/motifs/star2.cpp
    model/motifs/star2.h
    model/motifs/tile_color_defs.h
    model/motifs/tile_motif.cpp
    model/motifs/tile_motif.h
    model/prototypes/design_element.cpp
    model/prototypes/design_element.h
    model/prototypes/prototype.cpp
    model/prototypes/prototype.h
    model/settings/canvas.cpp
    model/settings/canvas.h
    model/settings/canvas_settings.cpp
    model/settings/canvas_settings.h
    model/settings/configuration.cpp
    model/settings/configuration.h
    model/settings/filldata.cpp
    model/settings/filldata.h
    model/settings/tristate.h
    model/settings/worklist.cpp
    model/settings/worklist.h
    model/styles/casing.cpp
    model/styles/casing.h
    model/styles/casing_data.h
    model/styles/casing_neighbours.cpp
    model/styles/casing_neighbours.h
    model/styles/casing_set.cpp
    model/styles/casing_set.h
    model/styles/casing_side.cpp
    model/styles/casing_side.h
    model/styles/colored.cpp
    model/styles/colored.h
    model/styles/colormaker.cpp
    model/styles/colormaker.h
    model/styles/colorset.cpp
    model/styles/colorset.h
    model/styles/emboss.cpp
    model/styles/emboss.h
    model/styles/filled.cpp
    model/styles/filled.h
    model/styles/interlace.cpp
    model/styles/interlace.h
    model/styles/interlace_casing.cpp
    model/styles/interlace_casing.h
    model/styles/interlace_side.cpp
    model/styles/interlace_side.h
    model/styles/outline.cpp
    model/styles/outline.h
    model/styles/outline_casing.cpp
    model/styles/outline_casing.h
    model/styles/plain.cpp
    model/styles/plain.h
    model/styles/sketch.cpp
    model/styles/sketch.h
    model/styles/style.cpp
    model/styles/style.h
    model/styles/thick.cpp
    model/styles/thick.h
    model/styles/tile_colors.cpp
    model/styles/tile_colors.h
    model/tilings/backgroundimage.cpp
    model/tilings/backgroundimage.h
    model/tilings//backgroundimage_perspective.cpp
    model/tilings//backgroundimage_perspective.h
    model/tilings/placed_tile.cpp
    model/tilings/placed_tile.h
    model/tilings/tile.cpp
    model/tilings/tile.h
    model/tilings/tile_reader.cpp
    model/tilings/tile_reader.h
    model/tilings/tile_writer.cpp
    model/tilings/tile_writer.h
    model/tilings/tiling.cpp
    model/tilings/tiling.h
    model/tilings/tiling_header.cpp
    model/tilings/tiling_header.h
    model/tilings/tiling_manager.cpp
    model/tilings/tiling_manager.h
    model/tilings/tiling_reader.cpp
    model/tilings/tiling_reader.h
    model/tilings/tiling_unit.cpp
    model/tilings/tiling_unit.cpp
    model/tilings/tiling_unit.h
    model/tilings/tiling_unit.h
    model/tilings/tiling_writer.cpp
    model/tilings/tiling_writer.h

    sys/engine/compare_bmp_engine.cpp
    sys/engine/compare_bmp_engine.h
    sys/engine/image_engine.cpp
    sys/engine/image_engine.h
    sys/engine/mosaic_bmp_generator.cpp
    sys/engine/mosaic_bmp_generator.h
    sys/engine/mosaic_stepper.cpp
    sys/engine/mosaic_stepper.h
    sys/engine/png_stepper.cpp
    sys/engine/png_stepper.h
    sys/engine/stepping_engine.cpp
    sys/engine/stepping_engine.h
    sys/engine/tiling_bmp_generator.cpp
    sys/engine/tiling_bmp_generator.h
    sys/engine/tiling_stepper.cpp
    sys/engine/tiling_stepper.h
    sys/engine/version_stepper.cpp
    sys/engine/version_stepper.h
    sys/engine/compare_bmp_stepper.cpp
    sys/engine/compare_bmp_stepper.h
    sys/engine/view_bmp_stepper.cpp
    sys/engine/view_bmp_stepper.h
    sys/enums/ebkgdimage.cpp
    sys/enums/ebkgdimage.h
    sys/enums/eborder.cpp
    sys/enums/eborder.h
    sys/enums/ecyclemode.h
    sys/enums/edesign.cpp
    sys/enums/edesign.h
    sys/enums/edgetype.cpp
    sys/enums/edgetype.h
    sys/enums/efilesystem.h
    sys/enums/efillmode.cpp
    sys/enums/efillmode.h
    sys/enums/efilltype.h


    sys/enums/elogmode.h
    sys/enums/emapeditor.cpp
    sys/enums/emapeditor.h
    sys/enums/emotiftype.cpp
    sys/enums/emotiftype.h
    sys/enums/emousemode.h
    sys/enums/epanelpage.cpp
    sys/enums/epanelpage.h
    sys/enums/erender.h
    sys/enums/estatemachineevent.cpp
    sys/enums/estatemachineevent.h
    sys/enums/estyletype.h
    sys/enums/etilingmaker.cpp
    sys/enums/etilingmaker.h
    sys/enums/eviewtype.cpp
    sys/enums/eviewtype.h
    sys/geometry/arcdata.cpp
    sys/geometry/arcdata.h
    sys/geometry/bounds.cpp
    sys/geometry/bounds.h
    sys/geometry/circle.cpp
    sys/geometry/circle.h
    sys/geometry/crop.cpp
    sys/geometry/crop.h
    sys/geometry/dcel.cpp
    sys/geometry/dcel.h
    sys/geometry/debug_map.cpp
    sys/geometry/debug_map.h
    sys/geometry/edge.cpp
    sys/geometry/edge.h
    sys/geometry/edge_poly.cpp
    sys/geometry/edge_poly.h
    sys/geometry/faces.cpp
    sys/geometry/faces.h
    sys/geometry/fill_region.cpp
    sys/geometry/fill_region.h
    sys/geometry/geo.cpp
    sys/geometry/geo.h
    sys/geometry/intersect.cpp
    sys/geometry/intersect.h
    sys/geometry/loose.cpp
    sys/geometry/loose.h
    sys/geometry/map.cpp
    sys/geometry/map.h
    sys/geometry/map_base.cpp
    sys/geometry/map_base.h
    sys/geometry/map_cleanser.cpp
    sys/geometry/map_cleanser.h
    sys/geometry/map_verifier.cpp
    sys/geometry/map_verifier.h
    sys/geometry/measurement.cpp
    sys/geometry/measurement.h
    sys/geometry/neighbour_map.cpp
    sys/geometry/neighbour_map.h
    sys/geometry/neighbours.cpp
    sys/geometry/neighbours.h
    sys/geometry/polygon.cpp
    sys/geometry/polygon.h
    sys/geometry/threads.cpp
    sys/geometry/threads.h
    sys/geometry/transform.cpp
    sys/geometry/transform.h
    sys/geometry/vertex.cpp
    sys/geometry/vertex.h
    sys/geometry/xform.cpp
    sys/geometry/xform.h
    sys/qt/qtapplog.cpp
    sys/qt/qtapplog.h
    sys/qt/timers.cpp
    sys/qt/timers.h
    sys/qt/tpm_io.h
    sys/qt/unique_qvector.h
    sys/qt/utilities.cpp
    sys/qt/utilities.h
    sys/sys/debugflags.h
    sys/sys/debugflags.cpp
    sys/sys/fileservices.cpp
    sys/sys/fileservices.h
    sys/sys/load_unit.cpp
    sys/sys/load_unit.h
    sys/sys/pugiconfig.hpp
    sys/sys/pugixml.cpp
    sys/sys/pugixml.hpp
    sys/sys/signal_blocker.h
    sys/sys/versioning.cpp
    sys/sys/versioning.h
    sys/main.cpp
    sys/sys.cpp
    sys/sys.h
    sys/tiledpatternmaker.cpp
    sys/tiledpatternmaker.h
    sys/version.h
)

set (OTHER_FILES
    ../etc/windows/aliases.txt
    ../etc/windows/binfo
    ../etc/windows/branches
    ../etc/windows/bstate
    ../etc/windows/build-install.md
    ../etc/windows/build-install-qt6.md
    ../etc/windows/configure-local.bat
    ../etc/windows/qt5.bat
    ../etc/windows/qt6.bat
    ../etc/linux/aliases.txt
    ../etc/linux/binfo
    ../etc/linux/branches
    ../etc/linux/bstate
    ../etc/linux/build-install.md
    ../etc/linux/build-install-qt6.md
    ../etc/linux/configure-local
    ../etc/TiledPatternMaker.nsi
    ../etc/models/Components.qmodel
    ../etc/models/designeditor.qmodel
    ../etc/models/Designs.qmodel
    ../etc/models/figure_editors.qmodel
    ../etc/models/figures.qmodel
    ../etc/models/makers.qmodel
    ../etc/models/makers2.qmodel
    ../etc/models/mosaic.qmodel
    ../etc/models/shapes.qmodel
    ../etc/models/styles.qmodel
    ../etc/models/Viewers.qmodel
    ../release-notes.txt
    txt/legacy.txt
    txt/mapeditor.txt
    txt/mosaic.txt
    txt/start-options.txt
    txt/support.txt
    txt/tilingmaker.txt
)

add_custom_target(Xtra-files SOURCES ${OTHER_FILES})

qt_add_resources(PROJECT_SOURCES tpm.qrc)

set(CMAKE_UNITY_BUILD ON)
if(WIN32)
    qt_add_executable(TiledPatternMaker WIN32 ${PROJECT_SOURCES} icon.rc)   # don't delete the WIN32
    target_link_options(TiledPatternMaker PRIVATE /STACK:32000000)
else()
    qt_add_executable(TiledPatternMaker ${PROJECT_SOURCES} icon.rc)
endif()

# Link Qt libraries
target_link_libraries(TiledPatternMaker PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
    Qt6::Concurrent
    Qt6::PrintSupport
)

# Optional: Unity batch size for better parallelism
set_target_properties(TiledPatternMaker PROPERTIES UNITY_BUILD_BATCH_SIZE 8)

message(STATUS "----------------------------------------")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Unity build enabled: ${CMAKE_UNITY_BUILD}")
get_target_property(UNITY_BATCH_SIZE TiledPatternMaker UNITY_BUILD_BATCH_SIZE)
message(STATUS "Unity batch size: ${UNITY_BATCH_SIZE}")
if(CCACHE_PROGRAM)
    message(STATUS "ccache: ${CCACHE_PROGRAM}")
else()
    message(STATUS "ccache: NOT FOUND")
endif()
message(STATUS "----------------------------------------")
