# opencv-calibrator

A little camera calibration utility using OpenCV.

## Building

You'll need to have OpenCV installed in such a way that CMake can find it. Then just do:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

There are two programs: `opencv-calibrate` and `undistort`. `opencv-calibrate` is used to produce a calibration file;
`undistort` allows you to preview the effects of said calibration file before using it in another project.

Usage for `opencv-calibrate`:
```
opencv-calibrate [OPTIONS] [IMAGE1 [IMAGE2 ... ]]   
  -h            Show this help message
  -s            Show images while processing
  -c ID         Capture images from camera ID rather than command line
  -n NUM        Capture NUM images before running
  -x WIDTH      Calibration chessboard width
  -y HEIGHT     Calibration chessboard height
  -e NUM        Calibration chessboard square size (arb. units)
  -o FILENAME   Name of the file to save the calibration into; defaults
                to 'calibration.yaml'
```

Usage for `undistort`:

```
undistort [OPTIONS] [IMAGE1 [IMAGE2 ... ]]
  -h            Display this help message and exit
  -f FILE       File to load the camera calibration from (default
                'calibration.yaml')
  -c ID         Camera to view live. Using this option ignores
                any images provided on the command line and simply
                shows the live feed.
```
