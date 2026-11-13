#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>
#include <vector>

using namespace android;

status_t gbm_lock_compat(GraphicBufferMapper* self, buffer_handle_t handle, uint32_t usage,
                         const Rect& bounds, void** vaddr, int32_t* bpp, int32_t* bps)
    __asm__("_ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPvPiS9_");
status_t gbm_unlock_compat(GraphicBufferMapper* self, buffer_handle_t handle)
    __asm__("_ZN7android19GraphicBufferMapper6unlockEPK13native_handle");

__attribute__((visibility("default")))
status_t gbm_lock_compat(GraphicBufferMapper* self, buffer_handle_t handle, uint32_t usage,
                         const Rect& bounds, void** vaddr, int32_t* bpp, int32_t* bps) {
    auto r = self->lock(handle, static_cast<int64_t>(usage), bounds);
    if (!r.has_value()) return r.error().asStatus();
    if (vaddr) *vaddr = r.value().address;
    int32_t b = r.value().bytesPerPixel;
    int32_t st = r.value().bytesPerStride;
    if (b < 0 || st < 0) {
        std::vector<ui::PlaneLayout> layouts;
        if (self->getPlaneLayouts(handle, &layouts) == OK && !layouts.empty()) {
            b = static_cast<int32_t>(layouts[0].sampleIncrementInBits / 8);
            st = static_cast<int32_t>(layouts[0].strideInBytes);
        }
    }
    if (bpp) *bpp = b;
    if (bps) *bps = st;
    return OK;
}

__attribute__((visibility("default")))
status_t gbm_unlock_compat(GraphicBufferMapper* self, buffer_handle_t handle) {
    return self->unlock(handle, nullptr);
}
