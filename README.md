# SVC Scene View Controller

## Target

Create simple, portable and modificable scene-view-controller for using it in
other projects.


## Requirements

1. Scene and Items must not contains operation for drawing or displaing it

2. All operation of drawing and displaing must be a part of view

3. Scene must provide functional for append, remove and move Items in
2-dimensonal space

4. All information about position, rotating or scale of Scene, Items or
View must store in matricies. We should not use different variables for store
the information

5. Scene must use cartesian koordinate system as default

6. Scene must have conditionally-infinity area

7. Scene must have functionality for quering items. Queries can be:
    - all
    - intersects with Point
    - intersects with Box
    - intersects with Shape
    - collisions for some Item

8. Scene must have functionality for ordering items. Need for set some custom
order for item

9. Items can have Shape

10. Items can be grouped

11. Need functionality for saving Scene in file and restoring it

12. View need functionality for displaing only part of scene, scaling, rotating
and zooming

13. Need functional for selecting one or several items for next using

14. Scene, View and Items must be modificable. We can inherite it, add new
members and methods

15. Minimalism


## Technologys

1. C++

2. Boost:
    - Geometry
    - Qvm
    - Serialization
    - Signals2

3. Sdl

4. OpenGl ES 3

5. ImGUI

6. Catch2


## Design

1. Scene and View must be realized by `Bridge` pattern

2. Use `rtree` for storing and quering Items in Scene. `Boost.Geometry` have good
implementation of `rtree`

3. Items must be realized by `Compositor` pattern (see requirement N10). Item
can have only one `parent` and not limited capacity of `children`

4. Items must support `Visitor` pattern. This especially needed for moving all
drawing operation to View (see requirement N1)

5. Scene and Items have to support archive operation from `Boost.Serialization`
(see requirement N11)

6. All drawing in View must be realized by `OpenGl ES 3`

7. All information for drawing (textures, vbo, ebo and other) must be load, sove
and use on View side

8. All matricies must be affine-transformation matricies (see requirement N3 and
N 4) and have size 3x2 (columnsXrows). All vectors must have only 2 numbers
