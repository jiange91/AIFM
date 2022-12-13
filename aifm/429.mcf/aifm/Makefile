SHENANGO_PATH=../../../shenango
include $(SHENANGO_PATH)/shared.mk


AIFM_PATH=../../../aifm
librt_libs = $(SHENANGO_PATH)/bindings/cc/librt++.a
INC += -I$(SHENANGO_PATH)/bindings/cc -I$(SHENANGO_PATH)/ksched -I$(AIFM_PATH)/inc -I$(AIFM_PATH)/DataFrame/AIFM/include/ -I.

override CXXFLAGS += -std=gnu++2a -fconcepts -Wno-unused-function
CXXFLAGS := $(filter-out -std=gnu++17,$(CXXFLAGS))
override LDFLAGS += -lnuma


lib_src = $(wildcard ../../src/*.cpp)
lib_src := $(filter-out ../../src/tcp_device_server.cpp,$(lib_src))
lib_obj = $(lib_src:.cpp=.o)

# obj_dir = obj
# _obj = implicit.o mcf.o mcfutil.o output.o pbeampp.o pbla.o pflowup.o psimplex.o pstart.o readmin.o treeup.o internal_utils.o
# OBJ = $(addprefix $(obj_dir)/, $(_obj))

OBJ = implicit.o mcf.o mcfutil.o output.o pbeampp.o pbla.o pflowup.o psimplex.o pstart.o readmin.o treeup.o internal_utils.o

mcf:  $(OBJ) $(librt_libs) $(RUNTIME_DEPS) $(lib_obj)
	$(LDXX) -o $@ $(OBJ) $(lib_obj) $(librt_libs) $(RUNTIME_LIBS) $(LDFLAGS)

%.d: %.cpp
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@
%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all
all: $(obj_dir) mcf

.PHONY: clean
clean:
	rm -f $(OBJ) $(addsuffix .ll, $(OBJ)) mcf

echo0:
	echo $(INC)