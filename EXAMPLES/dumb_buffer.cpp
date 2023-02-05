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