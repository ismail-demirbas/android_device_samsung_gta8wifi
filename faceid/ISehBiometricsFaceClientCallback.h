#pragma once
#include <android/hardware/biometrics/face/1.0/IBiometricsFaceClientCallback.h>
#include <hidl/HidlSupport.h>
#include <utils/StrongPointer.h>

namespace vendor {
namespace samsung {
namespace hardware {
namespace biometrics {
namespace face {
namespace V2_0 {

struct ISehBiometricsFaceClientCallback
    : public ::android::hardware::biometrics::face::V1_0::IBiometricsFaceClientCallback {
    virtual ::android::hardware::Return<void> sehOnPreviewUpdated(
            const ::android::hardware::hidl_vec<uint8_t>& data, int32_t a, int32_t b, int32_t c,
            uint32_t d) = 0;
    virtual ::android::hardware::Return<void> sehOnAuthenticated(
            uint64_t a, uint32_t b, int32_t c, const ::android::hardware::hidl_vec<uint8_t>& d,
            const ::android::hardware::hidl_vec<uint8_t>& e) = 0;
    virtual ::android::hardware::Return<void> sehOnPreviewFrame(
            const ::android::hardware::hidl_memory& mem, int32_t a, int32_t b, int32_t c,
            uint32_t d) = 0;

    ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;
    ::android::hardware::Return<void> debug(
            const ::android::hardware::hidl_handle& fd,
            const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;
    ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;
    ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;
    ::android::hardware::Return<void> setHALInstrumentation() override;
    ::android::hardware::Return<bool> linkToDeath(
            const ::android::sp<::android::hardware::hidl_death_recipient>& recipient,
            uint64_t cookie) override;
    ::android::hardware::Return<void> ping() override;
    ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;
    ::android::hardware::Return<void> notifySyspropsChanged() override;
    ::android::hardware::Return<bool> unlinkToDeath(
            const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;
};

}  // namespace V2_0
}  // namespace face
}  // namespace biometrics
}  // namespace hardware
}  // namespace samsung
}  // namespace vendor
