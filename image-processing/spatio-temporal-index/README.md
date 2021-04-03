# Spatial and temporal information for a video sequence
=============

The following software computes the spatial and temporal information as specified in the ITU-T Recommendation P.910: ["Subjective video quality assessment methods for multimedia applications"](https://www.itu.int/rec/T-REC-P.910-200804-I/en). Four implementations are offered, as briefly summarised in the following.

## Stand alone software
Written in C++, the software deals with planar YCbCr files with different chroma formats (4:2:0, 4:2:2 and 4:4:4) and either 8 or 10 bits per pixel. Input files can also be provided in the RGB colour space where the [ITU-R BT.709](https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.709-6-201506-I!!PDF-E.pdf) colour primaries are assumed when the RGB to YCbCr transformation is applied. The software builds under Windows and Linux operative systems. For Windows, VS 2019 has been used, whilst for Linux g++ 7.5.0 is used to build the software. A VS 2019 solution is included for development under Windows whereas a rudimentary makefile is available for Linux.

## Python script
The script also assumes planar YCbCr files with different chroma formats (4:2:0, 4:2:2 and 4:4:4) and either 8 or 10 bits per pixel. Input files can also be provided in the RGB colour space where the [ITU-R BT.709](https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.709-6-201506-I!!PDF-E.pdf) colour primaries are assumed when the RGB to YCbCr transformation is applied. The script requires the `numpy` and `scipy` packages which are listed in the `requirement.txt` files provided in the root of the **VCU** repository.

## Matlab and GNU Octave script
This implementation runs under Matlab or GNU Octave and doesn't require any specific toolbox. The same assumptions as per its Python's counterpart above are used.

## Git patch for FFmpeg
This patch can be applied on top of the master branch of [FFmpeg](https://github.com/FFmpeg/FFmpeg) and will introduce an AVFilter that computes the spatial and temporal information features. The main advantage of this implementation is that several video formats can be used (compressed and uncompressed ones) without the need to convert the content to the YCbCr planar format. The patch also includes a short documentation to activate the filter from the FFmpeg command line interface. Essentially, the filter is triggered with:
```bash
ffmpeg -i <input_content> -vf spatiotemporalinfo='stats_file=<my_file>.csv' -f null dev/null
```

Where `my_file.csv` is the comma separated value file containing the spatial and temporal information on a frame basis.

It is believed that the patch can be applied on top of the `master` branch of the FFmpeg repository without particular conflicts. The patch will be updated on a regular basis to track the FFmpeg's repo evolution. In case of conflicts, I'd be happy to assist you and solve them.