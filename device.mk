#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Enable updating of APEXes
$(call inherit-product, $(SRC_TARGET_DIR)/product/updatable_apex.mk)

# API levels
PRODUCT_SHIPPING_API_LEVEL := 30


# fastbootd
PRODUCT_PACKAGES += \
    libui_shim_gta8wifi \
    libc2_shim_gta8wifi \
    libsec_ssl_shim \
    libsec_base_shim \
    libsec_ima_shim \
    android.hardware.fastboot@1.1-impl-mock \
    fastbootd

# Health
PRODUCT_PACKAGES += \
    android.hardware.health@2.1-impl \
    android.hardware.health@2.1-impl.recovery

PRODUCT_PACKAGES += vendor.lineage.health-service.default
PRODUCT_PACKAGES += DeviceFrameworkOverlay \
    DeviceWifiOverlay \
    DeviceSettingsOverlay

PRODUCT_PACKAGES += gta8wifi_vendor_manifest.xml
PRODUCT_PACKAGES += vendor.gta8wifi.power-aot
PRODUCT_PACKAGES += android.hardware.biometrics.face-service.samsung
PRODUCT_PACKAGES += camera.ums512
PRODUCT_PACKAGES += libstagefright_sprd_mp3dec libstagefright_sprd_mp3enc
PRODUCT_PACKAGES += audio.bluetooth.ums512_gta8wifi audio.bluetooth.default
PRODUCT_PACKAGES += charger_bl_daemon
PRODUCT_PACKAGES += folio_daemon
PRODUCT_PACKAGES += folio-daemon_Settings
PRODUCT_PACKAGES += afc_mode.rc
PRODUCT_PACKAGES += swapfile_gta8wifi swapfile_rc_gta8wifi
PRODUCT_PACKAGES += SwapfileSettings_gta8wifi
PRODUCT_PACKAGES += OtaOverlayGta8wifi

PRODUCT_SYSTEM_PROPERTIES += persist.sys.folio.enabled=0
PRODUCT_SYSTEM_PROPERTIES += persist.sys.swapfile.enabled=0

# Camera2Api Temporary Fix
PRODUCT_SYSTEM_PROPERTIES += debug.stagefright.c2inputsurface=-1

# LKM Twealer
PRODUCT_SYSTEM_PROPERTIES += ro.lmk.swap_free_low_percentage=1
PRODUCT_SYSTEM_PROPERTIES += ro.lmk.lowmem_min_oom_score=1001

# Kernel
PRODUCT_ENABLE_UFFD_GC := false

# Overlays
PRODUCT_ENFORCE_RRO_TARGETS := *

# Partitions
PRODUCT_USE_DYNAMIC_PARTITIONS := true

# Product characteristics
PRODUCT_CHARACTERISTICS := tablet

# Rootdir
PRODUCT_PACKAGES += \
    log_to_csv.sh \
    loading.sh \
    para.sh \
    total.sh \

PRODUCT_PACKAGES += \
    init.cali.rc \
    init.factorytest.rc \
    init.gta8wifi.rc \
    init.storage.rc \
    init.ums512_25c10.rc \
    init.ums512_25c10.usb.rc

# Exclude 32 bit libraries
PRODUCT_PACKAGES_EXCLUDE_32BIT += \
    vendor.samsung.hardware.biometrics.face@3.0 \
    vendor.sprd.hardware.log-V1-ndk \
    vendor.unisoc.hardware.power-V1-ndk

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH)

# Inherit the proprietary files
PRODUCT_COPY_FILES += device/samsung/gta8wifi/sepolicy/vendor/vendor_service_contexts:$(TARGET_COPY_OUT_VENDOR)/etc/selinux/vendor_service_contexts

$(call inherit-product, vendor/samsung/gta8wifi/gta8wifi-vendor.mk)

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/etc/fstab.ums512_25c10:$(TARGET_COPY_OUT_RAMDISK)/fstab.ums512_25c10

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/etc/init.recovery.ums512_25c10.rc:$(TARGET_COPY_OUT_RECOVERY)/root/init.recovery.ums512_25c10.rc

