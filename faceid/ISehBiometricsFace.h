#pragma once
#include <android/hardware/biometrics/face/1.0/IBiometricsFace.h>
#include <hidl/HidlSupport.h>
#include <utils/StrongPointer.h>
#include <functional>
#include "ISehBiometricsFaceClientCallback.h"

namespace vendor {
namespace samsung {
namespace hardware {
namespace biometrics {
namespace face {
namespace V2_0 {

struct ISehBiometricsFace : public ::android::hardware::biometrics::face::V1_0::IBiometricsFace {
    using sehSetCallback_cb = std::function<void(
            const ::android::hardware::biometrics::face::V1_0::OptionalUint64& result)>;
    virtual ::android::hardware::Return<void> sehSetCallback(
            const ::android::sp<ISehBiometricsFaceClientCallback>& clientCallback,
            sehSetCallback_cb _hidl_cb) = 0;
    static ::android::hardware::Return<::android::sp<ISehBiometricsFace>> castFrom(
            const ::android::sp<::android::hardware::biometrics::face::V1_0::IBiometricsFace>& parent,
            bool emitError = false);
};

}  // namespace V2_0
}  // namespace face
}  // namespace biometrics
}  // namespace hardware
}  // namespace samsung
}  // namespace vendor
