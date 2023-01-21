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

First make sure there is some display devices plugged into the board, otherwise the preview cannot find any appropriate device and ```crtc``` during the initiation. 

To add a display plane, you have to create a ```drm_buffer``` struct and manually fill in the required information. The preview can diaplay both dumb buffers and ```dma-buf```. If a dumb buffer is used, the preview will allocate the memory and export an userspace pointer. Different buffer informations are required for the two type of buffer. See the table below for details. 

| Variable     | type           | Dumb buffer                 | ```dma-buff```                |
| ------------ | -------------- | --------------------------- | ----------------------------- |
| fb_id        | ```uint32_t``` | N/A                         | File descriptor of the buffer |
| width        | ```uint32_t``` | Width of the frame          | Width of the frame            |
| height       | ```uint32_t``` | Height of the frame         | Height of the frame           |
| bpp          | ```uint32_t``` | Bytes per pixel             | N/A                           |
| pixel_format | ```uint32_t``` | ```DRM``` pixel fourcc code | ```DRM``` pixel fourcc code   |

The preview can also resize, shift, and crop the frame. This is achieved by setting the display parameters in the ```drm_buffer```. See the example for further details. 

After properly assign the buffer information, a new plane can be added to the preview by calling the ```addPlane``` function which returns a ```plane_id``` for future use. It will automatically find the DRM plane that supporting the pixel format. When using multiple planes, make sure to add the planes in order. The first added one will be at the bottom and etc. 

Finally, to show the new frame, just call function ```showPlane``` and pass the correct ```plane_id``` to it. 

## Example (dumb buffer)

```c++
#include <stdio.h>
#include <memory>
#include <drm_fourcc.h>
#include <preview.h>

int main ()
{
	drmPreview preview;
	int plane_id;

	// Create the frame buffer
	std::shared_ptr<drm_buffer> buffer_ = preview.makeBuffer();
	
	// Fill in the buffer information
	// Here we have a 200x200 image with ARGB format. 
	// As we are using the dumb buffer, the bpp needed to be manually set
	buffer_->width = 200;
	buffer_->height = 200;
	buffer_->bpp = 32;
	buffer_->pixel_format = DRM_FORMAT_ARGB8888;
	// We want the square to show at the center of the display and 
	// with half the maximum display size. 
	buffer_->display_offset_x = 0.25;
	buffer_->display_offset_y = 0.25;
	buffer_->display_size_x = 0.5;
	buffer_->display_size_y = 0.5;

	// Add the new plane
	plane_id = preview.addPlane(buffer_, DRM_DUMB_BUFFER);

	// Fill the frame buffer which will be fully white and update the display
	memset(buffer_->vaddr, 0xff, buffer_->size);
	preview.showPlane(plane_id);

	// Now we want to show the square at the bottom right corner with quarter
	// the size of the display
	getchar();
	buffer_->display_offset_x = 0.75;
	buffer_->display_offset_y = 0.75;
	buffer_->display_size_x = 0.25;
	buffer_->display_size_y = 0.25;
	preview.showPlane(plane_id); // update the display
	
	return 0;
}
```

