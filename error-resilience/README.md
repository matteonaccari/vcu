# Tools for error reslient video coding experiments
Here you can find some utilities I wrote when researching on error resilient video coding in my PhD studies. If you need to generate error patterns according to a two state Gilbert distribution and/or corrupt an H.264/AVC or H.265/HEVC bistream to simulate its transmission over an error prone channel, don't look any further (as Dennis Edgwards used to sing in 1984).

## Tools available
 * **burst-counter**: Counts the number of contiguous packet losses (bursts in the jargon) occur in a given error patter file.
 * **gilbert-model**: Generates an error pattern file, i.e. a plain text file where characters `1` are associated with the loss of a video packet (e.g. a slice as specified by the H.264/AVC or H.265/HEVC standards). The pattern generated follows a two stage Gilbert model which is particularly suitable to model the loss of packets transmitted over IP networks.
 * **transmitter-simulator-avc**: A high level parser for bitstreams complaint with the H.264/AVC standard which drops packets according to an error pattern. See the pdf manual which ships with the subdirectory for more information.
 * **transmitter-simulator-hevc**: A high level parser for bitstreams complaint with the H.265/HEVC standard which drops packets according to an error pattern. See the pdf manual which ships with the subdirectory for more information.