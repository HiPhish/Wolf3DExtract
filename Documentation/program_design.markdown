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

Pic extract module
------------------
This module is responsible for loading, reading and decompressing the bitmap picture data. A bitmap picture is a 2D image that's neither a sprite nor a texture. They are used mostly as elements for the visual interface and menus. The resulting file of picure extraction constists of two signed 16-bit integers for the width and height, followed by a linear sequence of pixel bytes. These pixels are ordered the same way way as they were stored by Id back in the day, so they need to be converted to something modern afterwards.

vga2ppm converter
=================
The pictures extracted are stored in a custom VGA format, which is useless for other programs. This converter program will convert the VGA file into a binary PPM file; the source file is the standard input and the destination file is the standard output. The VGA pixel bytes are mapped to 32-bit RGBA values and ordered correctly.

While PPM is a very obscure standard it is a standard and can be converted to something more common, such as PNG, using another converter such as ImageMagick. Here is an example in Unix:

	./wolf3dextract -pic 3 | ./vga2ppm | convert /dev/stdin pic_3.png

First the picture with the magic number 3 is extracted to the standard output, then the output is piped into the standard input of the converter. Finally, its standard output is piped into the standard input of ImageMagick and passed as an argument followed by the output file name.
