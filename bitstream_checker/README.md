# Bit Stream Checker

Project of tool for checking stream of bits.


## Overview ##

There are some streams with bits which may be pulled one byte after another.

System is responsible for validating streams states.
If in stream appear three bits (next to each other) with this same value, stream state is changed to *invalid*, otherwise stream state is *valid*.
