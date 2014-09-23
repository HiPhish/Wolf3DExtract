Program design
==============

The program's code constists of several small "modules" and one central main unit that calls the modules. The modules themselves are self-contained and could in theory be compiled into their own libraries and used in another program. The compression module is used by the other modules for decompression, but has no dependencies on its own. In fact it's not even tied to the data formats of Wolfenstein 3D, just the decompression algorithms.

As more data formats are understood more modules will be added.

Compression module
------------------
This module implements decompression via the RLEW- and Carmack algorithms. The interface allows passing in a buffer of compressed data and a pre-allocated buffer for the decompressed data. The funcions don't require knowledge about the meaning of the data.

Map extraction module
---------------------
This module is responsible for loading, reading and decompressing the map data. The interface declares the data structures used and functions for filling them with data, but is not concerned with further processing, such as printing the data to a file.
