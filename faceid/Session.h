#pragma once

#include <aidl/android/hardware/biometrics/common/BnCancellationSignal.h>
#include <aidl/android/hardware/biometrics/common/OperationContext.h>
#include <aidl/android/hardware/biometrics/face/BnSession.h>
#include <aidl/android/hardware/biometrics/face/FaceEnrollOptions.h>
#include <aidl/android/hardware/biometrics/face/ISessionCallback.h>
#include <android/hardware/biometrics/face/1.0/IBiometricsFace.h>
#include <android/hardware/biometrics/face/1.0/IBiometricsFaceClientCallback.h>
#include "ISehBiometricsFace.h"
#include "ISehBiometricsFaceClientCallback.h"

#include <atomic>
#include <memory>
#include <optional>
#include <vector>

namespace gta8face {

namespace hv1 = android::hardware::biometrics::face::V1_0;
namespace aface = aidl::android::hardware::biometrics::face;
namespace acommon = aidl::android::hardware::biometrics::common;
namespace akm = aidl::android::hardware::keymaster;
using aidl::android::hardware::common::NativeHandle;
using android::hardware::hidl_vec;
using android::hardware::Return;
namespace hsam = vendor::samsung::hardware::biometrics::face::V2_0;

class HidlCallback : public hsam::ISehBiometricsFaceClientCallback {
  public:
    HidlCallback(std::shared_ptr<aface::ISessionCallback> cb,
                 std::shared_ptr<std::atomic<bool>> enrolling)
        : mCb(std::move(cb)), mEnrolling(std::move(enrolling)), mDetached(false) {}
    void detach() { mDetached = true; }
    Return<void> onEnrollResult(uint64_t deviceId, uint32_t faceId, int32_t userId,
                                uint32_t remaining) override;
    Return<void> onAuthenticated(uint64_t deviceId, uint32_t faceId, int32_t userId,
                                 const hidl_vec<uint8_t>& token) override;
    Return<void> onAcquired(uint64_t deviceId, int32_t userId, hv1::FaceAcquiredInfo acquiredInfo,
                            int32_t vendorCode) override;
    Return<void> onError(uint64_t deviceId, int32_t userId, hv1::FaceError error,
                         int32_t vendorCode) override;
    Return<void> onRemoved(uint64_t deviceId, const hidl_vec<uint32_t>& removed,
                           int32_t userId) override;
    Return<void> onEnumerate(uint64_t deviceId, const hidl_vec<uint32_t>& faceIds,
                             int32_t userId) override;
    Return<void> onLockoutChanged(uint64_t duration) override;
    Return<void> sehOnPreviewUpdated(const hidl_vec<uint8_t>& data, int32_t a, int32_t b, int32_t c,
                                     uint32_t d) override;
    Return<void> sehOnAuthenticated(uint64_t a, uint32_t b, int32_t c, const hidl_vec<uint8_t>& d,
                                    const hidl_vec<uint8_t>& e) override;
    Return<void> sehOnPreviewFrame(const android::hardware::hidl_memory& mem, int32_t a, int32_t b,
                                   int32_t c, uint32_t d) override;

  private:
    std::shared_ptr<aface::ISessionCallback> mCb;
    std::shared_ptr<std::atomic<bool>> mEnrolling;
    std::atomic<bool> mDetached;
};

class CancelSignal : public acommon::BnCancellationSignal {
  public:
    explicit CancelSignal(android::sp<hv1::IBiometricsFace> hal) : mHal(std::move(hal)) {}
    ndk::ScopedAStatus cancel() override;

  private:
    android::sp<hv1::IBiometricsFace> mHal;
};

class Session : public aface::BnSession {
  public:
    Session(android::sp<hv1::IBiometricsFace> hal, int32_t userId,
            std::shared_ptr<aface::ISessionCallback> cb);
    ndk::ScopedAStatus generateChallenge() override;
    ndk::ScopedAStatus revokeChallenge(int64_t challenge) override;
    ndk::ScopedAStatus getEnrollmentConfig(
            aface::EnrollmentType enrollmentType,
            std::vector<aface::EnrollmentStageConfig>* return_val) override;
    ndk::ScopedAStatus enroll(const akm::HardwareAuthToken& hat, aface::EnrollmentType enrollmentType,
                              const std::vector<aface::Feature>& features,
                              const std::optional<NativeHandle>& previewSurface,
                              std::shared_ptr<acommon::ICancellationSignal>* return_val) override;
    ndk::ScopedAStatus authenticate(
            int64_t keystoreOperationId,
            std::shared_ptr<acommon::ICancellationSignal>* returnVal) override;
    ndk::ScopedAStatus detectInteraction(
            std::shared_ptr<acommon::ICancellationSignal>* returnVal) override;
    ndk::ScopedAStatus enumerateEnrollments() override;
    ndk::ScopedAStatus removeEnrollments(const std::vector<int32_t>& enrollmentIds) override;
    ndk::ScopedAStatus getFeatures() override;
    ndk::ScopedAStatus setFeature(const akm::HardwareAuthToken& hat, aface::Feature feature,
                                  bool enabled) override;
    ndk::ScopedAStatus getAuthenticatorId() override;
    ndk::ScopedAStatus invalidateAuthenticatorId() override;
    ndk::ScopedAStatus resetLockout(const akm::HardwareAuthToken& hat) override;
    ndk::ScopedAStatus close() override;
    ndk::ScopedAStatus authenticateWithContext(
            int64_t operationId, const acommon::OperationContext& context,
            std::shared_ptr<acommon::ICancellationSignal>* out) override;
    ndk::ScopedAStatus enrollWithContext(
            const akm::HardwareAuthToken& hat, aface::EnrollmentType enrollmentType,
            const std::vector<aface::Feature>& features,
            const std::optional<NativeHandle>& previewSurface,
            const acommon::OperationContext& context,
            std::shared_ptr<acommon::ICancellationSignal>* out) override;
    ndk::ScopedAStatus detectInteractionWithContext(
            const acommon::OperationContext& context,
            std::shared_ptr<acommon::ICancellationSignal>* out) override;
    ndk::ScopedAStatus onContextChanged(const acommon::OperationContext& context) override;
    ndk::ScopedAStatus enrollWithOptions(
            const aface::FaceEnrollOptions& options,
            std::shared_ptr<acommon::ICancellationSignal>* out) override;

  private:
    ndk::ScopedAStatus doEnroll(const akm::HardwareAuthToken& hat,
                                const std::vector<aface::Feature>& features,
                                std::shared_ptr<acommon::ICancellationSignal>* out);
    android::sp<hv1::IBiometricsFace> mHal;
    std::shared_ptr<aface::ISessionCallback> mCb;
    std::shared_ptr<std::atomic<bool>> mEnrolling;
    android::sp<HidlCallback> mHidlCb;
};

}  // namespace gta8face
