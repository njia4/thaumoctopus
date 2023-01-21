# Thaumoctopus
A lightweight DRM based preview. 

## Description

This is a very simple c++ library for displaying and overlaying frames obtained from a camera or a video stream *without* an X server. I developed it so I could have a camera preview and some overlay images with transparency on the Raspberry Pi Zero board. The limited resources on a Pi Zero make it difficult to run any GUI windows, and overlay with transpanrency using ```GStreamer``` is also pretty costy for some reason I don't understand. So I built this little piece of code to do a light weight display and overlay using ```libdrm```. 

## Installation

I have only tested this on a Raspi Zero W board with XXX, so it is possible extra steps are needed. 

* Download the package

  ```shell
  git clone https://github.com/njia4/thaumoctopus
  ```

* You should already have ```libdrm``` on Raspi OS. In case this is not true, you can install it using

  ```shell
  sudo apt-get update
  sudo apt-get install libdrm-dev
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

First make sure there is some display devices plugged into the board, otherwise the preview cannot find any appropriate device and ```crtc```. 

To add a display plane, you have to create a ```drm_buffer``` struct and manually fill in the required information. The preview can diaplay both dumb buffers and ```dma-buf```. If a dumb buffer is used, the preview will allocate the memory and export an userspace pointer. Different buffer informations are required for the two type of buffer. See the table below for details. 

| Variable     | type           | Dumb buffer         | ```dma-buff```                |
| ------------ | -------------- | ------------------- | ----------------------------- |
| fb_id        | ```uint32_t``` | N/A                 | File descriptor of the buffer |
| width        | ```uint32_t``` | Width of the frame  | Width of the frame            |
| height       | ```uint32_t``` | Height of the frame | Height of the frame           |
| bpp          | ```uint32_t``` | Bytes per pixel     | N/A                           |
| pixel_format | ```uint32_t``` | N/A                 | ```DRM``` pixel fourcc code   |

The preview can also resize, shift, and crop the frame. This is achieved by setting the display parameters in the ```drm_buffer```. See the example for further details. 

## Example (dumb buffer)

