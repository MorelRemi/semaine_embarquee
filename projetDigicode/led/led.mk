LED_VERSION = 1.0
LED_SITE = $(TOPDIR)/package/led

LED_SITE_METHOD = local
LED_LICENSE = GPLv3+

LED_DEPENDENCIES = linux

define LED_BUILD_CMDS
	$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D)
endef

define LED_INSTALL_TARGET_CMDS
	$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D) modules_install
endef

$(eval $(generic-package))
