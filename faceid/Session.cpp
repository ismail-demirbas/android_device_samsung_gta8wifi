#include "Session.h"

#include <hidl/HybridInterface.h>
#include <gui/Surface.h>
#include <gui/bufferqueue/1.0/H2BGraphicBufferProducer.h>
#include <android/hardware/graphics/bufferqueue/1.0/IGraphicBufferProducer.h>
#include <system/window.h>
#include <sys/mman.h>
#include <log/log.h>
#include <algorithm>
#include <mutex>
#include <cstring>
#undef LOG_TAG
#define LOG_TAG "gta8face"

#include <android/binder_status.h>
#include <arpa/inet.h>
#include <endian.h>
#include <sys/stat.h>

#include <aidl/android/hardware/keymaster/HardwareAuthenticatorType.h>
#include <aidl/android/hardware/keymaster/Timestamp.h>

#include <cstring>
#include <string>

namespace gta8face {

namespace {
using HGBP = android::hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer;
std::mutex gPrevMu;
android::sp<android::Surface> gPrevSurface;
int gFrameLogs = 0;

void clearPreview() {
    std::lock_guard<std::mutex> l(gPrevMu);
    gPrevSurface = nullptr;
}

void setPreview(const std::optional<NativeHandle>& h) {
    std::lock_guard<std::mutex> l(gPrevMu);
    gPrevSurface = nullptr;
    gFrameLogs = 0;
    if (!h.has_value() || h->ints.empty()) {
        ALOGW("preview: yuzey yok");
        return;
    }
    int32_t size = h->ints[0];
    if (size <= 0 || size > 256 ||
        h->ints.size() * sizeof(int32_t) < sizeof(int32_t) + static_cast<size_t>(size)) {
        ALOGE("preview: gecersiz token boyutu %d", size);
        return;
    }
    android::HalToken tok;
    tok.resize(size);
    memcpy(tok.data(), &h->ints[1], size);
    android::sp<HGBP> hgbp = HGBP::castFrom(android::retrieveHalInterface(tok));
    if (hgbp == nullptr) {
        ALOGE("preview: retrieveHalInterface basarisiz");
        return;
    }
    android::sp<android::IGraphicBufferProducer> igbp =
            new android::hardware::graphics::bufferqueue::V1_0::utils::H2BGraphicBufferProducer(hgbp);
    gPrevSurface = new android::Surface(igbp, true);
    ALOGI("preview: yuzey hazir");
}

void setPreviewWindow(ANativeWindow* w) {
    std::lock_guard<std::mutex> l(gPrevMu);
    gFrameLogs = 0;
    gPrevSurface = static_cast<android::Surface*>(w);
    ALOGI("preview: yuzey hazir (surfacePreview)");
}

inline uint8_t clamp8(int v) { return v < 0 ? 0 : (v > 255 ? 255 : v); }

void drawFrame(const android::hardware::hidl_memory& mem, int32_t a, int32_t b, int32_t c,
               uint32_t d) {
    android::sp<android::Surface> s;
    bool log = false;
    {
        std::lock_guard<std::mutex> l(gPrevMu);
        s = gPrevSurface;
        if (gFrameLogs < 5) {
            gFrameLogs++;
            log = true;
        }
    }
    size_t sz = mem.size();
    if (log) {
        ALOGI("sehOnPreviewFrame size=%zu a=%d b=%d c=%d d=%u name=%s surface=%d", sz, a, b, c, d,
              mem.name().c_str(), s != nullptr);
    }
    if (s == nullptr || a <= 0 || b <= 0) return;
    const native_handle_t* nh = mem.handle();
    if (nh == nullptr || nh->numFds < 1) return;
    size_t w = a, h = b;
    bool rgba = sz >= w * h * 4;
    bool nv21 = !rgba && sz >= w * h * 3 / 2;
    if (!rgba && !nv21) return;
    void* p = mmap(nullptr, sz, PROT_READ, MAP_SHARED, nh->data[0], 0);
    if (p == MAP_FAILED) return;
    ANativeWindow* win = s.get();
    native_window_set_buffers_dimensions(win, 0, 0);
    native_window_set_buffers_format(win, HAL_PIXEL_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buf;
    if (s->lock(&buf, nullptr) == 0) {
        uint8_t* dst = static_cast<uint8_t*>(buf.bits);
        const uint8_t* src = static_cast<const uint8_t*>(p);
        size_t W = buf.width, H = buf.height;
        size_t dw, dh;
        if (W * h >= H * w) {
            dw = W;
            dh = h * W / w;
        } else {
            dh = H;
            dw = w * H / h;
        }
        size_t cx = (dw - W) / 2, cy = (dh - H) / 2;
        for (size_t j = 0; j < H; j++) {
            uint32_t* r = reinterpret_cast<uint32_t*>(dst + j * buf.stride * 4);
            for (size_t i = 0; i < W; i++) r[i] = 0xFF000000u;
        }
        for (size_t dy = 0; dy < H; dy++) {
            size_t j = (dy + cy) * h / dh;
            uint8_t* row = dst + dy * buf.stride * 4;
            const uint8_t* vu = src + w * h + (j / 2) * w;
            for (size_t dx = 0; dx < W; dx++) {
                size_t i = (dx + cx) * w / dw;
                if (rgba) {
                    memcpy(row + dx * 4, src + (j * w + i) * 4, 4);
                } else {
                    int Y = src[j * w + i];
                    int V = vu[i & ~static_cast<size_t>(1)] - 128;
                    int U = vu[(i & ~static_cast<size_t>(1)) + 1] - 128;
                    row[dx * 4 + 0] = clamp8(Y + ((359 * V) >> 8));
                    row[dx * 4 + 1] = clamp8(Y - ((88 * U + 183 * V) >> 8));
                    row[dx * 4 + 2] = clamp8(Y + ((454 * U) >> 8));
                    row[dx * 4 + 3] = 255;
                }
            }
        }
        s->unlockAndPost();
    }
    munmap(p, sz);
}
}  // namespace

namespace {

constexpr size_t kHatSize = 69;

hidl_vec<uint8_t> hatToHidl(const akm::HardwareAuthToken& h) {
    hidl_vec<uint8_t> v;
    v.resize(kHatSize);
    uint8_t* p = v.data();
    memset(p, 0, kHatSize);
    memcpy(p + 1, &h.challenge, 8);
    memcpy(p + 9, &h.userId, 8);
    memcpy(p + 17, &h.authenticatorId, 8);
    uint32_t t = htonl(static_cast<uint32_t>(h.authenticatorType));
    memcpy(p + 25, &t, 4);
    uint64_t ts = htobe64(static_cast<uint64_t>(h.timestamp.milliSeconds));
    memcpy(p + 29, &ts, 8);
    size_t n = h.mac.size() < 32 ? h.mac.size() : 32;
    if (n > 0) memcpy(p + 37, h.mac.data(), n);
    return v;
}

akm::HardwareAuthToken hidlToHat(const hidl_vec<uint8_t>& b) {
    akm::HardwareAuthToken h;
    if (b.size() < kHatSize) return h;
    const uint8_t* p = b.data();
    memcpy(&h.challenge, p + 1, 8);
    memcpy(&h.userId, p + 9, 8);
    memcpy(&h.authenticatorId, p + 17, 8);
    uint32_t t;
    memcpy(&t, p + 25, 4);
    h.authenticatorType = static_cast<akm::HardwareAuthenticatorType>(ntohl(t));
    uint64_t ts;
    memcpy(&ts, p + 29, 8);
    h.timestamp.milliSeconds = static_cast<int64_t>(be64toh(ts));
    h.mac.assign(p + 37, p + 69);
    return h;
}

bool statusOk(Return<hv1::Status>& r) {
    if (!r.isOk()) return false;
    hv1::Status s = r;
    return s == hv1::Status::OK;
}

}  // namespace

Return<void> HidlCallback::onEnrollResult(uint64_t, uint32_t faceId, int32_t, uint32_t remaining) {
    if (mDetached) return android::hardware::Void();
    if (remaining == 0) *mEnrolling = false;
    mCb->onEnrollmentProgress(static_cast<int32_t>(faceId), static_cast<int32_t>(remaining));
    return android::hardware::Void();
}

Return<void> HidlCallback::onAuthenticated(uint64_t, uint32_t faceId, int32_t,
                                           const hidl_vec<uint8_t>& token) {
    if (mDetached) return android::hardware::Void();
    if (faceId == 0) {
        mCb->onAuthenticationFailed();
    } else {
        mCb->onAuthenticationSucceeded(static_cast<int32_t>(faceId), hidlToHat(token));
    }
    return android::hardware::Void();
}

Return<void> HidlCallback::onAcquired(uint64_t, int32_t, hv1::FaceAcquiredInfo info,
                                      int32_t vendorCode) {
    if (mDetached) return android::hardware::Void();
    int v = static_cast<int>(info);
    aface::AcquiredInfo ai = (v >= 0 && v <= 22) ? static_cast<aface::AcquiredInfo>(v + 1)
                                                 : aface::AcquiredInfo::UNKNOWN;
    if (*mEnrolling) {
        aface::EnrollmentFrame f;
        f.data.acquiredInfo = ai;
        f.data.vendorCode = vendorCode;
        mCb->onEnrollmentFrame(f);
    } else {
        aface::AuthenticationFrame f;
        f.data.acquiredInfo = ai;
        f.data.vendorCode = vendorCode;
        mCb->onAuthenticationFrame(f);
    }
    return android::hardware::Void();
}

Return<void> HidlCallback::onError(uint64_t, int32_t, hv1::FaceError error, int32_t vendorCode) {
    if (mDetached) return android::hardware::Void();
    *mEnrolling = false;
    switch (error) {
        case hv1::FaceError::LOCKOUT:
            mCb->onLockoutTimed(30000);
            break;
        case hv1::FaceError::LOCKOUT_PERMANENT:
            mCb->onLockoutPermanent();
            break;
        case hv1::FaceError::VENDOR:
            mCb->onError(aface::Error::VENDOR, vendorCode);
            break;
        default: {
            int v = static_cast<int>(error);
            aface::Error e = (v >= 1 && v <= 6) ? static_cast<aface::Error>(v)
                                                : aface::Error::UNKNOWN;
            mCb->onError(e, vendorCode);
        }
    }
    return android::hardware::Void();
}

Return<void> HidlCallback::onRemoved(uint64_t, const hidl_vec<uint32_t>& removed, int32_t) {
    if (mDetached) return android::hardware::Void();
    std::vector<int32_t> ids;
    for (size_t i = 0; i < removed.size(); i++) ids.push_back(static_cast<int32_t>(removed[i]));
    mCb->onEnrollmentsRemoved(ids);
    return android::hardware::Void();
}

Return<void> HidlCallback::onEnumerate(uint64_t, const hidl_vec<uint32_t>& faceIds, int32_t) {
    if (mDetached) return android::hardware::Void();
    std::vector<int32_t> ids;
    for (size_t i = 0; i < faceIds.size(); i++) ids.push_back(static_cast<int32_t>(faceIds[i]));
    mCb->onEnrollmentsEnumerated(ids);
    return android::hardware::Void();
}

Return<void> HidlCallback::onLockoutChanged(uint64_t duration) {
    if (mDetached) return android::hardware::Void();
    if (duration > 0) {
        mCb->onLockoutTimed(static_cast<int64_t>(duration));
    } else {
        mCb->onLockoutCleared();
    }
    return android::hardware::Void();
}

ndk::ScopedAStatus CancelSignal::cancel() {
    mHal->cancel().isOk();
    clearPreview();
    return ndk::ScopedAStatus::ok();
}

Session::Session(android::sp<hv1::IBiometricsFace> hal, int32_t userId,
                 std::shared_ptr<aface::ISessionCallback> cb)
    : mHal(std::move(hal)),
      mCb(std::move(cb)),
      mEnrolling(std::make_shared<std::atomic<bool>>(false)) {
    mHidlCb = new HidlCallback(mCb, mEnrolling);
    mHal->setCallback(mHidlCb, [](const hv1::OptionalUint64 &) {}).isOk();
    {
        auto sehR = hsam::ISehBiometricsFace::castFrom(mHal);
        android::sp<hsam::ISehBiometricsFace> seh = sehR.isOk() ? android::sp<hsam::ISehBiometricsFace>(sehR) : nullptr;
        if (seh != nullptr) {
            seh->sehSetCallback(mHidlCb, [](const hv1::OptionalUint64 &) {}).isOk();
        }
    }
    std::string base = "/data/vendor_de/" + std::to_string(userId);
    std::string path = base + "/facedata";
    mkdir(base.c_str(), 0771);
    mkdir(path.c_str(), 0770);
    mHal->setActiveUser(userId, path).isOk();
}

ndk::ScopedAStatus Session::generateChallenge() {
    hv1::OptionalUint64 v;
    auto r = mHal->generateChallenge(60, [&v](const hv1::OptionalUint64 &o) { v = o; });
    if (!r.isOk()) {
        mCb->onError(aface::Error::HW_UNAVAILABLE, 0);
        return ndk::ScopedAStatus::ok();
    }
    if (v.status != hv1::Status::OK) {
        mCb->onError(aface::Error::UNABLE_TO_PROCESS, 0);
        return ndk::ScopedAStatus::ok();
    }
    mCb->onChallengeGenerated(static_cast<int64_t>(v.value));
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::revokeChallenge(int64_t challenge) {
    mHal->revokeChallenge().isOk();
    mCb->onChallengeRevoked(challenge);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::getEnrollmentConfig(aface::EnrollmentType,
                                                std::vector<aface::EnrollmentStageConfig>* out) {
    out->clear();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::doEnroll(const akm::HardwareAuthToken& hat,
                                     const std::vector<aface::Feature>& features,
                                     std::shared_ptr<acommon::ICancellationSignal>* out) {
    bool attention = false;
    bool diverse = false;
    for (auto f : features) {
        if (f == aface::Feature::REQUIRE_ATTENTION) attention = true;
        if (f == aface::Feature::REQUIRE_DIVERSE_POSES) diverse = true;
    }
    std::vector<hv1::Feature> d;
    if (!attention) d.push_back(hv1::Feature::REQUIRE_ATTENTION);
    if (!diverse) d.push_back(hv1::Feature::REQUIRE_DIVERSITY);
    hidl_vec<hv1::Feature> disabled;
    disabled = d;
    *mEnrolling = true;
    auto r = mHal->enroll(hatToHidl(hat), 60, disabled);
    if (!statusOk(r)) {
        *mEnrolling = false;
        mCb->onError(aface::Error::UNABLE_TO_PROCESS, 0);
    }
    *out = ndk::SharedRefBase::make<CancelSignal>(mHal);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::enroll(const akm::HardwareAuthToken& hat, aface::EnrollmentType,
                                   const std::vector<aface::Feature>& features,
                                   const std::optional<NativeHandle>& prev,
                                   std::shared_ptr<acommon::ICancellationSignal>* out) {
    setPreview(prev);
    return doEnroll(hat, features, out);
}

ndk::ScopedAStatus Session::authenticate(int64_t operationId,
                                         std::shared_ptr<acommon::ICancellationSignal>* out) {
    *mEnrolling = false;
    auto r = mHal->authenticate(static_cast<uint64_t>(operationId));
    if (!statusOk(r)) mCb->onError(aface::Error::UNABLE_TO_PROCESS, 0);
    *out = ndk::SharedRefBase::make<CancelSignal>(mHal);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::detectInteraction(std::shared_ptr<acommon::ICancellationSignal>*) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Session::enumerateEnrollments() {
    auto r = mHal->enumerate();
    if (!statusOk(r)) mCb->onEnrollmentsEnumerated(std::vector<int32_t>());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::removeEnrollments(const std::vector<int32_t>& ids) {
    for (auto id : ids) {
        auto r = mHal->remove(static_cast<uint32_t>(id));
        if (!statusOk(r)) mCb->onError(aface::Error::UNABLE_TO_REMOVE, 0);
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::getFeatures() {
    mCb->onFeaturesRetrieved(std::vector<aface::Feature>());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::setFeature(const akm::HardwareAuthToken&, aface::Feature feature,
                                       bool) {
    mCb->onFeatureSet(feature);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::getAuthenticatorId() {
    int64_t id = 0;
    mHal->getAuthenticatorId([&id](const hv1::OptionalUint64 &v) {
        if (v.status == hv1::Status::OK) id = static_cast<int64_t>(v.value);
    }).isOk();
    mCb->onAuthenticatorIdRetrieved(id);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::invalidateAuthenticatorId() {
    int64_t id = 0;
    mHal->getAuthenticatorId([&id](const hv1::OptionalUint64 &v) {
        if (v.status == hv1::Status::OK) id = static_cast<int64_t>(v.value);
    }).isOk();
    mCb->onAuthenticatorIdInvalidated(id);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::resetLockout(const akm::HardwareAuthToken& hat) {
    mHal->resetLockout(hatToHidl(hat)).isOk();
    mCb->onLockoutCleared();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::close() {
    mHal->cancel().isOk();
    mHidlCb->detach();
    mCb->onSessionClosed();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::authenticateWithContext(
        int64_t operationId, const acommon::OperationContext&,
        std::shared_ptr<acommon::ICancellationSignal>* out) {
    return authenticate(operationId, out);
}

ndk::ScopedAStatus Session::enrollWithContext(
        const akm::HardwareAuthToken& hat, aface::EnrollmentType,
        const std::vector<aface::Feature>& features, const std::optional<NativeHandle>& prev,
        const acommon::OperationContext&, std::shared_ptr<acommon::ICancellationSignal>* out) {
    setPreview(prev);
    return doEnroll(hat, features, out);
}

ndk::ScopedAStatus Session::detectInteractionWithContext(
        const acommon::OperationContext&, std::shared_ptr<acommon::ICancellationSignal>*) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Session::onContextChanged(const acommon::OperationContext&) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::enrollWithOptions(
        const aface::FaceEnrollOptions& options,
        std::shared_ptr<acommon::ICancellationSignal>* out) {
    if (options.surfacePreview.has_value() && options.surfacePreview->get() != nullptr) {
        setPreviewWindow(options.surfacePreview->get());
    } else {
        setPreview(options.nativeHandlePreview);
    }
    return doEnroll(options.hardwareAuthToken, options.features, out);
}

Return<void> HidlCallback::sehOnPreviewUpdated(const hidl_vec<uint8_t>&, int32_t, int32_t, int32_t,
                                               uint32_t) {
    return android::hardware::Void();
}

Return<void> HidlCallback::sehOnAuthenticated(uint64_t, uint32_t, int32_t, const hidl_vec<uint8_t>&,
                                              const hidl_vec<uint8_t>&) {
    return android::hardware::Void();
}

Return<void> HidlCallback::sehOnPreviewFrame(const android::hardware::hidl_memory& mem, int32_t a,
                                             int32_t b, int32_t c, uint32_t d) {
    if (mDetached) return android::hardware::Void();
    drawFrame(mem, a, b, c, d);
    return android::hardware::Void();
}

}  // namespace gta8face
