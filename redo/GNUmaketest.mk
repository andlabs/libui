# 22 april 2015
# should never be invoked directly, only ever from the main makefile

include test/GNUmakeinc.mk

baseHFILES = \
	$(testHFILES)

baseCFILES = $(testCFILES)

baseCFLAGS = $(testCFLAGS)
baseLDFLAGS = $(osLIB) $(testLDFLAGS)
baseSUFFIX = $(osEXESUFFIX)

include GNUbase.mk
