DIGIGAME_VERSION = 1.0
DIGIGAME_SITE = $(TOPDIR)/package/digigame
DIGIGAME_SITE_METHOD = local
DIGIGAME_LICENSE = GPLv3+

define DIGIGAME_BUILD_CMDS
	CC=$(TARGET_CC) $(MAKE) -C $(@D)
endef

define DIGIGAME_INSTALL_TARGET_CMDS
	prefix=$(TARGET_DIR) $(MAKE) -C $(@D) install
endef

$(eval $(generic-package))

