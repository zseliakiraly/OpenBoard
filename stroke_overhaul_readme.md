This readme should only be in the stroke-overhaul branch! Delete it if and when you merge this branch with any main one!

# Stroke system overhaul

This branch contains my attempts at overhauling OpenBoard's stroke system, which is a bit of a kludge currently.
It is a work in progress -- unfortunately I have not had time to finish what I started, but there are some promising leads.


## Current implementation
UBGraphicsScene handles most of the stroke drawing. 
It has a `UBGraphicsStroke`, `mCurrentStroke`, which is created in `inputDevicePress`. This stroke holds a list of polygons, and the polygons have a pointer to this stroke. However, `UBGraphicsStroke` is not the graphical element you see on the board, that allows you e.g to select a stroke, move it, scale it etc. This is handled by `UBGraphicsStrokesGroup`, which also holds that list of polygons but is supposed to simplify all the transformation operations that are made on the stroke as a whole.

Unfortunately, this makes the whole thing quite convoluted, since we first create a `UBGraphicsStroke`, then create a bunch of `UBGraphicsPolygonItems`, which are first parented to the `UBGraphicsStroke`; but when the input device is released, we replace the `stroke` by a `UBGraphicsStrokesGroup`, and reparent the `polygonItems` to it.

On top of that, there is poor encapsulation, in the sense that the scene does a lot of operations directly on the `polygonItems`, rather than on the stroke (which would then worry about the specifics of accessing the polygonItems when necessary). For example, when deep-copying a scene, the scene copies all polygonItems, builds a new stroke etc. Rather than just calling a function like `UBGraphicsStrokesGroup::duplicate`.


## Goals 
It is mostly this poor encapsulation that makes maintaining the code very complicated; there is a very steep learning curve and it takes a long time just to figure out what's going on, let alone figure out how to fix bugs or add features relating to strokes.

Therefore, this little project aimed to:

- Get rid of the old `UBGraphicsStroke`; rethink the `stroke` / `strokesGroup` / `polygonItem` concept
- Move all the drawing code from `UBGraphicsScene` to `UBDrawingController`.
- Simplify the system and have well-encapsulated, easy-to-maintain code. For example, it should be possible to duplicate a stroke by simply calling a function on it; the scene shouldn't have to be affected by how the stroke is represented internally (paths, polygons or other objects)


## New implementation

Please refer to the commits in this branch for more details. Below is a summary of the new implementation. 

A new UBGraphicsStroke class was implemented, which is essentially a mix of the old `UBGraphicsStroke` and `UBGraphicsStrokesGroup` classes. It is both a graphical element which can be moved, scaled etc, and contains all the variables and methods needed to draw the stroke.
I also removed the `UBGraphicsPolygonItem` class, which I thought we could do without.

Most of the drawing functionality was moved to `UBDrawingController` and to the new `UBGraphicsStroke`. 
Now, when the user starts drawing (as determined by `UBGraphicsScene::inputDevicePress`, `UBDrawingController::beginStroke` is called. This creates a new stroke, initializes its color, width etc. and adds it to the scene. `UBDrawingController::newStrokePoint` is called when the input device is moved, and all it really does is tell the stroke to add a new point to itself. As far as the rest of the program is concerned, that's all there is to it. Everything else is performed by the stroke.


The stroke is represented as a list of points (actually a list of tuples of a position and width at that position), which is the minimum information needed to recreate the stroke.
With these points, we can draw the stroke. Originally, I wanted to draw it using just a `QPainterPath`: this would remove the need for polygons altogether, and give access to some very helpful functions of the `QPainterPath` class (such as `simplify`, `addPath`, and so on). 
Unfortunately, this is where I ran into performance issues. Adding subpaths to an existing paths requires an update to the entire path in order to display the new segment, which is very slow. Likewise, subtracting from a path is even slower. 

So there is no way around the use of polygons. However, `QGraphicsPolygonItem`s do the job, and when parented to the stroke, they inherit all transforms etc. The scene doesn't have to worry about them; there is no need to explicitly add them to the scene, or anything similar that the old implementation did. 

Currently both solutions are implemented, with a bunch of lines being commented out. So the stroke has both a path and polygons. This should be simplified when a good, final solution is found.

For a little more insight into this, here is what I wrote about it as I implemented it:

> addPoint() creates a QPainterPath (thanks to UBGeometryUtils::curveToPath). This is super easily added to the path, but unfortunately this is problematic. Updating the entire path every time a subpath is added is very slow for large paths; updating only the new region (via update(QRectF)) doesn't work reliably.
> So I have done something like the old system: the new subpath is converted to a polygon, and a QGraphicsPolygonItem is created from it. The stroke contains a list of these polygons. Contrary to the old implementation, these aren't added to the scene.
> And since they are only necessary while drawing the stroke, I have made it an option to paint either the path or the polygons in UBGraphicsStroke::paint. The original idea of this was to paint the polygonItems while drawing the stroke, but paint the path afterwards. However, that doesn't currently work well for translucent strokes, so.. tbd.

## Current progress and future work

As explained above, `UBGraphicsScene` was heavily simplified, with all the drawing functions removed from it. Some were implemented in `UBDrawingController` and `UBGraphicsStroke`, but some still need to be re-implemented, such as the line and compass tools.
Strokes can be selected, transformed (moved, scaled, rotated), and duplicated. The eraser tool works, but undoing/redoing is still a work in progress. What's more, erasing is currently slow if there are many strokes on the page.
Transparency is supported, but in a different way from the old system: it is managed by a `QGraphicsOpacityEffect`, applied to the stroke. This means the individual polygons are opaque, and there is no fuss about avoiding overlapping regions between polygons of the same stroke (previously, when adding a new polygon to a transparent stroke, any overlapping regions had to be removed from the new polygon).

Here is an approximate list of things that still need to be done:

- [ ] Saving
-    [ ] Make sure that transparent strokes are saved in a way that they can be displayed by older versions
- [ ] Loading
- [ ] Undo/redo erasing part of stroke [partially OK; redoing doesn't reparent the polygons to the stroke, so they are not moved together]
- [ ] Undo/redo erasing entire stroke
- [ ] Re-implement line tool
- [ ] Re-implement compass tool
- [ ] Clean up UBGraphicsStroke.cpp (order of functions etc)
- [ ] Clean up other classes
- [ ] Get rid of UBInterpolator class [this was done in branch 1.4-dev, it "just" needs to be merged]
- [ ] Make sure the JS API still works
- [ ] Check for TODO statements, make sure everything is resolved
- [ ] Clean up / squash commits
- [ ] Test and eventually merge to main development branch
