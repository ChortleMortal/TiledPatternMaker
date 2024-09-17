#include "gui/panels/shortcuts.h"

QString Shortcuts::getMosaicShortcuts()
{
    const QString shortcuts = " \
<pre><br><br>\
    A   <br>\
    B   <br>\
    C   <br>\
    D   Duplicate (in new window)<br>\
    E   <br>\
    F   <br>\
    G   Grid (toggle) <br>\
    H   Toggle Show Center/ Toggle Circle<br>\
    I   <br>\
    J   Save Menu BMP<br>\
    K   Toggle debugMap in Motif Maker View<br>\
    L   <br>\
    M   Raise Menu<br>\
    N   Bring to center of primary screen<br>\
    O   <br>\
    P   Print Save Image BMP <br>\
    Q   Quit app (or quit cycler)<br>\
    R   Toggle replication in Motif Maker View<br>\
    S   <br>\
    T   Xfrom mode MODEL DATA<br>\
    U   Xform mode BKGD<br>\
    V   Xform mode VIEW<br>\
    W   Xform mode OBJECT DATA<br>\
    X   Show Center/Show circle (X toggle) <br>\
    Y   Save Image SVG<br>\
    Z   <br>\
        <br>\
    ESC MODE_DEFAULT<br>\
    F1  Help (show this)<br>\
    F2  MODE_TRANSFORM up/down/left/right/,/./-/= <br>\
    F3  MODE_MOVE window up/down/left/right<br>\
    F4  Dump info <br>\
    F5  Unload canvas (drain the swamp) <br>\
       <br>\
</pre> \
";
    return shortcuts;
}


QString Shortcuts::getDesignShortcuts()
{
    const QString shortcuts = " \
<pre><br><br>\
    A   MODE_ORIGIN    up/alt-up/down/alt-down/left/alt-left/right/alt-right <br>\
    B   MODE_OFFSET    up/down/left/right <br>\
    C   <br>\
    D   Duplicate (in new window)<br>\
    E   <br>\
    F   <br>\
    G   Grid (toggle) <br>\
    H   Hide Circles (toggle) <br>\
    I   Layer In (show) <br>\
    J   <br>\
    K   <br>\
    L   MODE_LAYER  0-9 <br>\
    M   Raise Menu<br>\
    N   Bring to center of primary screen<br>\
    O   Layer Out (hide) <br>\
    P   Print (actually Save Image) <br>\
    Q   Quit app (or quit cycler)<br>\
    R   <br>\
ALT-R   Run (start timer and mode layer) <br>\
    S   MODE_SEPARATION  up/down/left/right <br>\
ALT-S   MODE_STEP  up/down  0-9 <br>\
    T   <br>\
    U   <br>\
    V   <br>\
    W   <br>\
    X   Show Center/Show circle (X toggle) <br>\
    Y   Save as SVG<br>\
    Z   MODE_ZLEVEL   up/down <br>\
        <br>\
    0   Toggle Design 0 visibility <br>\
    1   Toggle Design 0 visibility <br>\
    2   Toggle Design 2 visibility <br>\
    3   Toggle Design 3 visibility <br>\
    ESC MODE_DEFAULT<br>\
    F1  Help (show this)<br>\
    F2  MODE_TRANSFORM up/down/left/right/,/./-/= <br>\
    F3  <br>\
    F4  Dump info <br>\
    F5  Unload canvas (drain the swamp) <br>\
       <br>\
</pre> \
";
    return shortcuts;
}

QString Shortcuts::getTilingMakerShortcuts()
{
    const  QString td_shortcuts = " \
<pre><br><br>\
Modes:<br>\
    ESC Normal Mode<br>\
    F1  Help (show this)<br>\
    F2  Adjust View: up/down/left/right/,/./-/= <br>\
          Use arrow keys to position,./, keys to rotate. and   +/- keys to zoom<br>\
          Also use 'Set Center' to set center for rotation and zooming\
    F3  Translation Vector Mode<br>\
          Use the mouse to draw the two translation vectors used to tile the plane<br>\
    F4  New Polygon Mode<br>\
          Click on vertices or whitespace to make a point. Chain a series of points counter-clockwise to draw a free-form polygon<br>\
    F5  Copy Polygons Mode<br>\
          Copy polygons by drag-and-drop with the mouse<br>\
    F6  Delete Polygons Mode<br>\
          Click on a polygons to delete them<br>\
    F7  Include/Excwlude Polygons Mode<br>\
          Click on polygons to toggle inclusion/exclusion in the tiling<br>\
    F8  Position Mode<br>\
    F9  Measure Mode<br>\
    F10 Unify Mode<br>\
          Click on each tile to unify (meaning same tile with different position) Press ESC to end<br>\
<br><br>\
Actions:<br>\
    A  Add Polygon<br>\
          Adds a regular polygon with N sides, where N is a number selected using the keyboard.<br>\
    C  Copy Polygon<br>\
          Copies the selected (clicked) polygon.  It can now be dragged<br>\
    D  Delete Polygon<br>\
         Deletes the selected (clicked) polygon<br>\
    E  Exclude All Polygons<br>\
          Excludes all polygons from the tiling<br>\
    F  Fill Using Translation Vectors<br>\
          Surrounds the design with copies using the translation vectors<br>\
    H  Toggles Show Centers<br>\
    I  Include/Exclude Polygons in Tiling<br>\
          Toggles the inclusion/exclusion of the polygon under the mouse<br>\
    R  Remove Excluded Polygons<br>\
          Removes all polygons that are not included in the tiling<br>\
    Q  Quit<br>\
          Exit the application<br>\
    X  Clear Translation Vectors<br>\
          Clears the translation vectors used to tile the plane<br>\
 </pre>\
";
    return td_shortcuts;
}

QString Shortcuts::getMapEditorShortcuts()
{
    const QString fme_shortcuts = " \
<pre><br><br>\
Modes:<br>\
    ESC: no mode<br>\
    F1 : Help (show this)<br>\
    F2 : TRANSFORM up/down/left/right/,/./-/= <br>\
    F3 : Draw Construction Lines<br>\
    F4 : Draw lines<br>\
    F5 : Delete selected<br>\
    F6 : Split Edge in middle<br>\
    F7 : Extend Line<br>\
    F8 : Create Crop Mask<br>\
    F9 : Draw Consctruction Circles<br>\
            Right-click = create<br>\
            Left-click and drag = move<br>\
            Shift/Left-click = resize<br>\
<br>\
Actions:<br>\
    F  Flip Line Extension<br>\
    M  Raise Menu<br>\
    Q  Quit - Exit the application<br>\
 </pre>\
";
    return fme_shortcuts;
}
