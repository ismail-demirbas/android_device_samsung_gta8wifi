SOCKO_LOCAL_PATH := device/samsung/gta8wifi/socko
PRODUCT_COPY_FILES += $(foreach f,$(wildcard $(SOCKO_LOCAL_PATH)/*.ko),$(f):$(TARGET_COPY_OUT_VENDOR)/socko/$(notdir $(f)))
PRODUCT_COPY_FILES += $(SOCKO_LOCAL_PATH)/socko.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/socko.rc
