/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2020, Raspberry Pi (Trading) Ltd.
 *
 * drm_preview.cpp - DRM-based preview window.
 */

#include <drm.h>
#include <drm_fourcc.h>
#include <drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sys/mman.h>
#include <preview.h>

#define ERRSTR strerror(errno)

static int drm_set_property(int fd, int plane_id, char const *name, char const *val)
{
    drmModeObjectPropertiesPtr properties = nullptr;
    drmModePropertyPtr prop = nullptr;
    int ret = -1;
    properties = drmModeObjectGetProperties(fd, plane_id, DRM_MODE_OBJECT_PLANE);

    for (unsigned int i = 0; i < properties->count_props; i++)
    {
        int prop_id = properties->props[i];
        prop = drmModeGetProperty(fd, prop_id);
        if (!prop)
            continue;

        if (!drm_property_type_is(prop, DRM_MODE_PROP_ENUM) || !strstr(prop->name, name))
        {
            drmModeFreeProperty(prop);
            prop = nullptr;
            continue;
        }

        // We have found the right property from its name, now search the enum table
        // for the numerical value that corresponds to the value name that we have.
        for (int j = 0; j < prop->count_enums; j++)
        {
            if (!strstr(prop->enums[j].name, val))
                continue;

            ret = drmModeObjectSetProperty(fd, plane_id, DRM_MODE_OBJECT_PLANE, prop_id, prop->enums[j].value);
            if (ret < 0)
                std::cerr << "DrmPreview: failed to set value " << val << " for property " << name << std::endl;
            goto done;
        }

        std::cerr << "DrmPreview: failed to find value " << val << " for property " << name << std::endl;
        goto done;
    }

    std::cerr << "DrmPreview: failed to find property " << name << std::endl;
done:
    if (prop)
        drmModeFreeProperty(prop);
    if (properties)
        drmModeFreeObjectProperties(properties);
    return ret;
}

int getFBInfo(std::shared_ptr<drm_buffer> buffer_, uint32_t offsets[], uint32_t pitches[], uint32_t bo_handles[])
{
    uint32_t pixel_format = buffer_->pixel_format;
    int stride = buffer_->width;
    int height = buffer_->height;
    int width = buffer_->width;

    if (pixel_format == DRM_FORMAT_YUV420)
    {
        offsets[0] = 0; offsets[1] = stride * height; offsets[2] = stride * height + (stride / 2) * (height / 2);
        pitches[0] = stride; pitches[1] = stride / 2; pitches[2] = stride / 2;
        bo_handles[0] = buffer_->handle; bo_handles[1] = buffer_->handle; bo_handles[2] = buffer_->handle;
        return 0;
    }

    if (pixel_format == DRM_FORMAT_ABGR8888)
    {
        offsets[0] = 0;
        pitches[0] = stride;
        bo_handles[0] = buffer_->handle ;
        return 0;
    }

    return 1;
}

drmPreview::drmPreview()
{   
    verbose = 1; // TODO
    
    // Open the drm device
    drmfd = drmOpen("vc4", NULL);
    // drmfd_ = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (drmfd < 0)
        throw std::runtime_error("drmOpen failed: " + std::string(ERRSTR));
    if (!drmIsMaster(drmfd))
        throw std::runtime_error("DRM preview unavailable - not master");

    try
    {
        con_id = 0;
        findCrtc();
    }
    catch (std::exception const &e)
    {
        throw;
    }
}

drmPreview::~drmPreview()
{

}

