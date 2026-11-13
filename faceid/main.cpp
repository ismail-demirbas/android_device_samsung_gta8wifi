#include <aidl/android/hardware/biometrics/face/BnFace.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <hidl/HidlTransportSupport.h>

#include "Session.h"

namespace gta8face {

class Face : public aface::BnFace {
  public:
    ndk::ScopedAStatus getSensorProps(std::vector<aface::SensorProps>* out) override {
        aface::SensorProps p;
        p.commonProps.sensorId = 0;
        p.commonProps.sensorStrength = acommon::SensorStrength::CONVENIENCE;
        p.commonProps.maxEnrollmentsPerUser = 1;
        p.sensorType = aface::FaceSensorType::RGB;
        p.halControlsPreview = true;
        p.previewDisplayId = 0;
        p.enrollPreviewWidth = 0;
        p.enrollPreviewHeight = 0;
        p.enrollTranslationX = 0.0f;
        p.enrollTranslationY = 0.0f;
        p.enrollPreviewScale = 1.0f;
        p.supportsDetectInteraction = false;
        out->clear();
        out->push_back(p);
        return ndk::ScopedAStatus::ok();
    }

    ndk::ScopedAStatus createSession(int32_t, int32_t userId,
                                     const std::shared_ptr<aface::ISessionCallback>& cb,
                                     std::shared_ptr<aface::ISession>* out) override {
        android::sp<hv1::IBiometricsFace> hal = hv1::IBiometricsFace::getService();
        if (hal == nullptr) {
            return ndk::ScopedAStatus::fromServiceSpecificError(
                    static_cast<int32_t>(aface::Error::HW_UNAVAILABLE));
        }
        *out = ndk::SharedRefBase::make<Session>(hal, userId, cb);
        return ndk::ScopedAStatus::ok();
    }
};

}  // namespace gta8face

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    android::hardware::configureRpcThreadpool(1, false);
    auto face = ndk::SharedRefBase::make<gta8face::Face>();
    const std::string instance = std::string(gta8face::Face::descriptor) + "/default";
    binder_status_t st = AServiceManager_addService(face->asBinder().get(), instance.c_str());
    if (st != STATUS_OK) return 1;
    ABinderProcess_joinThreadPool();
    return 1;
}
