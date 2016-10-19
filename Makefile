#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:

ifeq ($(strip $(WUT_ROOT)),)
$(error "Please ensure WUT_ROOT is in your environment.")
endif

ifeq ($(findstring CYGWIN,$(shell uname -s)),CYGWIN)
ROOT := $(shell cygpath -w ${CURDIR})
WUT_ROOT := $(shell cygpath -w ${WUT_ROOT})
else
ROOT := $(CURDIR)
endif


export PORTLIBS		:=	$(DEVKITPRO)/portlibs/ppc

include $(WUT_ROOT)/rules/rpl.mk

AS      := $(PREFIX)as

TARGET   := $(notdir $(CURDIR))
BUILD    := build
SOURCE   := src \
			src/dynamic_libs \
			src/fs \
			src/game \
			src/gui \
			src/kernel \
			src/loader \
			src/menu \
			src/network \
			src/patcher \
			src/resources \
			src/settings \
			src/sounds \
			src/system \
			src/utils \
			src/video \
			src/video/shaders
INCLUDE  := src
DATA     := data \
			data/images \
			data/fonts \
			data/sounds
LIBS     := -lgcc -lcrt -lcoreinit -lproc_ui -lnsysnet -lsndcore2 -lvpad -lgx2 -lgd -lpng -lz -lfreetype -lmad -lvorbisidec

CFLAGS   += -O3 -std=gnu11 -Wall -Wextra -Wno-unused-parameter -Wno-strict-aliasing
CXXFLAGS += -O3 -std=gnu++11 -Wall -Wextra -Wno-unused-parameter -Wno-strict-aliasing

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(ROOT)/$(TARGET)
export VPATH    := $(foreach dir,$(SOURCE),$(ROOT)/$(dir)) \
                   $(foreach dir,$(DATA),$(ROOT)/$(dir))
export BUILDDIR := $(ROOT)
export DEPSDIR  := $(BUILDDIR)

CFILES    := $(foreach dir,$(SOURCE),$(notdir $(wildcard $(dir)/*.c)))
CXXFILES  := $(foreach dir,$(SOURCE),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES    := $(foreach dir,$(SOURCE),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

ifeq ($(strip $(CXXFILES)),)
export LD := $(CC)
else
export LD := $(CXX)
endif

export OFILES := $(CFILES:.c=.o) \
                 $(CXXFILES:.cpp=.o) \
                 $(SFILES:.S=.o) \
				 $(addsuffix .o,$(BINFILES))

export INCLUDES := $(foreach dir,$(INCLUDE),-I$(ROOT)/$(dir)) \
                   -I$(ROOT)/$(BUILD) \
                   -I$(PORTLIBS)/include -I$(PORTLIBS)/include/freetype2
#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIB_DIRS	:=  $(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
                    -L$(PORTLIBS)/lib

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(ROOT)/Makefile

clean:
	@echo "[RM]  $(notdir $(OUTPUT))"
	@rm -rf $(BUILD) $(OUTPUT).elf $(OUTPUT).rpx $(OUTPUT).a

else

# workaround as wut overwrites the LIBPATHS with its rules -> not cool
export LIBPATHS := $(LIBPATHS) $(LIB_DIRS)

DEPENDS	:= $(OFILES:.o=.d)

$(OUTPUT).rpx: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)


#---------------------------------------------------------------------------------
%.png.o : %.png
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
%.jpg.o : %.jpg
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
%.ttf.o : %.ttf
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
%.bin.o : %.bin
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
%.wav.o : %.wav
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
%.mp3.o : %.mp3
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

#---------------------------------------------------------------------------------
%.ogg.o : %.ogg
	@echo $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