void drmPreview::findCrtc()
{
    unsigned int i;
    
    drmModeRes *res = drmModeGetResources(drmfd);
    if (!res)
        throw std::runtime_error("drmModeGetResources failed: " + std::string(ERRSTR));
    if (res->count_crtcs <= 0)
        throw std::runtime_error("drm: no crts found.");

    // Find connectors
    if (!con_id)
    {
        if (verbose)
            std::cerr << "No connector ID specified.  Choosing default from list:" << std::endl;

        for (i = 0; i < res->count_connectors; i++)
        {
            drmModeConnector *con = drmModeGetConnector(drmfd, res->connectors[i]);
            drmModeEncoder *enc = NULL;
            drmModeCrtc *crtc = NULL;

            if (con->encoder_id)
            {
                enc = drmModeGetEncoder(drmfd, con->encoder_id);
                if (enc->crtc_id)
                {
                    crtc = drmModeGetCrtc(drmfd, enc->crtc_id);
                }
            }

            if (!con_id && crtc)
            {
                con_id = con->connector_id;
                crtc_id = crtc->crtc_id;
                conn = con; // TODO: THIS IS FOR ADDING DUMB BUFFER ONLY (used in the drmModeSetCrtc())
            }

            if (crtc)
            {
                display_width = crtc->width;
                display_height = crtc->height;
            }

            if (verbose)
                std::cerr << "Connector " << con->connector_id << " (crtc " << (crtc ? crtc->crtc_id : 0) << "): type "
                          << con->connector_type << ", " << (crtc ? crtc->width : 0) << "x" << (crtc ? crtc->height : 0)
                          << (con_id == (int)con->connector_id ? " (chosen)" : "") << std::endl;
        }

        if (!con_id)
            throw std::runtime_error("No suitable enabled connector found");
    }

    // Find CRTC
    int crtcIdx_ = -1;

    for (i = 0; i < res->count_crtcs; ++i)
    {
        if (crtc_id == res->crtcs[i])
        {
            crtcIdx_ = i;
            break;
        }
    }

    if (crtcIdx_ == -1)
    {
        drmModeFreeResources(res);
        throw std::runtime_error("drm: CRTC " + std::to_string(crtc_id) + " not found");
    }

    if (res->count_connectors <= 0)
    {
        drmModeFreeResources(res);
        throw std::runtime_error("drm: no connectors");
    }

    drmModeConnector *c;
    c = drmModeGetConnector(drmfd, con_id);
    if (!c)
    {
        drmModeFreeResources(res);
        throw std::runtime_error("drmModeGetConnector failed: " + std::string(ERRSTR));
    }

    if (!c->count_modes)
    {
        drmModeFreeConnector(c);
        drmModeFreeResources(res);
        throw std::runtime_error("connector supports no mode");
    }

    drmModeCrtc *crtc = drmModeGetCrtc(drmfd, crtc_id);
    drmModeFreeCrtc(crtc);
}

int drmPreview::findPlane(unsigned int fourcc)
{
    unsigned int i, j;

    drmModePlaneResPtr planes;
    drmModePlanePtr plane;

    planes = drmModeGetPlaneResources(drmfd);
    if (!planes)
        throw std::runtime_error("drmModeGetPlaneResources failed: " + std::string(ERRSTR));

    try
    {
        int top_plane_id = 0;
        top_plane_id = buffers_.size();
        if (top_plane_id >= planes->count_planes)
            throw std::runtime_error("Maximum plane number reached!");

        for (i = top_plane_id; i < planes->count_planes; ++i)
        {
            plane = drmModeGetPlane(drmfd, planes->planes[i]);

            if (!planes)
                throw std::runtime_error("drmModeGetPlane failed: " + std::string(ERRSTR));

            if (!(plane->possible_crtcs & (crtc_id)))
            {
                drmModeFreePlane(plane);
                continue;
            }

            for (j = 0; j < plane->count_formats; ++j)
            {
                if (plane->formats[j] == fourcc)
                {
                    break;
                }
            }

            if (j == plane->count_formats)
            {
                drmModeFreePlane(plane);
                continue;
            }

            drmModeFreePlane(plane);

            return plane->plane_id;
        }
    }
    catch (std::exception const &e)
    {
        drmModeFreePlaneResources(planes);
        throw;
    }

    drmModeFreePlaneResources(planes);
    return -1;
}

std::shared_ptr<drm_buffer> drmPreview::makeBuffer()
{
    std::shared_ptr<drm_buffer> buffer_ = std::make_shared<drm_buffer>();

    buffer_->width = 0;
    buffer_->height = 0;

    if (verbose)
        std::cout << "Generating new buffer" << std::endl;

    return buffer_;
}

