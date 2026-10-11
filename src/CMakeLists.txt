cmake_minimum_required(VERSION 3.21)

project(TiledPatternMaker LANGUAGES CXX)

# Enable Qt features
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Unity builds globally
set(CMAKE_UNITY_BUILD ON)

# Try to find ccache (with fallback path hint)
if (WIN32)
    find_program(CCACHE_PROGRAM ccache  PATHS "C:/ProgramData/chocolatey/bin")
else()
    find_program(CCACHE_PROGRAM ccache)
endif()

if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

if(CCACHE_PROGRAM)
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
else()
    message(WARNING "ccache not found, continuing without compiler cache.")
endif()

# Use mold linker if available — faster and far less memory-hungry than GNU ld
find_program(MOLD_LINKER mold)
if(MOLD_LINKER)
    add_link_options(-fuse-ld=mold)
    message(STATUS "Using mold linker")
endif()

# Give link jobs their own small pool so they don't all run concurrently
#set_property(GLOBAL PROPERTY JOB_POOLS compile_pool=16 link_pool=2)
#set(CMAKE_JOB_POOL_COMPILE compile_pool)
#set(CMAKE_JOB_POOL_LINK link_pool)

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg Concurrent PrintSupport)
qt_standard_project_setup()

# Project sources
set(PROJECT_SOURCES

    gui/map_editor/map_editor.cpp
    gui/map_editor/map_editor.h
    gui/map_editor/map_editor_db.cpp
    gui/map_editor/map_editor_db.h
    gui/map_editor/map_editor_layer.cpp
    gui/map_editor/map_editor_layer.h
    gui/map_editor/map_editor_map_loader.cpp
    gui/map_editor/map_editor_map_loader.h
    gui/map_editor/map_editor_map_writer.cpp
    gui/map_editor/map_editor_map_writer.h
    gui/map_editor/map_editor_maps.cpp
    gui/map_editor/map_editor_maps.h
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

    gui/model_editors/motif_edit/adaptive_motif_editors.cpp
    gui/model_editors/motif_edit/adaptive_motif_editors.h
    gui/model_editors/motif_edit/extender_editor.cpp
    gui/model_editors/motif_edit/extender_editor.h
    gui/model_editors/motif_edit/lower_third_editor.cpp
    gui/model_editors/motif_edit/lower_third_editor.h
    gui/model_editors/motif_edit/motif_editor.cpp
    gui/model_editors/motif_edit/motif_editor.h
    gui/model_editors/motif_edit/motif_editor_widget.cpp
    gui/model_editors/motif_edit/motif_editor_widget.h
    gui/model_editors/motif_edit/motif_maker_widget.cpp
    gui/model_editors/motif_edit/motif_maker_widget.h
    gui/model_editors/motif_edit/motif_type_selector.cpp
    gui/model_editors/motif_edit/motif_type_selector.h
    gui/model_editors/motif_edit/radial_motif_editors.cpp
    gui/model_editors/motif_edit/radial_motif_editors.h
    gui/model_editors/motif_edit/tile_motif_button.cpp
    gui/model_editors/motif_edit/tile_motif_button.h

    gui/model_editors/style_edit/border_editor.cpp
    gui/model_editors/style_edit/border_editor.h
    gui/model_editors/style_edit/colored_editor.cpp
    gui/model_editors/style_edit/colored_editor.h
    gui/model_editors/style_edit/emboss_editor.cpp
    gui/model_editors/style_edit/emboss_editor.h
    gui/model_editors/style_edit/fill_editor.cpp
    gui/model_editors/style_edit/fill_editor.h
    gui/model_editors/style_edit/fill_face_editor.cpp
    gui/model_editors/style_edit/fill_face_editor.h
    gui/model_editors/style_edit/fill_group_editor.cpp
    gui/model_editors/style_edit/fill_group_editor.h
    gui/model_editors/style_edit/fill_new1_editor.cpp
    gui/model_editors/style_edit/fill_new1_editor.h
    gui/model_editors/style_edit/fill_original_editor.cpp
    gui/model_editors/style_edit/fill_original_editor.h
    gui/model_editors/style_edit/fill_set_editor.cpp
    gui/model_editors/style_edit/fill_set_editor.h
    gui/model_editors/style_edit/interlace_editor.cpp
    gui/model_editors/style_edit/interlace_editor.h
    gui/model_editors/style_edit/style_editor.cpp
    gui/model_editors/style_edit/style_editor.h
    gui/model_editors/style_edit/thick_editor.cpp
    gui/model_editors/style_edit/thick_editor.h
    gui/model_editors/style_edit/tile_colors_editor.cpp
    gui/model_editors/style_edit/tile_colors_editor.h

    gui/model_editors/tiling_edit/tile_selection.cpp
    gui/model_editors/tiling_edit/tile_selection.h
    gui/model_editors/tiling_edit/tiling_mouseactions.cpp
    gui/model_editors/tiling_edit/tiling_mouseactions.h

    gui/panels/page_backgrounds.cpp
    gui/panels/page_backgrounds.h
    gui/panels/page_borders.cpp
    gui/panels/page_borders.h
    gui/panels/page_canvas_settings.cpp
    gui/panels/page_canvas_settings.h
    gui/panels/page_crop_maker.cpp
    gui/panels/page_crop_maker.h
    gui/panels/page_debug.cpp
    gui/panels/page_debug.h
    gui/panels/page_grid.cpp
    gui/panels/page_grid.h
    gui/panels/page_image_tools.cpp
    gui/panels/page_image_tools.h
    gui/panels/page_layer_alignmentl.cpp
    gui/panels/page_layer_alignmentl.h
    gui/panels/page_layers.cpp
    gui/panels/page_layers.h
    gui/panels/page_layers_tabs.cpp
    gui/panels/page_layers_tabs.h
    gui/panels/page_loaders.cpp
    gui/panels/page_loaders.h
    gui/panels/page_log.cpp
    gui/panels/page_log.h
    gui/panels/page_map_editor.cpp
    gui/panels/page_map_editor.h
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
    gui/panels/page_settings.cpp
    gui/panels/page_settings.h
    gui/panels/page_system_info.cpp
    gui/panels/page_system_info.h
    gui/panels/page_tiling_maker.cpp
    gui/panels/page_tiling_maker.h
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
    gui/panels/panel_worker.cpp
    gui/panels/panel_worker.h
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
    gui/top/view_accessor.cpp
    gui/top/view_accessor.h
    gui/top/view_controller.cpp
    gui/top/view_controller.h

    gui/viewers/construction_lines_view.cpp
    gui/viewers/construction_lines_view.h
    gui/viewers/crop_maker_view.cpp
    gui/viewers/crop_maker_view.h
    gui/viewers/debug_view.cpp
    gui/viewers/debug_view.h
    gui/viewers/geo_graphics.cpp
    gui/viewers/geo_graphics.h
    gui/viewers/grid_view.cpp
    gui/viewers/grid_view.h
    gui/viewers/image_view.cpp
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

    gui/widgets/colorset_widget.cpp
    gui/widgets/colorset_widget.h
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
    gui/widgets/dlg_rosette_tips.cpp
    gui/widgets/dlg_rosette_tips.h
    gui/widgets/dlg_savename.cpp
    gui/widgets/dlg_savename.h
    gui/widgets/dlg_saveversion.cpp
    gui/widgets/dlg_saveversion.h
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
    gui/widgets/panel_misc.cpp
    gui/widgets/panel_misc.h
    gui/widgets/position_widget.h
    gui/widgets/position_widget.cpp
    gui/widgets/rounded_polygon.cpp
    gui/widgets/rounded_polygon.h
    gui/widgets/smx_widget.cpp
    gui/widgets/smx_widget.h
    gui/widgets/transparent_widget.cpp
    gui/widgets/transparent_widget.h
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
    model/makers/motif_maker.cpp
    model/makers/motif_maker.h
    model/makers/motif_view_data.cpp
    model/makers/motif_view_data.h
    model/makers/proto_view_data.cpp
    model/makers/proto_view_data.h
    model/makers/tiling_maker.cpp
    model/makers/tiling_maker.h

    model/borders/border.cpp
    model/borders/border.h
    model/borders/border_2color.cpp
    model/borders/border_2color.h
    model/borders/border_blocks.cpp
    model/borders/border_blocks.h
    model/borders/border_plain.cpp
    model/borders/border_plain.h

    model/mosaics/legacy_loader.cpp
    model/mosaics/legacy_loader.h
    model/mosaics/mosaic.cpp
    model/mosaics/mosaic.h
    model/mosaics/mosaic_reader.cpp
    model/mosaics/mosaic_reader.h
    model/mosaics/mosaic_writer.cpp
    model/mosaics/mosaic_writer.h
    model/mosaics/reader_base.cpp
    model/mosaics/reader_base.h
    model/mosaics/writer_base.cpp
    model/mosaics/writer_base.h

    model/motifs/adaptive_extender.cpp
    model/motifs/adaptive_extender.h
    model/motifs/adaptive_girih.cpp
    model/motifs/adaptive_girih.h
    model/motifs/adaptive_girih_branches.cpp
    model/motifs/adaptive_girih_branches.h
    model/motifs/adaptive_hourglass.cpp
    model/motifs/adaptive_hourglass.h
    model/motifs/adaptive_inference.cpp
    model/motifs/adaptive_inference.h
    model/motifs/adaptive_intersect.cpp
    model/motifs/adaptive_intersect.h
    model/motifs/adaptive_motif.cpp
    model/motifs/adaptive_motif.h
    model/motifs/adaptive_rosette.cpp
    model/motifs/adaptive_rosette.h
    model/motifs/adaptive_star.cpp
    model/motifs/adaptive_star.h
    model/motifs/adaptive_star_branches.cpp
    model/motifs/adaptive_star_branches.h
    model/motifs/adaptive_tile.cpp
    model/motifs/adaptive_tile.h
    model/motifs/adaptive_tools.cpp
    model/motifs/adaptive_tools.h
    model/motifs/explicit_map_motif.cpp
    model/motifs/explicit_map_motif.h
    model/motifs/extended_boundary.cpp
    model/motifs/extended_boundary.h
    model/motifs/extender.cpp
    model/motifs/extender.h

    model/motifs/motif.cpp
    model/motifs/motif.h
    model/motifs/radial_extender.cpp
    model/motifs/radial_extender.h
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

    model/prototypes/tile_motif.cpp
    model/prototypes/tile_motif.h
    model/prototypes/prototype.cpp
    model/prototypes/prototype.h

    model/settings/canvas.cpp
    model/settings/canvas.h
    model/settings/configuration.cpp
    model/settings/configuration.h
    model/settings/filldata.cpp
    model/settings/filldata.h
    model/settings/distortion.cpp
    model/settings/distortion.h
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
    model/styles/colorset.cpp
    model/styles/colorset.h
    model/styles/emboss.cpp
    model/styles/emboss.h
    model/styles/fill_color_group.cpp
    model/styles/fill_color_group.h
    model/styles/fill_color_maker.cpp
    model/styles/fill_color_maker.h
    model/styles/fill_color_set.cpp
    model/styles/fill_color_set.h
    model/styles/fill_deprecated.cpp
    model/styles/fill_deprecated.h
    model/styles/fill_faces.cpp
    model/styles/fill_faces.h
    model/styles/fill_new1.cpp
    model/styles/fill_new1.h
    model/styles/fill_original.cpp
    model/styles/fill_original.h
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
    model/tilings/backgroundimage_perspective.cpp
    model/tilings/backgroundimage_perspective.h
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
    model/tilings/tiling_reader.cpp
    model/tilings/tiling_reader.h
    model/tilings/tiling_unit.cpp
    model/tilings/tiling_unit.h
    model/tilings/tiling_writer.cpp
    model/tilings/tiling_writer.h

    sys/engine/compare_bmp_engine.cpp
    sys/engine/compare_bmp_engine.h
    sys/engine/compare_bmp_stepper.cpp
    sys/engine/compare_bmp_stepper.h
    sys/engine/convert_bmp_engine.cpp
    sys/engine/convert_bmp_engine.h
    sys/engine/image_engine.cpp
    sys/engine/image_engine.h
    sys/engine/mosaic_bmp_generator.cpp
    sys/engine/mosaic_bmp_generator.h
    sys/engine/mosaic_stepper.cpp
    sys/engine/mosaic_stepper.h
    sys/engine/mosaic_xml_regenerator.cpp
    sys/engine/mosaic_xml_regenerator.h
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
    sys/engine/view_bmp_stepper.cpp
    sys/engine/view_bmp_stepper.h

    sys/enums/eactivecanvas.h
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
    sys/enums/efilltype.cpp
    sys/enums/efilltype.h
    sys/enums/elogmode.h
    sys/enums/emousemode.h
    sys/enums/epanelpage.cpp
    sys/enums/epanelpage.h
    sys/enums/erender.h
    sys/enums/estyletype.h
    sys/enums/etilingmaker.h

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
    manifest.xml
    sys/version.cpp.in
    ../etc/windows/aliases.txt
    ../etc/TiledPatternMaker.nsi
    ../etc/linux/aliases.txt
    ../etc/linux/binfo
    ../etc/linux/branches
    ../etc/linux/bstate
    ../etc/linux/build-install-qt6.md
    ../etc/linux/build-install.md
    ../etc/linux/configure-local
    ../etc/models/Components.qmodel
    ../etc/models/Designs.qmodel
    ../etc/models/Viewers.qmodel
    ../etc/models/designeditor.qmodel
    ../etc/models/figure_editors.qmodel
    ../etc/models/figures.qmodel
    ../etc/models/makers.qmodel
    ../etc/models/makers2.qmodel
    ../etc/models/mosaic.qmodel
    ../etc/models/motifmaker.qmodel
    ../etc/models/shapes.qmodel
    ../etc/models/styles.qmodel
    ../etc/models/viewers2.qmodel
    ../etc/windows/binfo
    ../etc/windows/branches
    ../etc/windows/bstate
    ../etc/windows/build-install-qt6.md
    ../etc/windows/build-install.md
    ../etc/windows/configure-local.bat
    ../etc/windows/qt5.bat
    ../etc/windows/qt6.bat
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
    qt_import_plugins(TiledPatternMaker INCLUDE Qt6::QXcbIntegrationPlugin)
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

# Build timestamp generation (update when any real source changes)
set(VERSION_CPP ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)
set(VERSION_TMP ${CMAKE_CURRENT_BINARY_DIR}/version.tmp)

add_custom_command(
    OUTPUT ${VERSION_CPP}
    COMMAND ${CMAKE_COMMAND}
            -DOUT_FILE=${VERSION_TMP}
            -DVERSION_IN_DIR=${CMAKE_CURRENT_SOURCE_DIR}/sys
            -P ${CMAKE_CURRENT_SOURCE_DIR}/sys/gen_version.cmake
    COMMAND ${CMAKE_COMMAND} -E compare_files ${VERSION_TMP} ${VERSION_CPP} ||
            ${CMAKE_COMMAND} -E copy ${VERSION_TMP} ${VERSION_CPP}
    DEPENDS ${PROJECT_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/sys/version.cpp.in
    COMMENT "Updating version.cpp only if timestamp changed"
)

add_custom_target(buildinfo_touch DEPENDS ${VERSION_CPP})
add_dependencies(TiledPatternMaker buildinfo_touch)

# Make sure the generated file is compiled
target_sources(TiledPatternMaker PRIVATE ${VERSION_CPP})

set_source_files_properties(${VERSION_CPP} PROPERTIES UNITY_BUILD OFF)

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

# ---------------------------------------------------------------------------
# Linux packaging (.deb via CPack) — has no effect on Windows/Mac builds
# ---------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    set_target_properties(TiledPatternMaker PROPERTIES OUTPUT_NAME "tiledpatternmaker")
    install(TARGETS TiledPatternMaker       RUNTIME DESTINATION bin)
    install(FILES tiledpatternmaker.desktop DESTINATION share/applications)
    install(FILES tiledpatternmaker.png     DESTINATION share/icons/hicolor/256x256/apps)
    install(DIRECTORY ../media/             DESTINATION share/tiledpatternmaker/media)
    install(FILES manifest.xml              DESTINATION share/tiledpatternmaker/media)

    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_PACKAGE_NAME "tiledpatternmaker")
    set(CPACK_PACKAGE_VERSION "4.3.0")
    set(CPACK_PACKAGE_CONTACT "dcportal3@gmail.com")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "David  Casper <dcportal3@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Design tool for Mosaic/Islamic geometric patterns")

    include(CPack)

endif()