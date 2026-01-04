$(info "CppOpts.mk")

CPPOPS=-std=c++20
CPPOPS+=-Wnon-virtual-dtor -Woverloaded-virtual -Wnull-dereference -fno-rtti -fno-exceptions -fno-unwind-tables
CPPOPS+=-Wuseless-cast -Wold-style-cast
CPPOPS+=-fno-threadsafe-statics -fno-use-cxa-atexit
CPPOPS+=-Wshadow -Wshadow=local