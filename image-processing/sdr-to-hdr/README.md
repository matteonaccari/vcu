# Standard to high dynamic range conversion
The software included in this directory performs a dynamic range conversion by putting a Standard Dynamic Range (SDR) image/video into its High Dynamic Range (HDR) counterpart assuming the use of the BT.2100 Hybrid Log-Gamma (HLG) transfer characteristics.

In particular, the process described in Section 5.2.1 of the [ITU-R report BT.2408](https://www.itu.int/dms_pub/itu-r/opb/rep/R-REP-BT.2408-4-2021-PDF-E.pdf) is implemented, using the simplification described in Section 5.1.2.4 of the same report. The conversion merely places an SDR content into an HDR-HLG one, assuming that 100% of the SDR signal is mapped into 75% HDR. Accordingly, no re-grading is performed here. More information is also available from the following study presented at the [7th meeting of the Joint Video Experts Team (JVET)](https://jvet-experts.org/doc_end_user/documents/7_Torino/wg11/JVET-G0059-v2.zip).

Two implementations are offered, as briefly summarised in the following.

## Python script
The script assumes input signals in the YCbCr colour space with ITU-R BT.709 primaries, with 4:4:4 chroma format and in the video range. Input bit depths of 8 and 10 bits are supported. The script requires the `numpy` package which is listed in the `requirement.txt` files provided in the root of the **VCU** repository. An example of usage may be the following:
```bash
py sdr2hdr.py --input sdr_file.yuv --output hdr_file.yuv --height 1080 --width 1920
```
This will apply the SDR to HDR conversion over all frames of the `sdr_file.yuv` input.

## Git patch for FFmpeg
This patch can be applied on top of the master branch of [FFmpeg](https://github.com/FFmpeg/FFmpeg) and will introduce an AVFilter that performs the SDR to HDR conversion. The main advantage of this implementation is that several video formats can be used (compressed and uncompressed ones) without the need to convert the content to the YCbCr planar format and then operate with the Python implementation. The patch also includes a short documentation to activate the filter from the FFmpeg command line interface. Please note that the filter assumes the same chroma format (4:4:4) as input and 10 bits per pixel. FFmpeg's will internally convert all frames of the incoming video which do not meet these requirement before calling the dynamic range conversion. Essentially, the filter is triggered with:
```bash
ffmpeg -i <input_content> -vf sdr2hdr output_file
```

It is believed that the patch can be applied on top of the `master` branch of the FFmpeg repository without particular conflicts. The patch will be updated on a regular basis to track the FFmpeg's repo evolution. In case of conflicts, I'd be happy to assist you and solve them.