int drmPreview::addPlane(std::shared_ptr<drm_buffer> buffer_, buffer_type bo_type)
{
    int plane_id = -1;

    plane_id = findPlane(buffer_->pixel_format);
    
    if (plane_id == -1)
        throw std::runtime_error("No plane found");

    buffer_->crtc_x = (uint32_t) display_width  * buffer_->display_x;
    buffer_->crtc_y = (uint32_t) display_height * buffer_->display_y;
    buffer_->crtc_w = (uint32_t) display_width  * buffer_->display_w;
    buffer_->crtc_h = (uint32_t) display_height * buffer_->display_h;

    buffer_->src_x = (uint32_t) buffer_->width  * buffer_->roi_x;
    buffer_->src_y = (uint32_t) buffer_->height * buffer_->roi_y;
    buffer_->src_w = (uint32_t) buffer_->width  * buffer_->roi_w;
    buffer_->src_h = (uint32_t) buffer_->height * buffer_->roi_h;

    buffer_->pixel_format = buffer_->pixel_format;

    buffers_[plane_id] = buffer_;

    if (verbose)
        std::cout << "Plane (" << std::to_string(plane_id) << ")" << std::endl;

    if (bo_type == DRM_DUMB_BUFFER)
    {
        if (verbose)
            std::cout << "Adding a dumb buffer to plane (" << std::to_string(plane_id) << ")" << std::endl;
        addDumbBuffer(plane_id);
        return plane_id;
    }
    if (bo_type == DRM_PRIME_BUFFER)
    {
        if (verbose)
            std::cout << "Adding a prime buffer to plane (" << std::to_string(plane_id) << ")" << std::endl;
        addPrimeBuffer(plane_id);
        return plane_id;
    }

    return 0;
}

void drmPreview::addDumbBuffer(int plane_id)
{
    // TODO: ADD SOME PRINTOUT OF THE BUFFER INFORMATION
    if (verbose)
        std::cout << "Adding dumb buffer to plane (" << std::to_string(plane_id) << ")" << std::endl;
    
    std::shared_ptr<drm_buffer> buf_ = buffers_[plane_id];

    struct drm_mode_create_dumb create = {};
    struct drm_mode_map_dumb map = {};

    // Create a dumb buffer
    create.width  = buf_->width;
    create.height = buf_->height;
    create.bpp = buf_->bpp;
    drmIoctl(drmfd, DRM_IOCTL_MODE_CREATE_DUMB, &create);

    // Add the dumb buffer
    buf_->pitch  = create.pitch;
    buf_->size   = create.size;
    buf_->handle = create.handle;
    if (drmModeAddFB(drmfd, buf_->width, buf_->height, buf_->bpp, buf_->bpp, buf_->pitch, buf_->handle, &buf_->fb_id))
        throw std::runtime_error("drmModeAddFB2 failed: " + std::string(ERRSTR));

    // Map buffer to userspace address
    map.handle = buf_->handle;
    drmIoctl(drmfd, DRM_IOCTL_MODE_MAP_DUMB, &map);
    buf_->vaddr = mmap(NULL, create.size, PROT_READ | PROT_WRITE, MAP_SHARED, drmfd, map.offset);

    // Set controller
    drmModeSetCrtc(drmfd, crtc_id, buf_->fb_id, 0, 0, (uint32_t*) &con_id, 1, &conn->modes[0]);

}

void drmPreview::addPrimeBuffer(int plane_id)
{
    std::shared_ptr<drm_buffer> buf_ = buffers_[plane_id];

    if (drmPrimeFDToHandle(drmfd, buf_->fb_id, &buf_->handle))
        throw std::runtime_error("drmPrimeFDToHandle failed for fd " + std::to_string(buf_->fb_id));

    int stride = buf_->width; // TODO: ASSIGN PROPER STRIDE IN MIPI CAMERA. IT'S IN STREAM INFO
    int height = buf_->height;
    int width  = buf_->width;

    uint32_t offsets[4];
    uint32_t pitches[4];
    uint32_t bo_handles[4];

    if (getFBInfo(buf_, offsets, pitches, bo_handles))
        throw std::runtime_error("getFBInfo failed: " + std::string(ERRSTR));

    if (drmModeAddFB2(drmfd, width, height, buf_->pixel_format, bo_handles, pitches, offsets, &buf_->handle, 0))
        throw std::runtime_error("drmModeAddFB2 failed: " + std::string(ERRSTR));

}

void drmPreview::showPlane(int plane_id)
{
    // TODO: CUSTOM DEFINED BUFFER CROP
    
    std::shared_ptr<drm_buffer> buf_ = buffers_[plane_id];

    drmModeSetPlane(drmfd, plane_id, crtc_id, buf_->fb_id, 0,
        buf_->crtc_x, buf_->crtc_y, buf_->crtc_w, buf_->crtc_h,
        0, 0, buf_->width << 16, buf_->height << 16);
}
