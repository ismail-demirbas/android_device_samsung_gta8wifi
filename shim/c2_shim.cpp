#include <memory>
#include <new>
#include <C2BufferPriv.h>
#include <C2PlatformStorePluginLoader.h>

extern "C++" {

void c2_pool_ctor_compat(C2PooledBlockPool* self, const std::shared_ptr<C2Allocator>& a,
                         uint64_t id)
    __asm__("_ZN17C2PooledBlockPoolC1ERKNSt3__110shared_ptrI11C2AllocatorEEm");
c2_status_t c2_create_allocator_compat(C2PlatformStorePluginLoader* self, uint32_t id,
                                       std::shared_ptr<C2Allocator>* out)
    __asm__("_ZN27C2PlatformStorePluginLoader15createAllocatorEjPNSt3__110shared_ptrI11C2AllocatorEE");
c2_status_t c2_create_blockpool_compat(C2PlatformStorePluginLoader* self, uint32_t aid,
                                       uint64_t bid, std::shared_ptr<C2BlockPool>* out)
    __asm__("_ZN27C2PlatformStorePluginLoader15createBlockPoolEjmPNSt3__110shared_ptrI11C2BlockPoolEE");

__attribute__((visibility("default")))
void c2_pool_ctor_compat(C2PooledBlockPool* self, const std::shared_ptr<C2Allocator>& a,
                         uint64_t id) {
    new (self) C2PooledBlockPool(a, id, C2PooledBlockPool::VER_HIDL);
}

__attribute__((visibility("default")))
c2_status_t c2_create_allocator_compat(C2PlatformStorePluginLoader* self, uint32_t id,
                                       std::shared_ptr<C2Allocator>* out) {
    return self->createAllocator(id, out);
}

__attribute__((visibility("default")))
c2_status_t c2_create_blockpool_compat(C2PlatformStorePluginLoader* self, uint32_t aid,
                                       uint64_t bid, std::shared_ptr<C2BlockPool>* out) {
    return self->createBlockPool(aid, bid, out);
}

std::shared_ptr<C2Buffer> c2_linbuf_old(void* self, const std::shared_ptr<C2LinearBlock>& b)
    __asm__("_ZN7android17SimpleC2Component18createLinearBufferERKNSt3__110shared_ptrI13C2LinearBlockEE");
std::shared_ptr<C2Buffer> c2_linbuf_new(void* self, const std::shared_ptr<C2LinearBlock>& b,
                                        size_t o, size_t s)
    __asm__("_ZN7android17SimpleC2Component18createLinearBufferERKNSt3__110shared_ptrI13C2LinearBlockEEmm");

__attribute__((visibility("default")))
std::shared_ptr<C2Buffer> c2_linbuf_old(void* self, const std::shared_ptr<C2LinearBlock>& b) {
    return c2_linbuf_new(self, b, b->offset(), b->size());
}

}
