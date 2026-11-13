#define LOG_TAG "BtAudioShim"
#include <android/binder_process.h>
#include <android/binder_status.h>
#include <dlfcn.h>
#include <errno.h>
#include <hardware/hardware.h>
#include <log/log.h>

extern "C" binder_status_t createIBluetoothAudioProviderFactory();

#ifdef __LP64__
#define REAL_LIB "/vendor/lib64/hw/audio.bluetooth.default.so"
#else
#define REAL_LIB "/vendor/lib/hw/audio.bluetooth.default.so"
#endif

static hw_module_t* real_module() {
    static hw_module_t* m = nullptr;
    if (m) return m;
    void* h = dlopen(REAL_LIB, RTLD_NOW | RTLD_GLOBAL);
    if (!h) {
        ALOGE("dlopen failed: %s", dlerror());
        return nullptr;
    }
    m = static_cast<hw_module_t*>(dlsym(h, HAL_MODULE_INFO_SYM_AS_STR));
    if (!m) ALOGE("dlsym failed: %s", dlerror());
    return m;
}

static int shim_open(const hw_module_t*, const char* name, hw_device_t** dev) {
    hw_module_t* m = real_module();
    if (!m || !m->methods || !m->methods->open) return -ENODEV;
    return m->methods->open(m, name, dev);
}

static hw_module_methods_t shim_methods = {.open = shim_open};

__attribute__((constructor)) static void shim_init() {
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    binder_status_t st = createIBluetoothAudioProviderFactory();
    if (st != STATUS_OK) ALOGE("factory register failed: %d", st);
    ABinderProcess_startThreadPool();
}

extern "C" __attribute__((visibility("default"))) hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = 0,
    .hal_api_version = 0,
    .id = "audio",
    .name = "BT audio shim",
    .author = "gta8",
    .methods = &shim_methods,
};
