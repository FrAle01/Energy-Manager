CONTIKI_PROJECT = inverter

all: $(CONTIKI_PROJECT)

# Do not try to build on Sky because of code size limitation
PLATFORMS_EXCLUDE = sky z1

# Include CoAP resources
# MODULES_REL += ./resources

CONTIKI=../../..

# Include the CoAP implementation
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

# Add the utils directory
MODULES_REL += ./utils
MODULES_REL += ./model
MODULES_REL += ./resources
MODULES_REL += ./global_variable


TARGET_LIBFILES += -lm

CFLAGS = -Wno-unused-variable

MODULES_REL+=/home/iot_ubuntu_intel/.local/lib/python3.10/site-packages/emlearn
INC+=/home/iot_ubuntu_intel/.local/lib/python3.10/site-packages/emlearn



include $(CONTIKI)/Makefile.include