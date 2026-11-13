#include <aidl/android/hardware/power/BnPower.h>
#include <android-base/file.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <memory>
#include <string>

using aidl::android::hardware::power::BnPower;
using aidl::android::hardware::power::Boost;
using aidl::android::hardware::power::IPower;
using aidl::android::hardware::power::Mode;

class PowerAot : public BnPower {
  public:
    explicit PowerAot(std::shared_ptr<IPower> real) : mReal(std::move(real)) {}

    ndk::ScopedAStatus setMode(Mode type, bool enabled) override {
        if (type == Mode::DOUBLE_TAP_TO_WAKE) {
            android::base::WriteStringToFile(enabled ? "aot_enable,1" : "aot_enable,0",
                                             "/sys/class/sec/tsp/cmd");
            return ndk::ScopedAStatus::ok();
        }
        return mReal->setMode(type, enabled);
    }

    ndk::ScopedAStatus isModeSupported(Mode type, bool* ret) override {
        if (type == Mode::DOUBLE_TAP_TO_WAKE) {
            *ret = true;
            return ndk::ScopedAStatus::ok();
        }
        return mReal->isModeSupported(type, ret);
    }

    ndk::ScopedAStatus setBoost(Boost type, int32_t durationMs) override {
        return mReal->setBoost(type, durationMs);
    }

    ndk::ScopedAStatus isBoostSupported(Boost type, bool* ret) override {
        return mReal->isBoostSupported(type, ret);
    }

  private:
    std::shared_ptr<IPower> mReal;
};

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    const std::string name = std::string(IPower::descriptor) + "/default";
    ndk::SpAIBinder binder(AServiceManager_waitForService(name.c_str()));
    std::shared_ptr<IPower> real = IPower::fromBinder(binder);
    if (!real) return 1;
    auto svc = ndk::SharedRefBase::make<PowerAot>(real);
    if (AServiceManager_addService(svc->asBinder().get(), name.c_str()) != STATUS_OK) return 1;
    ABinderProcess_joinThreadPool();
    return 0;
}
