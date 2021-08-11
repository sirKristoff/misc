### Binary Search Tree

Binary search tree (not self-balancing) for storing data.

Left subtree of a node contains only nodes with lesser than the node's key data.
Right subtree of a node contains only nodes with greater than the node's key data.
What is "lesser" and "greater" is defined by a user-specified comparison function. 
Allows for usage of either dynamically or statically allocated memory.

The module does NOT deal with memory allocation/deallocation. It is the 
user's responsibility to assure that memory management is done in the 
correct way.