$(info "Soc.mk")
# Allwinner H2+/H3
# Cortex A7 with NEON Processor

ARMOPS=-mfpu=neon-vfpv4 -mcpu=cortex-a7 -mfloat-abi=hard -mhard-float

CMSISOPS=-D__FPU_PRESENT=1 -D__GIC_PRESENT=1