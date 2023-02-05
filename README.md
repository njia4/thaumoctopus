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
	// Add the new plane
	plane_id = preview.addPlane(buffer_, DRM_DUMB_BUFFER); // Add a new plane
	// Fill the frame buffer which will be fully white and update the display
	memset(buffer_->vaddr, 0xff, buffer_->size);

	// Display the whole frame
	buffer_->roi_x = 0;
	buffer_->roi_y = 0;
	buffer_->roi_w = 1;
	buffer_->roi_h = 1;

	// We want the square to show at the center of the display and 
	// with half the maximum display size. 
	buffer_->display_x = 0.25;
	buffer_->display_y = 0.25;
	buffer_->display_w = 0.5;
	buffer_->display_h = 0.5;
	preview.setPlaneSizes(buffer_); // This is not necessary. addPlane() call this internally
	preview.showPlane(plane_id); // Display the plane
	getchar();

	// Now we want to show the square at the bottom right corner with 1/8
	// the size of the display
	buffer_->display_x = 0.75;
	buffer_->display_y = 0.75;
	buffer_->display_w = 0.25;
	buffer_->display_h = 0.25;
	preview.setPlaneSizes(buffer_); // This is required to calculate the display parameters
	preview.showPlane(plane_id); // update the display
	getchar();
	
	return 0;
}
```

