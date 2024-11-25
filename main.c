#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "include/manager.h"
void check_error(int result, const char *msg) {
  if (result < 0) {
    fprintf(stderr, "%s: %s\n", msg, strerror(-result));
    exit(EXIT_FAILURE);
  }
}

int drm_fd;
drmModeRes *res;
drmModeConnector *conn;
drmModeModeInfo mode;

uint32_t *fb_ptr;
unsigned long long dumbSize = 0;
uint32_t fb;
uint32_t crtc_id = 0;
drmModeCrtc *crtc = NULL;

int main() {
  // Open the DRM device
  drm_fd = open("/dev/dri/card2", O_RDWR | O_CLOEXEC);
  if (drm_fd < 0) {
    perror("Cannot open /dev/dri/card0");
    return EXIT_FAILURE;
  }
  printf("DRM device opened successfully.\n");

  // Get DRM resources
  res = drmModeGetResources(drm_fd);
  if (!res) {
    fprintf(stderr, "Cannot get DRM resources: %s\n", strerror(errno));
    close(drm_fd);
    return EXIT_FAILURE;
  }
  printf("DRM resources retrieved.\n");

  // Find a connected connector
  conn = NULL;
  for (int i = 0; i < res->count_connectors; i++) {
    conn = drmModeGetConnector(drm_fd, res->connectors[i]);
    if (conn && conn->connection == DRM_MODE_CONNECTED &&
        conn->count_modes > 0) {
      printf("Connected display found on connector %d.\n", res->connectors[i]);
      break;
    }
    drmModeFreeConnector(conn);
    conn = NULL;
  }
  if (!conn) {
    fprintf(stderr, "No connected connector found.\n");
    drmModeFreeResources(res);
    close(drm_fd);
    return EXIT_FAILURE;
  }

  // Automatically select the first available display mode for width and height
  mode = conn->modes[0];
  int width = mode.hdisplay;
  int height = mode.vdisplay;
  printf("Selected display mode: %dx%d\n", width, height);

  // Find a compatible CRTC
  for (int i = 0; i < res->count_crtcs; i++) {
    crtc = drmModeGetCrtc(drm_fd, res->crtcs[i]);
    if (crtc) {
      crtc_id = crtc->crtc_id;
      printf("Using CRTC with ID: %d\n", crtc_id);
      break;
    }
  }
  if (!crtc) {
    fprintf(stderr, "No valid CRTC found for connector.\n");
    drmModeFreeConnector(conn);
    drmModeFreeResources(res);
    close(drm_fd);
    return EXIT_FAILURE;
  }

  // Create a Dumb Buffer
  struct drm_mode_create_dumb create_dumb = {0};
  create_dumb.width = width;
  create_dumb.height = height;
  create_dumb.bpp = 32;
  int ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb);
  if (ret < 0) {
    fprintf(stderr, "Cannot create dumb buffer: %s\n", strerror(errno));
    drmModeFreeCrtc(crtc);
    drmModeFreeConnector(conn);
    drmModeFreeResources(res);
    close(drm_fd);
    return EXIT_FAILURE;
  }
  printf("Dumb buffer created: size=%d bytes\n", create_dumb.size);

  // Create a framebuffer
  ret = drmModeAddFB(drm_fd, width, height, 24, 32, create_dumb.pitch,
                     create_dumb.handle, &fb);
  if (ret) {
    fprintf(stderr, "Cannot create framebuffer: %s\n", strerror(errno));
    drmModeFreeCrtc(crtc);
    drmModeFreeConnector(conn);
    drmModeFreeResources(res);
    close(drm_fd);
    return EXIT_FAILURE;
  }
  printf("Framebuffer added.\n");

  // Map the Dumb Buffer
  struct drm_mode_map_dumb map_dumb = {0};
  map_dumb.handle = create_dumb.handle;
  ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb);
  if (ret) {
    fprintf(stderr, "Cannot map dumb buffer: %s\n", strerror(errno));
    drmModeRmFB(drm_fd, fb);
    drmModeFreeCrtc(crtc);
    drmModeFreeConnector(conn);
    drmModeFreeResources(res);
    close(drm_fd);
    return EXIT_FAILURE;
  }

  // Map framebuffer to memory
  fb_ptr = mmap(0, create_dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd,
                map_dumb.offset);
  if (fb_ptr == MAP_FAILED) {
    fprintf(stderr, "Cannot mmap framebuffer: %s\n", strerror(errno));
    drmModeRmFB(drm_fd, fb);
    drmModeFreeCrtc(crtc);
    drmModeFreeConnector(conn);
    drmModeFreeResources(res);
    close(drm_fd);
    return EXIT_FAILURE;
  }
  printf("Framebuffer memory mapped.\n");

  dumbSize = create_dumb.size;

  updateDisplay();
  startStep2(fb_ptr,width,height);

  while(running) {loop();};

      // Cleanup
      drmModeSetCrtc(drm_fd, crtc->crtc_id, crtc->buffer_id, 0, 0,
     &conn->connector_id, 1, &crtc->mode); drmModeFreeCrtc(crtc);
      drmModeFreeConnector(conn);
      drmModeFreeResources(res);
      munmap(fb_ptr, create_dumb.size);

      struct drm_mode_destroy_dumb destroy_dumb = {0};
      destroy_dumb.handle = create_dumb.handle;
      drmIoctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_dumb);

      close(drm_fd);
      printf("Resources cleaned up and DRM device closed.\n");
  
  return EXIT_SUCCESS;
}

void updateDisplay() {

  int ret =
      drmModeSetCrtc(drm_fd, crtc_id, fb, 0, 0, &conn->connector_id, 1, &mode);
  if (ret) {
    fprintf(stderr, "Cannot set CRTC: %s\n", strerror(errno));
    munmap(fb_ptr, dumbSize);
    drmModeRmFB(drm_fd, fb);
    drmModeFreeCrtc(crtc);
    drmModeFreeConnector(conn);
    drmModeFreeResources(res);
    close(drm_fd);
  }
}
