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

#include <iostream>
#include <string>
#include <functional>
#include <stdexcept>
#include <map>
#include <cstring>
#include <memory>

// #include <buffer.h>
// #include <glog/logging.h>

// For StreamInfo
// TODO: REPLACE IT WITH HARD CODED ONES
// #include <optional>

enum buffer_type
    {
        DRM_DUMB_BUFFER,
        DRM_PRIME_BUFFER
    };

struct drm_buffer {
    uint32_t fb_id;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t handle;
    uint32_t size;
    uint32_t bpp;
    uint32_t pixel_format;
    void *vaddr;

    float display_x;
    float display_y;
    float display_w;
    float display_h;
    uint32_t crtc_x;
    uint32_t crtc_y;
    uint32_t crtc_w;
    uint32_t crtc_h;

    float roi_x;
    float roi_y;
    float roi_w;
    float roi_h;
    uint32_t src_x;
    uint32_t src_y;
    uint32_t src_w;
    uint32_t src_h;
};

static int drm_set_property(int fd, int plane_id, char const *name, char const *val);

class drmPreview
{
public:
    drmPreview();
    ~drmPreview();
    std::shared_ptr<drm_buffer> makeBuffer();
    int addPlane(std::shared_ptr<drm_buffer> buffer_, buffer_type type);
    void showPlane(int plane_id);
    void reset() {};
    void debug(int debug) { verbose = debug; };

private:
    void findCrtc();
    int findPlane(unsigned int fourcc);
    void addDumbBuffer(int plane_id);
    void addPrimeBuffer(int plane_id);

    int verbose = 0;
    int drmfd;
    int con_id;
    int crtc_id;
    unsigned int display_width;
    unsigned int display_height;
    std::map<int, std::shared_ptr<drm_buffer>> buffers_;

    drmModeConnector *conn; // TODO: THIS IS FOR ADDING THE DUMB BUFFER, MAYBE GET THE OBJECT BY IT'S ID
};
