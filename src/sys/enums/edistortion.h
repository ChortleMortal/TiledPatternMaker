#pragma once

// how a canvas's Distortion is applied - see Distortion, Canvas::computeCanvasTransform()
enum eDistortionMode
{
    DISTORT_NONE,
    DISTORT_CANVAS,
    DISTORT_TILE
};
