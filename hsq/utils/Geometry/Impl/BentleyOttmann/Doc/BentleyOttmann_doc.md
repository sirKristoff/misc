Private submodule to Geometry

Implements a sweep line algorithm (Bentley-Ottmann) used for finding all intersections in a set of line segments (or polygon).

The main approach is to use a vertical line, i.e. the sweep line, that moves from left to right across the plane.
This sweep line structure intersects the input line segment set (or polygon) in sequence as it moves,
and records all points where two line segments intersects one another.

The implementation is based on two different data structures:
1. The sweep line itself that uses a binary search tree for storing all intersected line segments.
2. An event queue storing all future 'events' (i.e. line segment end points). 
   This queue is processed by checking each element one by one. 