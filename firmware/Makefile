# Copyright (c) 2015, M.Naruoka (fenrir)
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification, 
# are permitted provided that the following conditions are met:
# 
# - Redistributions of source code must retain the above copyright notice, 
#   this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright notice, 
#   this list of conditions and the following disclaimer in the documentation 
#   and/or other materials provided with the distribution.
# - Neither the name of the naruoka.org nor the names of its contributors 
#   may be used to endorse or promote products derived from this software 
#   without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

PACKAGE = gpibusb_mini.hex

BIN_PATH = /usr/bin:/usr/local/bin
CXX = sdcc
AS = asx8051
CPPFLAGS = -V --use-stdout -D__SDCC__ -D__F380_VER__
CFLAGS = -V --debug --stack-auto --nooverlay --model-small --use-stdout -D__SDCC__ -D__F380_VER__ --opt-code-speed #-mmcs51
LFLAGS = -V --debug --use-stdout --stack-auto --model-small --iram-size 0x0100 --xram-loc 0x0000 --xram-size 0x0400 --code-size 0x7a00  #-mmcs51
ASFLAGS = -plosgff
INCLUDES = #-I
LIBS = #-L
BUILD_DIR = build_by_sdcc

SRCS_C = \
	$(shell ls *.c)
SRCS_ASM = \
	$(shell ls *.asm)
	
OBJS = $(SRCS_C:.c=.rel) $(SRCS_ASM:.asm=.rel)

all : $(BUILD_DIR) depend $(PACKAGE)

# ヘッダーファイルの依存関係
depend: $(SRCS_C)
	export PATH=$(BIN_PATH):$$PATH; \
	rm -f $(BUILD_DIR)/depend.inc; \
	for i in $^; do \
		$(CXX) -E -MMM $(INCLUDES) $(CPPFLAGS) $$i >> tempfile; \
		if ! [ $$? = 0 ]; then \
			rm -f tempfile; \
			exit 1; \
		fi; \
	done; \
	cat tempfile | sed -e 's/^+.*//g' -e 's/[^\.]*\.rel/$(BUILD_DIR)\/&/g' >> $(BUILD_DIR)/depend.inc; \
	rm -f tempfile

-include $(BUILD_DIR)/depend.inc

$(BUILD_DIR)/%.rel : %.c
	export PATH=$(BIN_PATH):$$PATH; \
	$(CXX) -c $(CFLAGS) $(INCLUDES) -o $@ $<

$(BUILD_DIR)/%.rel : %.asm
	export PATH=$(BIN_PATH):$$PATH; \
	cp $< $(BUILD_DIR)/; \
	cd $(BUILD_DIR); \
	$(AS) $(ASFLAGS) $<

$(PACKAGE) : $(patsubst %,$(BUILD_DIR)/%,$(PACKAGE))

$(BUILD_DIR)/$(PACKAGE) : $(patsubst %,$(BUILD_DIR)/%,$(OBJS))
	export PATH=$(BIN_PATH):$$PATH; \
	$(CXX) $(LFLAGS) $(INCLUDES) -o $@ $(LIBS) $^; \
	if [ -f $(basename $@) ]; then mv $(basename $@) $(basename $@).omf; fi

$(BUILD_DIR) :
	mkdir $@

clean :
	rm -f $(BUILD_DIR)/*

run : all

.PHONY : clean all depend

