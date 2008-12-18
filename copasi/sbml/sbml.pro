# Begin CVS Header
#   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/sbml/sbml.pro,v $
#   $Revision: 1.21 $
#   $Name:  $
#   $Author: shoops $
#   $Date: 2008/12/18 19:09:16 $
# End CVS Header

# Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
# Properties, Inc., EML Research, gGmbH, University of Heidelberg,
# and The University of Manchester.
# All rights reserved.

# Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
# Properties, Inc. and EML Research, gGmbH.
# All rights reserved.

LIB = sbmlimport
SRC_TARGET = sbml

include(../lib.pri)
include(../common.pri)

contains(BUILD_OS, WIN32) {
  DEFINES += LIBSBML_STATIC
}

# Input
HEADERS += StdException.h \
           CSBMLExporter.h \
           ConverterASTNode.h \
           SBMLImporter.h \
           SBMLIncompatibility.h \
           SBMLReference.h \
           SBMLUtils.h \
           UnitConversionFactory.hpp \
           incompatibilities.h



SOURCES += StdException.cpp \
           CSBMLExporter.cpp \
           ConverterASTNode.cpp \
           SBMLImporter.cpp \
           SBMLIncompatibility.cpp \
           SBMLReference.cpp \
           SBMLUtils.cpp \
           UnitConversionFactory.cpp


DISTFILES += sbml.vcproj

#The following line was inserted by qt3to4
QT +=  qt3support 
