# Thaumoctopus
A lightweight DRM based preview. 

## Motivation

I was working on a hobby project to overlay images from a visible light camera with a thermal camera using a Raspi Zero. Due to the limited resource available on an RPi Zero, using an X server is difficult and laggy. So I build this simple library to display images with little overhead. It works just fine for me. 

## Installation

I have only tested this on a Raspi Zero W board with bullseye (32bits), so it is possible extra steps are needed. 

* Download the package

  ```shell
  git clone https://github.com/njia4/thaumoctopus
  ```

* Install required packages (`drm` and `glog`)

  ```shell
  sudo apt-get update
  sudo apt-get install libdrm-dev
  sudo apt-get install libgoogle-glog-dev
  ```

* Make 

  ```shell
  mkdir build && cd build
  cmake ..
  make
  ```

* Install the package (optional)

  ```shell
  make install
  ```

## Usage

First make sure there is some display devices plugged into the board, otherwise the preview cannot find any appropriate device and ```crtc``` during the initiation. 

To add a display plane, you have to create a ```drm_buffer``` struct and manually fill in the required information. The preview can diaplay both dumb buffers and ```dma-buf```, and they require slightly different parameters. 

The size (`width` and `height`) of the frame, ROI, and display parameters are required for both types of buffers. The ROI and display parameter are float numbers from 0 to 1, defined relative to the full size of the frame and screen size respectively. 

The pixel format is defined differently between the two buffer types. For dumb buffer, you have to define the `bpp` (bits per pixel) variable. The prime buffer can take a Fourcc code and calculate the `bpp` automatically. 

Finally, to show the new frame, just call function ```showPlane``` and pass the correct ```plane_id``` to it. You can adjust the ROI and display parameters on the fly, but remember to call `setPlaneSizes` function to recalculated the display parameters. 

See the [sample code](./EXAMPLES/dumb_buffer.cpp) for a simple implementation of dumb buffer, where the offsets and cropping is changed during the run. 
