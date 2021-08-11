### Generic description of Geometry module

This 'module' contains the interfaces and implementations for different geometrical objects and their helper functions. The module contains the following sub-modules:
* ICoordinate: The type is globally available, but this interface contains various math operations that can be applied to coordinates
* ILine: Represented by two coordinates, with related functions
* IPath: An open shape, containing a number of coordinates to represent one or more connected lines
* IPolygon: A closed shape, containing a number of coordinates, with an implicit edge connecting the last node to the first
* IShape: Base container for shapes with a number of coordinates (e.g. IPath&IPolygon)
* IPolar: Polar coordinates with functions for translating to from or tICoordinate2D
