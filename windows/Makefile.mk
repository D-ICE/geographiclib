# $Id: Makefile.mk 6976 2011-02-21 11:05:24Z karney $

PROGRAMS = GeoConvert TransverseMercatorTest CartConvert Geod EquidistantTest \
	GeoidEval Planimeter

VSPROJECTS = $(addsuffix -vc8.vcproj,Geographic $(PROGRAMS)) \
	$(addsuffix -vc9.vcproj,Geographic $(PROGRAMS)) \
	$(addsuffix -vc10.vcxproj,Geographic $(PROGRAMS))

all:
	@:
install:
	@:
clean:
	@:
list:
	@echo GeographicLib-vc8.sln GeographicLib-vc9.sln $(VSPROJECTS)

.PHONY: all install list clean
