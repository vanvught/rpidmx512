$(info "Timestamp.mk")

ifneq ($(findstring _TIME_STAMP_YEAR_,$(DEFINES)), _TIME_STAMP_YEAR_)
	DEFINES += \
		-D_TIME_STAMP_YEAR_=$(shell date +"%Y") \
		-D_TIME_STAMP_MONTH_=$(shell date +"%m" | sed 's/^0*//') \
		-D_TIME_STAMP_DAY_=$(shell date +"%d" | sed 's/^0*//')
endif