DEFINES =WIDGET_HAVE_FLASHROM
DEFINES+=NO_HDMI_OUTPUT

EXTRA_SRCDIR+=src/flashrom src/nohdmi src/params

EXTRA_INCLUDES+=../lib-flashcode/include ../lib-dmx/include ../lib-rdm/include ../lib-usb/include
