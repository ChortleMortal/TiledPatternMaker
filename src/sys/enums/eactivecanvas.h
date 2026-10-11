#pragma once

// which real owner (Tiling or Mosaic) the interactive session's active canvas
// currently derives from - see Sys::getActiveCanvas()
enum eActiveCanvas
{
    ACTIVE_TILING,
    ACTIVE_MOSAIC
};
