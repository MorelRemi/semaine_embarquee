HELLOWORLD_VERSION = 1.0
HELLOWORLD_SITE = $(TOPDIR)/package/helloworld
HELLOWORLD_SITE_METHOD = local
HELLOWORLD_LICENSE = GPLv3+

define HELLOWORLD_BUILD_CMDS
	CC=$(TARGET_CC) $(MAKE) -C $(@D)
endef

define HELLOWORLD_INSTALL_TARGET_CMDS
	prefix=$(TARGET_DIR) $(MAKE) -C $(@D) install
endef

$(eval $(generic-package))
