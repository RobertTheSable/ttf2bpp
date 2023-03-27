# TTF-2-BPP

This program takes a Fretype-compatible font file as input, and renders it in 
the SNES 2BPP tile format. It may also be used to convert to/from an 
intermediate PNG file.

The program also generates a Yaml file with a list of encoding information 
for use in a [Sable](https://github.com/robertTheSable/sable-text-converter) font file.

## Usage

### Convert a font to 2BPP format based on a Unicode block

Requires the [ucd.all.grouped.xml](https://www.unicode.org/Public/UCD/latest/ucdxml/) file to be present int he working directory.

```
ttf-2-bpp.exe -u ASCII some-font.ttf
```

The Unicode block name must correspon to the block's "blk" attribute in 
the [Unicode Character Database](https://www.unicode.org/Public/UCD/latest/ucdxml/).


### Convert a font to 2BPP format based on two Unicode blocks

Requires the [ucd.all.grouped.xml](https://www.unicode.org/Public/UCD/latest/ucdxml/) file to be present in the working directory.

```
ttf-2-bpp.exe -u ASCII -u Latin_1_Sup some-font.ttf
```

### Convert a font to 2BPP format to a png file

```
ttf-2-bpp.exe -u ASCII some-font.ttf some-font.png
```

### Convert a font to 2BPP format from a PNG file

```
ttf-2-bpp.exe some-font.png some-font.bin
```

Note: PNG input mode does not generate an encoding Yaml file. If you adjust the
width of the glyphs in the PNG file before reconverting, be sure to udpate the
widths in the encoding file to match.

## Glyph input

Glyphs for rendering may be provided in one  of two ways:

* From one or more Unicode character blocks.
* From a plaintext list of glyphs in a file.

If no other input options are provided, the program will attempt to read from
a plaintext "glyphs.txt" file if it is present.

## Configuration

The behavior of the program may be incluenced by a `config.yml` file present
in the working directory.

If no such file exists, it will be generated with a set of default options.

### Palette

A sequence defining what order the background, text, and text border colors 
should be assumed when geenrating the 2BPP data. Note that ttf-2-bpp does not
generate a palette, but this information is still required for setting the
correct color indexes. 

The list must contain these 4 entries in any non-repeating order:
* Unused
* Border
* Text
* Background

A few examples:

#### Background is color 4, Border is color 2, Text body is color 3

```yaml
Palette:
  - Unused
  - Border
  - Text
  - Background
```

#### Background is color 1, Text body is color 2, Border is color 3

```yaml
Palette:
  - Background
  - Text
  - Border
  - Unused
```

### FontExtension

File extension for the 2bpp data if no filename is provided.

The default is "smc" for Tile Layer Pro compatibility.

### EncodingFile

Name of the file to write the encoding information file to. If not specified,
it is chosen based on the name of the input file.

### GlyphIndexStart

Generated glyphs encodign data will start from this value.

i.e. if this value is 1, and the string "ABCDE" is encoded:
* A will have code 1
* B will have code 2
* C will have code 3
* D will have code 4
* E will have code 5

If not specified, the fedault value is 1.

### ReservedGlyphs

This field must be in the form of a YAML map. The key of the fields may be the
glyphs UTF8 representation, or it may be an alias.
 
Each entry in the map must have a "code" field defined. The code corresponds to
the position in the generated 2BPP data the glyph must be inserted in.
 
Each entry may also have a "glyph" field defined. If present, this will be used 
as the Unicode representation of the file when rendering the font. 

Example usage:
```yaml
ReservedGlyphs:
    " ":
        code: 2
    "[Space2]":
        code: 6
        glyph: " "
```

### Rendering Parameters

#### Baseline

Position from the top of the rendering area where the glyph baseline should 
be assumed.

#### AlphaThreshold

Threshold which determines whether a pixel will be colored as part of
the text border or main body.

#### BorderPointSize

Size in points (1/64th of a pixel) to use for rendering the text border.

#### GlyphWidth

With of the generated glyphs. Must be 8 or 16.

#### RenderWidth

Width to use to constrain rendering. Does not have to match the GlyphWidth.

## Building

### Build dependencies

* Freetype
* Boost Locale
* ICU
* Expat
* png++/libPNG
* [yaml-cpp](https://github.com/jbeder/yaml-cpp) 0.6.3
* [cxxopts](https://github.com/jarro2783/cxxopts)
* (Optional) [Boxer](https://github.com/aaronmjacobs/Boxer)

All dependencies except Boxer are provided on the Conan center.

Boxer may be exported locally using the files in `external/boxer`.

### Instructions

#### Exporting Boxer

* cd external/boxer
* conan install (options) .
* conan build .
* conan export-pkg .

#### Building

* conan install (options) .
* cd build

To build with Ninja:
* cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=Release/generators/conan_toolchain.cmake 
* ninja

To build with Make:
* cmake .. -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=Release/generators/conan_toolchain.cmake 
* make

