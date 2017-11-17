COM_VERSION = 1.0
COM_SITE = $(TOPDIR)/package/com
COM_SITE_METHOD = local
COM_LICENSE = GPLv3+

COM_DEPENDENCIES = linux

define COM_BUILD_CMDS
	$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D)
endef

define COM_INSTALL_TARGET_CMDS
	$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D) modules_install
endef

$(eval $(generic-package))
