# libp2p
Pack/unpack pixels.

The "p2p" library implements conversion between packed and planar image
formats. A packed format is any memory layout that stores more than one image
component ("plane") in a single array. For example, the common ARGB format
stores pixels as an array of DWORDs holding all components for each pixel. In
contrast, a planar format stores each image component in its own array.

Building
------
libp2p is intended for embedding within other libraries and applications. The
header "p2p.h" contains a template library for generating packing and unpacking
routines, which can be used to instantiate functions for various pixel formats.
"v210.cpp" holds a special-case implementation for the Apple ProRes "v210"
format. "p2p_api.h" and "p2p_api.cpp" implement a "C" wrapper for a fixed set
of commonly encountered packed formats. If the "C" wrapper is used from another
library, a method to control symbol visibility should be used to prevent name
conflicts with other, potentially incompatible, instances of libp2p.
