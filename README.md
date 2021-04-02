# Video Coding Utilities (VCU)
Collection of C++, Python and Matlab/GNU Octave software for different algorithms and methods used in video coding research.


## Requirements
Matlab/GNU Octave scripts run with any recent version of both environments and do not require any specific toolbox for Matlab (or package for GNU Octave).

C++ software requires a compiler compliant with C++14 or above. The development used MS Visual Studio 2019 as IDE and toolchain and has been been tested using Ubuntu 18.14 LTS under WSL with g++ 7.5.0

Python software uses Python 3 and the required packages are listed in the `requirements.txt` file. To install them with `pip` use:
```bash
> pip install -r requirements.txt
```

## Available tools
VCU provides you with a suite of utilities to be in video coding research, spanning from error resilient video coding to image and video objective quality assessment. Part of the software provided in this git repository was originally available through my personal [webpage](https://sites.google.com/site/matteonaccari/home). Under the software section you might find some additional tools which are not part of this repository which I'm not planning to maintain anymore. The software provide here is divided in the following categories:
 * **error-resilience**: Tools to create error corruption patterns and simulate the transmission of H.264/AVC and H.265/HEVC bistreams over error prone channels based on packet switched networks.
 * **image-processing**: Basic methods to perform simple edge detection, add rining noise and quantify the complexity of the content portrayed using the ITU-R P.910 spatial and temporal information.
 * **image-qa**: A collection of objective metrics to quantify artifacts such as blurrin, ringing and blocking.
 * **transform-coding**: Implementation of some fundamental processing associated with the Discrete Cosine Transform (DCT), its integer arithmetic approximation as specified in the H.264/AVC standard and the quarter pixel upsampling filter used by the same standard to perform motion compensation.

 ## Usage
The software provided by this repository is intended for research and/or teaching purposes. Its interface is believed to be self-explanatory and can be modified/extended to fit your needs.

## License
Please note the copyright and disclaimer statement attached to each file

## Contributing
Please submit pull requests for bug fixes, features addition and any improvement you may want to contribute to.