# Compatibility matrix
DEVICE_COMPATIBILITY_MATRIX_FILE := $(LOCAL_PATH)/vintf/compatibility_matrix.device.xml
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE := $(LOCAL_PATH)/vintf/compatibility_matrix.framework.xml
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE += $(LOCAL_PATH)/vintf/device_framework_matrix.xml
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE += $(LOCAL_PATH)/vintf/compatibility_matrix.xml

PRODUCT_PACKAGES += \
    gta8wifi_symlink_firmware_audio_structure \
    gta8wifi_symlink_firmware_cvs \
    gta8wifi_symlink_firmware_dsp_smartamp \
    gta8wifi_symlink_firmware_dsp_vbc \
    gta8wifi_symlink_lib_enhance_ums512_so \
    gta8wifi_symlink_lib_hw_vulkan_ums512_so \
    gta8wifi_symlink_lib_libOpenCL_so \
    gta8wifi_symlink_lib_libOpenCL_so_1 \
    gta8wifi_symlink_lib_libOpenCL_so_1_1 \
    gta8wifi_symlink_lib_libOpenCL_so_1_2 \
    gta8wifi_symlink_lib_npidevice_libaudionpi_so \
    gta8wifi_symlink_lib_npidevice_libcamcalitest_so \
    gta8wifi_symlink_lib_npidevice_libnpi_rtc_so \
    gta8wifi_symlink_lib_npidevice_libsensornpi_so \
    gta8wifi_symlink_lib64_enhance_ums512_so \
    gta8wifi_symlink_lib64_hw_vulkan_ums512_so \
    gta8wifi_symlink_lib64_libOpenCL_so \
    gta8wifi_symlink_lib64_libOpenCL_so_1 \
    gta8wifi_symlink_lib64_libOpenCL_so_1_1 \
    gta8wifi_symlink_lib64_libOpenCL_so_1_2 \
    gta8wifi_symlink_lib64_libcheckkeybox_so

PRODUCT_PACKAGES += \
    android.hardware.audio.effect@2.0-impl \
    android.hardware.audio.effect@6.0-impl \
    android.hardware.audio@2.0-impl \
    android.hardware.audio@6.0-impl \
    android.hardware.graphics.composer@2.3-service \
    android.hardware.power.stats@1.0-service.mock \
    applypatch \
    enhance.ums512 \
    hwcomposer.ums512 \
    libSecC2ComponentStore \
    libcodec2_sec_aacdec \
    libcodec2_sec_imadec \
    libhyper_vnd \
    liblte \
    libpowerhal_cli \
    libsprdaudiohal@5.0 \
    libsprdaudiohal@6.0 \
    libstagefright_sprd_av1dec \
    libstagefright_sprd_deintl \
    libstagefright_sprd_h264dec \
    libstagefright_sprd_h264enc \
    libstagefright_sprd_h265dec \
    libstagefright_sprd_h265enc \
    libstagefright_sprd_mpeg4dec \
    libstagefright_sprd_mpeg4enc \
    libstagefright_sprd_vp9dec \
    libstagefright_sprd_vpxdec \
    libwvdrmengine \
    libwvhidl \
    systemDebuggerd \
    vendor.samsung.hardware.light-V1-ndk_platform \
    vendor.samsung.hardware.light-service \
    vendor.sprd.hardware.broadcastradio@2.0-service \
    vendor.unisoc.hardware.power-V1-ndk_platform \
    vendor.unisoc.hardware.power-service \
    wpa_supplicant

PRODUCT_OTA_ENFORCE_VINTF_KERNEL_REQUIREMENTS := false

PRODUCT_BUILD_VBMETA_IMAGE := false

PRODUCT_PRECOMPILED_SEPOLICY := false

PRODUCT_COPY_FILES += \
    device/samsung/gta8wifi/sepolicy/vendor/vendor_sepolicy.cil:$(TARGET_COPY_OUT_VENDOR)/etc/selinux/vendor_sepolicy.cil

PRODUCT_PACKAGES += libcomposer_res21_v30 libcomposer_res22_v30

PRODUCT_PACKAGES += \
    vndservicemanager \
    vndservice

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml

