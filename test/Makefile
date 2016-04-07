# This makefile can help you to build test inputs for your callgrapher tool
# and produce easier to interpret images from its output. If you wish to
# build call graphs using your tool as well, you must define the CALLGRAPHER
# variable below to contain the path to your compiled callgrapher binary.
# If you wish to compile your own tests, set the paths for CLANG and OPT,
# as well.
#
# To build LLVM assembly files from C source files:
#   make llvmasm
#
# To build GraphViz format call graphs using your tool:
#   make graphviz
#
# To convert the GraphViz format call graphs into pictures using dot:
#   make pictures
#
# To remove previous output & intermediate files:
#   make clean
#

CALLGRAPHER  := ../../callgrapher/bin/callgrapher
CSV_TO_GV    := ../scripts/csv_to_gv.py
CLANG        := clang
OPT          := opt
DOT          := dot
RM           := rm
SOURCE_FILES := $(sort $(wildcard c/*.c))
ASM_FILES    := $(addprefix ll/,$(notdir $(SOURCE_FILES:.c=.ll)))
GV_FILES     := $(addprefix gv/,$(notdir $(ASM_FILES:.ll=.gv)))
PICTURES     := $(addprefix img/,$(notdir $(GV_FILES:.gv=.png)))


all: $(PICTURES)
llvmasm: $(ASM_FILES)
graphviz: $(GV_FILES)
pictures: $(PICTURES)


ll/%.ll: c/%.c
	$(CLANG) -g -emit-llvm -S $< -o - | $(OPT) -mem2reg -S -o $@

gv/%.gv: ll/%.ll
	$(CALLGRAPHER) $< | $(CSV_TO_GV) > $@

img/%.png: gv/%.gv
	$(DOT) -Tpng $< -o $@

clean:
	$(RM) -f $(PICTURES) $(GV_FILES)

veryclean:
	$(RM) -f $(PICTURES) $(GV_FILES) $(ASM_FILES)

