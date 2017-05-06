#ifndef MODULES_H
#define MODULES_H

struct nodedata
{
    struct vox_node *node;
};

struct cameradata
{
    struct vox_camera *camera;
    struct vox_camera_interface *iface;
};

struct cddata
{
    struct vox_cd *cd;
};

struct scancodedata
{
    const char *key;
    unsigned char code;
};

#endif
