GPIO2_VERSION = 1.0
GPIO2_SITE = $(TOPDIR)/package/gpio2
GPIO2_SITE_METHOD = local
GPIO2_LICENSE = GPLv3+

GPIO2_DEPENDENCIES = linux

define GPIO2_BUILD_CMDS
$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D)
endef

define GPIO2_INSTALL_TARGET_CMDS
$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D) modules_install
endef

$(eval $(generic-package))

