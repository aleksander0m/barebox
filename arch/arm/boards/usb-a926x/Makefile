obj-y += init.o

bootstrap-$(CONFIG_MACH_USB_A9263) = usb_a9263_bootstrap.o
obj-$(CONFIG_AT91_BOOTSTRAP) += $(bootstrap-y)

lowlevel_init-$(CONFIG_MACH_USB_A9263) = usb_a9263_lowlevel_init.o

lwl-$(CONFIG_HAVE_AT91_LOWLEVEL_INIT) += $(lowlevel_init-y)
bbenv-$(CONFIG_DEFAULT_ENVIRONMENT_GENERIC) += defaultenv-usb-a926x
