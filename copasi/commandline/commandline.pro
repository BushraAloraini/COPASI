######################################################################
# $Revision: 1.8.2.1 $ $Author: shoops $ $Date: 2006/05/18 15:43:52 $  
######################################################################

LIB = commandline

include(../lib.pri)
include(../common.pri)

#Input
HEADERS += CConfigurationFile.h \
           COptionParser.h \
           COptions.h

SOURCES += CConfigurationFile.cpp \
           COptionParser.cpp \
           COptions.cpp

contains(BUILD_PARSER, yes) {
  CLOPP = $$system(which clo++)
  count(CLOPP, 1) {
    1.target = COptionParser.cpp
    1.depends = COptionParser.xml
    1.commands = \
          rm $@ $*.h; \
          clo++ -o c++ $< && \
          cat $@ | \
          sed -e 's/compare([^,]*,[^,]*, *\([^,]*\) *,[^)]*)/compare(\1)/g' \
              > $@.tmp && \
          mv $@.tmp $@; \
          ../../cvs_admin/c++style $@; \
          ../../cvs_admin/c++style $*.h
        
    QMAKE_EXTRA_UNIX_TARGETS += 1
  }
}

DISTFILES += \
             commandline.dsp \
             COptionParser.xml