PRODUCT_COPY_FILES += \
    device/samsung/gta8wifi/vendor_fixes/etc/init/android.hardware.wifi.supplicant-service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/android.hardware.wifi.supplicant-service.rc \
    device/samsung/gta8wifi/vendor_fixes/etc/init/hostapd.android.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hostapd.android.rc \
    device/samsung/gta8wifi/vendor_fixes/etc/init/vendor.samsung.hardware.hqm@1.0-service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/vendor.samsung.hardware.hqm@1.0-service.rc \
    device/samsung/gta8wifi/vendor_fixes/etc/init/vendor.samsung.hardware.security.drk@2.0-service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/vendor.samsung.hardware.security.drk@2.0-service.rc \
    device/samsung/gta8wifi/vendor_fixes/etc/init/vendor.samsung.hardware.security.sem@1.0-service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/vendor.samsung.hardware.security.sem@1.0-service.rc \
    device/samsung/gta8wifi/vendor_fixes/etc/init/vendor.samsung.hardware.security.widevine.keyprov-service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/vendor.samsung.hardware.security.widevine.keyprov-service.rc \
    device/samsung/gta8wifi/vendor_fixes/etc/init/vendor.samsung.hardware.tlc.iccc@1.0-service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/vendor.samsung.hardware.tlc.iccc@1.0-service.rc \
    device/samsung/gta8wifi/vendor_fixes/etc/init/vendor.samsung.hardware.tlc.kg@1.1-service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/vendor.samsung.hardware.tlc.kg@1.1-service.rc \
    device/samsung/gta8wifi/vendor_fixes/etc/init/vendor.samsung.hardware.wifi@2.0-service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/vendor.samsung.hardware.wifi@2.0-service.rc \
    device/samsung/gta8wifi/vendor_fixes/etc/init/vendor.sprd.hardware.athuaqin@1.0-service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/vendor.sprd.hardware.athuaqin@1.0-service.rc \
    device/samsung/gta8wifi/vendor_fixes/etc/init/vendor.sprd.hardware.cplog_svc@1.0-service.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/vendor.sprd.hardware.cplog_svc@1.0-service.rc

PRODUCT_COPY_FILES += device/samsung/gta8wifi/vendor_fixes/ueventd.rc:$(TARGET_COPY_OUT_VENDOR)/etc/ueventd.rc
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.biometrics.face.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/android.hardware.biometrics.face.xml

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/sepolicy/system_ext/mapping/30.0.system.cil:$(TARGET_COPY_OUT_SYSTEM)/etc/selinux/mapping/30.0.cil \
    $(LOCAL_PATH)/sepolicy/system_ext/mapping/30.0.system_ext.cil:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/selinux/mapping/30.0.cil

PRODUCT_PACKAGES += \
    android.hardware.security.keymint-service.nonsecure \
    android.hardware.hardware_keystore.xml \
    com.android.hardware.gatekeeper.nonsecure

$(call inherit-product, device/samsung/gta8wifi/vendor_fixes/seccomp_policy/seccomp.mk)
$(call inherit-product, device/samsung/gta8wifi/vendor_fixes/keymaster/keymaster.mk)
$(call inherit-product, device/samsung/gta8wifi/vendor_fixes/gralloc/gralloc.mk)
$(call inherit-product, device/samsung/gta8wifi/vendor_fixes/libion_sprd/libion_sprd.mk)
$(call inherit-product, device/samsung/gta8wifi/vendor_fixes/media_codecs/media_codecs.mk)
$(call inherit-product, device/samsung/gta8wifi/vendor_fixes/omx/omx.mk)
$(call inherit-product, device/samsung/gta8wifi/vendor_fixes/bt_audio/bt_audio.mk)

# Lineage Health
$(call soong_config_set,lineage_health,charging_control_charging_path,/sys/class/power_supply/battery/batt_slate_mode)
$(call soong_config_set,lineage_health,charging_control_charging_enabled,0)
$(call soong_config_set,lineage_health,charging_control_charging_disabled,1)
$(call soong_config_set_bool,lineage_health,charging_control_supports_bypass,false)
$(call soong_config_set_bool,lineage_health,charging_control_supports_toggle,true)
$(call soong_config_set,lineage_health,fast_charge_node,/sys/class/sec/afc/afc_disable_val)
$(call soong_config_set,lineage_health,fast_charge_value_none,1)
$(call soong_config_set,lineage_health,fast_charge_value_fast_charge,0)
