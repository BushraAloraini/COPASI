/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/CQExperimentData.cpp,v $
   $Revision: 1.6 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/11/15 23:15:44 $
   End CVS Header */

/****************************************************************************
 ** Form implementation generated from reading ui file 'CQExperimentData.ui'
 **
 ** Created: Tue Nov 15 18:09:19 2005
 **      by: The User Interface Compiler ($Id: CQExperimentData.cpp,v 1.6 2005/11/15 23:15:44 shoops Exp $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#include "CQExperimentData.h"

#include <qvariant.h>
 #include <qpushbutton.h>
 #include <qtoolbutton.h>
 #include <qlabel.h>
 #include <qlistbox.h>
 #include <qcheckbox.h>
 #include <qlineedit.h>
 #include <qbuttongroup.h>
 #include <qradiobutton.h>
 #include <qtable.h>
 #include <qlayout.h>
 #include <qtooltip.h>
 #include <qwhatsthis.h>
 #include <qimage.h>
 #include <qpixmap.h>

#include "CQExperimentData.ui.h"
static const unsigned char image0_data[] =
  {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x01,
    0xdd, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x8d, 0x93, 0xbd, 0x6a, 0x55,
    0x41, 0x10, 0xc7, 0x7f, 0xb3, 0x67, 0xf7, 0x28, 0x18, 0x44, 0x24, 0x82,
    0x56, 0x56, 0xa2, 0x4d, 0x7c, 0x01, 0x9b, 0x54, 0xe2, 0x03, 0xd8, 0x5a,
    0xf9, 0x04, 0x3e, 0x80, 0x8d, 0x45, 0xc0, 0x5a, 0xb0, 0xb1, 0xb4, 0x11,
    0x04, 0x5b, 0xb5, 0xb0, 0x92, 0x40, 0x08, 0x1a, 0x3f, 0x90, 0x34, 0x26,
    0x51, 0x50, 0x50, 0xf1, 0x92, 0xfb, 0xed, 0xfd, 0xd8, 0xdd, 0x19, 0x8b,
    0x73, 0x4e, 0x3c, 0x68, 0x72, 0x71, 0x60, 0x61, 0x77, 0x76, 0xf7, 0xb7,
    0x33, 0xfb, 0x9f, 0x91, 0xf5, 0xcd, 0xed, 0xce, 0xa9, 0x93, 0x4b, 0xc7,
    0xd4, 0x8c, 0xc6, 0xd4, 0x60, 0x3a, 0x9d, 0x11, 0xe7, 0x91, 0xce, 0x7e,
    0xdf, 0xa5, 0xa8, 0xdd, 0x5e, 0x7f, 0x7c, 0xed, 0xe6, 0x8d, 0xab, 0x1f,
    0xf8, 0xdb, 0x5e, 0xbd, 0xdd, 0x99, 0xa8, 0xaa, 0xb5, 0x47, 0xce, 0xd9,
    0xc6, 0x93, 0x68, 0x83, 0x61, 0xb4, 0x37, 0xef, 0xf7, 0xec, 0xe3, 0xee,
    0x37, 0x7b, 0xf6, 0x62, 0xeb, 0xe7, 0x83, 0x87, 0x4f, 0x57, 0xcd, 0x8c,
    0xf6, 0x70, 0xff, 0x10, 0x01, 0x11, 0xc1, 0x39, 0x10, 0xa7, 0xb8, 0xc2,
    0x71, 0xf6, 0xdc, 0x32, 0xab, 0x57, 0x56, 0x96, 0x2f, 0x5d, 0x38, 0xff,
    0xfc, 0xd1, 0x93, 0x97, 0xd7, 0xdb, 0x67, 0x0f, 0x05, 0x18, 0x20, 0x40,
    0x59, 0x7a, 0x3e, 0x7d, 0xfe, 0xc2, 0xc6, 0xc6, 0x16, 0xaf, 0xdf, 0x6d,
    0x93, 0xe2, 0xb4, 0x1c, 0x0e, 0x7a, 0x8f, 0x6f, 0xdf, 0xb9, 0x7f, 0x6b,
    0x21, 0x00, 0x40, 0x9c, 0xe0, 0x04, 0xce, 0x2c, 0x9f, 0x66, 0x1e, 0x8d,
    0x1f, 0xdf, 0xbb, 0xec, 0xec, 0x7e, 0xa5, 0x3c, 0x5e, 0x32, 0x1c, 0xf5,
    0xd6, 0x9a, 0x73, 0xfe, 0xd0, 0xcb, 0x80, 0x13, 0x21, 0x45, 0xe5, 0xf2,
    0xca, 0x45, 0x66, 0xd3, 0xc8, 0x7e, 0xb7, 0x4f, 0x67, 0xbf, 0x47, 0x08,
    0x25, 0x9b, 0x1b, 0xeb, 0x2c, 0x04, 0x98, 0x19, 0x39, 0x2b, 0x89, 0x4c,
    0xaf, 0xf7, 0x8b, 0xd1, 0x68, 0xca, 0x6c, 0x36, 0xa7, 0x0c, 0x81, 0x10,
    0x3c, 0x45, 0x51, 0x2c, 0x06, 0x24, 0x35, 0x86, 0xa3, 0x39, 0x29, 0x25,
    0x34, 0x1b, 0x65, 0x19, 0x10, 0x07, 0xaa, 0x4a, 0xf0, 0x01, 0x11, 0x8e,
    0x06, 0xc4, 0xac, 0x4c, 0x26, 0x91, 0xac, 0x55, 0x32, 0xae, 0x10, 0x3c,
    0x0e, 0x91, 0x92, 0x9c, 0x13, 0x21, 0x84, 0x3a, 0xc9, 0x06, 0x50, 0xcf,
    0xa7, 0x31, 0x23, 0x40, 0x4c, 0xb9, 0x56, 0xc0, 0xa1, 0x19, 0xc4, 0x41,
    0x12, 0x45, 0xb2, 0x02, 0x9e, 0xc2, 0x7b, 0xa4, 0x15, 0x82, 0x6f, 0x64,
    0x48, 0x49, 0x0f, 0x42, 0x93, 0x42, 0x20, 0x19, 0x82, 0x54, 0x35, 0x21,
    0x82, 0x15, 0xe0, 0x14, 0x0a, 0x81, 0x4a, 0xe8, 0xca, 0x9c, 0xb6, 0x7f,
    0x1e, 0x2a, 0x48, 0xe3, 0xac, 0xe9, 0xae, 0x70, 0x08, 0x0e, 0x11, 0xa9,
    0x5f, 0xff, 0x13, 0xc1, 0x91, 0x75, 0xf0, 0xbf, 0xe6, 0x1a, 0x82, 0x69,
    0xd5, 0x44, 0x58, 0x15, 0x60, 0xab, 0xb7, 0x30, 0xad, 0x17, 0x56, 0x49,
    0xdc, 0x4e, 0xc1, 0x37, 0x7b, 0x4b, 0x27, 0xc2, 0x81, 0x53, 0xa1, 0x52,
    0xc1, 0x20, 0x25, 0x23, 0xab, 0x91, 0x72, 0x46, 0x63, 0xc6, 0x87, 0x92,
    0xb6, 0x8e, 0xbe, 0x3f, 0x18, 0x8c, 0xd7, 0xee, 0xde, 0x4b, 0xce, 0xf9,
    0xfa, 0x11, 0x45, 0x70, 0x98, 0x29, 0x66, 0x60, 0x75, 0x67, 0x98, 0x19,
    0x98, 0x21, 0xe2, 0x18, 0x0f, 0x86, 0xb3, 0x06, 0xf0, 0x1b, 0xda, 0xc4,
    0xfd, 0x8b, 0x61, 0x0d, 0x6a, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
  };

static const unsigned char image1_data[] =
  {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x02,
    0x7c, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x7d, 0x93, 0x5d, 0x68, 0x8e,
    0x61, 0x18, 0xc7, 0x7f, 0xf7, 0xfd, 0x3c, 0xf7, 0xf3, 0xce, 0x5e, 0x66,
    0x63, 0xbe, 0x45, 0xbe, 0xb2, 0x48, 0xc9, 0x81, 0x14, 0x49, 0x42, 0x0e,
    0x70, 0xb4, 0x13, 0x07, 0x8e, 0x28, 0x25, 0xca, 0x28, 0x49, 0x51, 0x1c,
    0xcc, 0x47, 0x39, 0x21, 0x1f, 0x71, 0x20, 0x39, 0x51, 0x6a, 0x25, 0xc5,
    0x94, 0x1d, 0x88, 0xb1, 0x96, 0xcd, 0x47, 0xbe, 0xc6, 0x36, 0x9f, 0xb3,
    0x79, 0x37, 0xdb, 0xfb, 0xb1, 0x87, 0x77, 0xef, 0xf3, 0x3c, 0xf7, 0xe5,
    0xc0, 0xfb, 0xda, 0x8b, 0xe5, 0xaa, 0xab, 0xae, 0x83, 0xeb, 0xfa, 0xdf,
    0xff, 0x8f, 0x6e, 0xd5, 0xd8, 0xfc, 0xb2, 0xaf, 0xbc, 0x6c, 0x74, 0xcc,
    0x8a, 0x50, 0x28, 0x2b, 0x90, 0xcd, 0x0e, 0x11, 0xe4, 0x02, 0xfa, 0xfa,
    0x53, 0x3a, 0x0c, 0xec, 0x40, 0x32, 0xe5, 0xaf, 0xdf, 0xba, 0x65, 0xdd,
    0x73, 0xfe, 0xae, 0x47, 0x4f, 0xda, 0x7f, 0x58, 0x6b, 0xa5, 0xb8, 0xa3,
    0x28, 0x12, 0xff, 0x47, 0x20, 0xe9, 0x4c, 0x20, 0x8f, 0x9f, 0x75, 0xca,
    0xdb, 0x8e, 0x6e, 0xa9, 0x6f, 0x68, 0xed, 0xbd, 0x78, 0xe5, 0xd6, 0x2a,
    0x11, 0xa1, 0xb8, 0xf5, 0x3f, 0x88, 0x80, 0x52, 0x0a, 0xad, 0x41, 0x69,
    0x8b, 0x76, 0x34, 0x93, 0xa7, 0x54, 0xb2, 0x6a, 0xf9, 0xa2, 0xca, 0xaa,
    0x79, 0x33, 0x6f, 0x5f, 0xad, 0xbb, 0x57, 0x5d, 0xbc, 0x3b, 0x22, 0x80,
    0x00, 0x0a, 0xf0, 0x3c, 0x97, 0x77, 0xef, 0x3f, 0xd1, 0xd4, 0xd4, 0x4a,
    0xcb, 0xd3, 0x97, 0x84, 0x41, 0xd6, 0xcb, 0xa4, 0x93, 0xd7, 0x0e, 0x1e,
    0x39, 0x5b, 0xf3, 0x5f, 0x00, 0x00, 0xa5, 0x15, 0x5a, 0xc1, 0x84, 0xca,
    0x71, 0xe4, 0xac, 0xe2, 0x6b, 0x4f, 0x8a, 0xf6, 0x8e, 0x4f, 0x78, 0x25,
    0x1e, 0x99, 0xc1, 0x64, 0x6d, 0x61, 0xcf, 0x1d, 0xf1, 0x18, 0xd0, 0x4a,
    0x11, 0x06, 0x21, 0x8b, 0x16, 0x57, 0x91, 0xcb, 0x06, 0xf4, 0x65, 0x86,
    0xe8, 0xef, 0xee, 0xc6, 0x18, 0x8f, 0xe6, 0xa6, 0xc6, 0x11, 0x24, 0xd8,
    0x08, 0x5e, 0x35, 0x43, 0x5b, 0x0b, 0x16, 0x08, 0x05, 0x42, 0x63, 0x18,
    0xec, 0xec, 0x44, 0x9f, 0xd8, 0x83, 0x69, 0xa8, 0xc3, 0x94, 0xc6, 0xf1,
    0x3c, 0x83, 0xe3, 0x38, 0xbf, 0xcf, 0x86, 0x19, 0x7c, 0x7e, 0x03, 0x0f,
    0xeb, 0xc1, 0x4f, 0x22, 0x1f, 0x3b, 0xf1, 0x57, 0x54, 0x23, 0x1f, 0xda,
    0xa8, 0x38, 0x75, 0x80, 0x58, 0x43, 0x3d, 0xa3, 0x66, 0x3c, 0x20, 0x58,
    0xb8, 0x14, 0x3d, 0x71, 0x1a, 0x4a, 0x31, 0x02, 0xc0, 0x94, 0x39, 0x30,
    0x73, 0x3e, 0xdc, 0xa9, 0xc3, 0x69, 0xbb, 0x44, 0xfc, 0x51, 0x23, 0xba,
    0xbd, 0x1d, 0x73, 0xef, 0x2e, 0xc1, 0xd4, 0xd9, 0x0c, 0xec, 0x3c, 0x8c,
    0x53, 0x39, 0x09, 0xa3, 0x6c, 0x5e, 0x64, 0x01, 0x20, 0x3f, 0x67, 0xb5,
    0x8b, 0x5a, 0x59, 0x8d, 0xfa, 0x3e, 0x84, 0xb9, 0x7a, 0x9e, 0xd8, 0x8b,
    0x1b, 0x90, 0x18, 0x20, 0x58, 0xb0, 0x98, 0xfe, 0x43, 0x67, 0x08, 0x66,
    0x55, 0xe1, 0x0c, 0x0e, 0xe2, 0xb8, 0x2e, 0xaa, 0x88, 0x82, 0x5b, 0x30,
    0x21, 0x8c, 0x40, 0x19, 0x07, 0x57, 0x97, 0xa0, 0xba, 0x12, 0xf0, 0xa5,
    0x17, 0x32, 0x59, 0xe8, 0xfa, 0x86, 0xb2, 0x0a, 0xe5, 0x19, 0x1c, 0x05,
    0x8e, 0x2a, 0x04, 0x9d, 0x37, 0xd1, 0x16, 0x9c, 0x0f, 0x72, 0x78, 0x57,
    0x4e, 0x13, 0x3b, 0xbe, 0x1f, 0x7a, 0x53, 0x44, 0x4b, 0x96, 0x61, 0xe3,
    0xe3, 0x31, 0x2d, 0xaf, 0x19, 0xbf, 0x61, 0x2d, 0xb1, 0x9b, 0x37, 0x60,
    0x54, 0x69, 0xfe, 0xf5, 0x61, 0x06, 0xbf, 0x53, 0x70, 0xef, 0xd7, 0x63,
    0xce, 0x1c, 0x83, 0x9e, 0x3e, 0x72, 0x9b, 0xb7, 0x92, 0xb9, 0x7c, 0x1b,
    0xff, 0xe4, 0x05, 0xa4, 0x6c, 0x2c, 0x4e, 0x22, 0x41, 0xf9, 0xde, 0x5d,
    0xe8, 0x44, 0x0f, 0x62, 0xbc, 0x3f, 0x22, 0x1f, 0x96, 0xb0, 0x74, 0x35,
    0x6a, 0xf5, 0x26, 0x64, 0xfa, 0x5c, 0x72, 0xdb, 0x6a, 0x10, 0x11, 0x82,
    0x8d, 0x1b, 0x49, 0x5e, 0xbb, 0x4e, 0xe9, 0xbe, 0xdd, 0xf8, 0xdb, 0x77,
    0x60, 0xc7, 0x8c, 0xc5, 0x09, 0xc2, 0x3f, 0x24, 0xb8, 0x36, 0x3f, 0xc7,
    0x2b, 0xca, 0xe0, 0xe8, 0x39, 0x04, 0x30, 0x40, 0x14, 0x81, 0x08, 0x44,
    0x6b, 0x56, 0x92, 0x7b, 0xdc, 0x8a, 0x0a, 0x84, 0xb8, 0x9f, 0xc3, 0x75,
    0x0d, 0xc5, 0x39, 0xba, 0xa9, 0x74, 0xda, 0xaf, 0x3d, 0x7e, 0x3a, 0xd4,
    0xfa, 0x57, 0xa2, 0x82, 0x45, 0xa1, 0x11, 0xb1, 0x88, 0x80, 0xe4, 0x7f,
    0x86, 0x88, 0x80, 0x08, 0x4a, 0x69, 0xfc, 0x74, 0x66, 0xa8, 0x00, 0xf0,
    0x13, 0xf0, 0x32, 0x22, 0x79, 0x63, 0x5a, 0xb7, 0x6f, 0x00, 0x00, 0x00,
    0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
  };

static const unsigned char image2_data[] =
  {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x00,
    0x8f, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0xa5, 0x52, 0x49, 0x12, 0xc0,
    0x20, 0x08, 0x8b, 0x4e, 0x1f, 0xc6, 0xd3, 0x7c, 0x1a, 0x3f, 0xb3, 0x07,
    0x6d, 0x59, 0x9a, 0xee, 0x78, 0x10, 0x42, 0x58, 0x26, 0x5a, 0x00, 0x85,
    0x99, 0xe0, 0x3a, 0xce, 0x26, 0x28, 0x80, 0xa2, 0x25, 0x52, 0x9b, 0x27,
    0x62, 0x42, 0x79, 0x35, 0x12, 0xb6, 0x22, 0x75, 0x8d, 0x84, 0x34, 0x32,
    0xac, 0x62, 0x0f, 0x3c, 0x91, 0xf9, 0xfe, 0xb6, 0xfc, 0x32, 0xa6, 0x69,
    0x5a, 0xf9, 0x29, 0xe6, 0xd6, 0xfd, 0x7a, 0x9f, 0x88, 0xc8, 0x04, 0xe3,
    0x22, 0x46, 0xc7, 0xf9, 0x47, 0x4c, 0x29, 0xaf, 0xc6, 0x95, 0xd8, 0x9a,
    0x71, 0x6a, 0xc6, 0x16, 0xeb, 0xe8, 0x89, 0x32, 0x67, 0xe4, 0xe2, 0xcc,
    0xd3, 0xa1, 0x41, 0x2c, 0x7c, 0x17, 0x17, 0x00, 0x87, 0xa7, 0xe1, 0x3f,
    0x91, 0x63, 0x70, 0xe2, 0x74, 0x00, 0x3d, 0x09, 0x76, 0x83, 0xc1, 0xbe,
    0xf2, 0x57, 0x73, 0x22, 0xb6, 0x92, 0x93, 0x4f, 0xb1, 0x5f, 0xb6, 0x02,
    0x6f, 0x3b, 0x53, 0x57, 0x71, 0xe6, 0x68, 0xdf, 0x00, 0x00, 0x00, 0x00,
    0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
  };

/*
 *  Constructs a CQExperimentData as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CQExperimentData::CQExperimentData(QWidget* parent, const char* name, bool modal, WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  QImage img;
  img.loadFromData(image0_data, sizeof(image0_data), "PNG");
  image0 = img;
  img.loadFromData(image1_data, sizeof(image1_data), "PNG");
  image1 = img;
  img.loadFromData(image2_data, sizeof(image2_data), "PNG");
  image2 = img;
  if (!name)
    setName("CQExperimentData");
  setSizeGripEnabled(TRUE);
  CQExperimentDataLayout = new QVBoxLayout(this, 11, 6, "CQExperimentDataLayout");

  mpLayoutFile = new QGridLayout(0, 1, 1, 0, 3, "mpLayoutFile");

  mpBtnFileAdd = new QToolButton(this, "mpBtnFileAdd");
  mpBtnFileAdd->setMaximumSize(QSize(20, 20));
  mpBtnFileAdd->setIconSet(QIconSet(image0));

  mpLayoutFile->addWidget(mpBtnFileAdd, 0, 1);

  mpLblFile = new QLabel(this, "mpLblFile");

  mpLayoutFile->addWidget(mpLblFile, 0, 0);

  mpBoxFile = new QListBox(this, "mpBoxFile");
  mpBoxFile->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, mpBoxFile->sizePolicy().hasHeightForWidth()));
  mpBoxFile->setMinimumSize(QSize(0, 60));

  mpLayoutFile->addMultiCellWidget(mpBoxFile, 1, 1, 0, 2);

  mpBtnExperimentDelete = new QToolButton(this, "mpBtnExperimentDelete");
  mpBtnExperimentDelete->setMaximumSize(QSize(20, 20));
  mpBtnExperimentDelete->setIconSet(QIconSet(image1));

  mpLayoutFile->addWidget(mpBtnExperimentDelete, 0, 5);

  mpBtnFileDelete = new QToolButton(this, "mpBtnFileDelete");
  mpBtnFileDelete->setMaximumSize(QSize(20, 20));
  mpBtnFileDelete->setIconSet(QIconSet(image1));

  mpLayoutFile->addWidget(mpBtnFileDelete, 0, 2);

  mpBtnExperimentAdd = new QToolButton(this, "mpBtnExperimentAdd");
  mpBtnExperimentAdd->setMaximumSize(QSize(20, 20));
  mpBtnExperimentAdd->setIconSet(QIconSet(image0));

  mpLayoutFile->addWidget(mpBtnExperimentAdd, 0, 4);

  mpBoxExperiment = new QListBox(this, "mpBoxExperiment");
  mpBoxExperiment->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, mpBoxExperiment->sizePolicy().hasHeightForWidth()));
  mpBoxExperiment->setMinimumSize(QSize(0, 60));

  mpLayoutFile->addMultiCellWidget(mpBoxExperiment, 1, 1, 3, 5);

  mpLblExperiment = new QLabel(this, "mpLblExperiment");

  mpLayoutFile->addWidget(mpLblExperiment, 0, 3);
  CQExperimentDataLayout->addLayout(mpLayoutFile);

  mpLayoutExperiment = new QGridLayout(0, 1, 1, 0, 3, "mpLayoutExperiment");

  mpBtnLast = new QToolButton(this, "mpBtnLast");
  mpBtnLast->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, 0, 0, mpBtnLast->sizePolicy().hasHeightForWidth()));
  mpBtnLast->setMaximumSize(QSize(20, 20));
  mpBtnLast->setIconSet(QIconSet(image2));

  mpLayoutExperiment->addWidget(mpBtnLast, 1, 5);

  mpLblLast = new QLabel(this, "mpLblLast");
  mpLblLast->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));

  mpLayoutExperiment->addWidget(mpLblLast, 1, 3);

  mpCheckTab = new QCheckBox(this, "mpCheckTab");
  mpCheckTab->setChecked(TRUE);

  mpLayoutExperiment->addWidget(mpCheckTab, 1, 2);

  mpEditFirst = new QLineEdit(this, "mpEditFirst");
  mpEditFirst->setFrameShape(QLineEdit::LineEditPanel);
  mpEditFirst->setFrameShadow(QLineEdit::Sunken);

  mpLayoutExperiment->addWidget(mpEditFirst, 0, 4);

  mpEditHeader = new QLineEdit(this, "mpEditHeader");
  mpEditHeader->setEnabled(FALSE);

  mpLayoutExperiment->addWidget(mpEditHeader, 2, 4);

  mpLblName = new QLabel(this, "mpLblName");
  mpLblName->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));

  mpLayoutExperiment->addWidget(mpLblName, 0, 0);

  mpEditSeparator = new QLineEdit(this, "mpEditSeparator");
  mpEditSeparator->setEnabled(FALSE);
  mpEditSeparator->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, mpEditSeparator->sizePolicy().hasHeightForWidth()));

  mpLayoutExperiment->addWidget(mpEditSeparator, 1, 1);

  mpBtnFirst = new QToolButton(this, "mpBtnFirst");
  mpBtnFirst->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, 0, 0, mpBtnFirst->sizePolicy().hasHeightForWidth()));
  mpBtnFirst->setMaximumSize(QSize(20, 20));
  mpBtnFirst->setIconSet(QIconSet(image2));

  mpLayoutExperiment->addWidget(mpBtnFirst, 0, 5);

  mpBtnGroup = new QButtonGroup(this, "mpBtnGroup");
  mpBtnGroup->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, mpBtnGroup->sizePolicy().hasHeightForWidth()));
  mpBtnGroup->setFrameShape(QButtonGroup::NoFrame);
  mpBtnGroup->setAlignment(int(QButtonGroup::AlignTop));
  mpBtnGroup->setColumnLayout(0, Qt::Vertical);
  mpBtnGroup->layout()->setSpacing(6);
  mpBtnGroup->layout()->setMargin(0);
  mpBtnGroupLayout = new QVBoxLayout(mpBtnGroup->layout());
  mpBtnGroupLayout->setAlignment(Qt::AlignTop);

  mpBtnSteadystate = new QRadioButton(mpBtnGroup, "mpBtnSteadystate");
  mpBtnSteadystate->setChecked(TRUE);
  mpBtnGroupLayout->addWidget(mpBtnSteadystate);

  mpBtnTimeCourse = new QRadioButton(mpBtnGroup, "mpBtnTimeCourse");
  mpBtnGroupLayout->addWidget(mpBtnTimeCourse);

  mpLayoutExperiment->addMultiCellWidget(mpBtnGroup, 0, 2, 7, 7);

  mpBtnHeader = new QToolButton(this, "mpBtnHeader");
  mpBtnHeader->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, 0, 0, mpBtnHeader->sizePolicy().hasHeightForWidth()));
  mpBtnHeader->setMaximumSize(QSize(20, 20));
  mpBtnHeader->setIconSet(QIconSet(image2));

  mpLayoutExperiment->addWidget(mpBtnHeader, 2, 5);

  mpEditLast = new QLineEdit(this, "mpEditLast");

  mpLayoutExperiment->addWidget(mpEditLast, 1, 4);

  mpLblSeperator = new QLabel(this, "mpLblSeperator");
  mpLblSeperator->setMinimumSize(QSize(0, 0));
  mpLblSeperator->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));

  mpLayoutExperiment->addWidget(mpLblSeperator, 1, 0);

  mpCheckHeader = new QCheckBox(this, "mpCheckHeader");

  mpLayoutExperiment->addWidget(mpCheckHeader, 2, 6);

  mpEditName = new QLineEdit(this, "mpEditName");

  mpLayoutExperiment->addMultiCellWidget(mpEditName, 0, 0, 1, 2);

  mpLblFirst = new QLabel(this, "mpLblFirst");
  mpLblFirst->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));

  mpLayoutExperiment->addWidget(mpLblFirst, 0, 3);

  mpLblHeader = new QLabel(this, "mpLblHeader");
  mpLblHeader->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));

  mpLayoutExperiment->addWidget(mpLblHeader, 2, 3);
  CQExperimentDataLayout->addLayout(mpLayoutExperiment);

  mpTable = new QTable(this, "mpTable");
  mpTable->setNumCols(mpTable->numCols() + 1);
  mpTable->horizontalHeader()->setLabel(mpTable->numCols() - 1, tr("Column Name"));
  mpTable->setNumCols(mpTable->numCols() + 1);
  mpTable->horizontalHeader()->setLabel(mpTable->numCols() - 1, tr("Type"));
  mpTable->setNumCols(mpTable->numCols() + 1);
  mpTable->horizontalHeader()->setLabel(mpTable->numCols() - 1, tr("Hidden"));
  mpTable->setNumCols(mpTable->numCols() + 1);
  mpTable->horizontalHeader()->setLabel(mpTable->numCols() - 1, tr("<>"));
  mpTable->setNumCols(mpTable->numCols() + 1);
  mpTable->horizontalHeader()->setLabel(mpTable->numCols() - 1, tr("Model Object"));
  mpTable->setNumCols(mpTable->numCols() + 1);
  mpTable->horizontalHeader()->setLabel(mpTable->numCols() - 1, tr("Hidden"));
  mpTable->setNumRows(3);
  mpTable->setNumCols(6);
  mpTable->setSelectionMode(QTable::NoSelection);
  CQExperimentDataLayout->addWidget(mpTable);

  mpLayoutButton = new QHBoxLayout(0, 0, 6, "mpLayoutButton");

  mpBtnOK = new QPushButton(this, "mpBtnOK");
  mpLayoutButton->addWidget(mpBtnOK);

  mpBtnRevert = new QPushButton(this, "mpBtnRevert");
  mpLayoutButton->addWidget(mpBtnRevert);

  mpBtnCancel = new QPushButton(this, "mpBtnCancel");
  mpLayoutButton->addWidget(mpBtnCancel);
  CQExperimentDataLayout->addLayout(mpLayoutButton);
  languageChange();
  resize(QSize(398, 377).expandedTo(minimumSizeHint()));
  clearWState(WState_Polished);

  // signals and slots connections
  connect(mpBtnFirst, SIGNAL(clicked()), this, SLOT(slotFirst()));
  connect(mpBtnLast, SIGNAL(clicked()), this, SLOT(slotLast()));
  connect(mpBtnHeader, SIGNAL(clicked()), this, SLOT(slotHeader()));
  connect(mpCheckHeader, SIGNAL(toggled(bool)), this, SLOT(slotCheckHeader(bool)));
  connect(mpCheckTab, SIGNAL(toggled(bool)), this, SLOT(slotCheckTab(bool)));
  connect(mpBtnSteadystate, SIGNAL(toggled(bool)), this, SLOT(slotExprimentType(bool)));
  connect(mpBtnOK, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(mpBtnRevert, SIGNAL(clicked()), this, SLOT(slotRevert()));
  connect(mpBtnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
  connect(mpBtnFileAdd, SIGNAL(clicked()), this, SLOT(slotFileAdd()));
  connect(mpBtnFileDelete, SIGNAL(clicked()), this, SLOT(slotFileDelete()));
  connect(mpBtnExperimentAdd, SIGNAL(clicked()), this, SLOT(slotExperimentAdd()));
  connect(mpBtnExperimentDelete, SIGNAL(clicked()), this, SLOT(slotExperimentDelete()));
  connect(mpBoxFile, SIGNAL(currentChanged(QListBoxItem*)), this, SLOT(slotFileChanged(QListBoxItem*)));
  connect(mpBoxExperiment, SIGNAL(currentChanged(QListBoxItem*)), this, SLOT(slotExperimentChanged(QListBoxItem*)));
  connect(mpEditHeader, SIGNAL(lostFocus()), this, SLOT(slotUpdateTable()));
  connect(mpEditSeparator, SIGNAL(textChanged(const QString&)), this, SLOT(slotSeparator()));
  init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
CQExperimentData::~CQExperimentData()
{
  destroy();
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CQExperimentData::languageChange()
{
  setCaption(tr("Experimental Data"));
  mpBtnFileAdd->setText(QString::null);
  mpLblFile->setText(tr("File"));
  mpBoxFile->clear();
  mpBoxFile->insertItem(QString::null);
  mpBtnExperimentDelete->setText(QString::null);
  mpBtnFileDelete->setText(QString::null);
  mpBtnExperimentAdd->setText(QString::null);
  mpBoxExperiment->clear();
  mpBoxExperiment->insertItem(QString::null);
  mpLblExperiment->setText(tr("Experiment"));
  mpBtnLast->setText(QString::null);
  mpLblLast->setText(tr("Last Row"));
  mpCheckTab->setText(tr("<tab>"));
  mpLblName->setText(tr("Name"));
  mpBtnFirst->setText(QString::null);
  mpBtnGroup->setTitle(tr("Experiment Type"));
  mpBtnSteadystate->setText(tr("Steady State"));
  mpBtnTimeCourse->setText(tr("Time Course"));
  mpBtnHeader->setText(QString::null);
  mpLblSeperator->setText(tr("Separator"));
  mpCheckHeader->setText(QString::null);
  mpLblFirst->setText(tr("First Row"));
  mpLblHeader->setText(tr("Header"));
  mpTable->horizontalHeader()->setLabel(0, tr("Column Name"));
  mpTable->horizontalHeader()->setLabel(1, tr("Type"));
  mpTable->horizontalHeader()->setLabel(2, tr("Hidden"));
  mpTable->horizontalHeader()->setLabel(3, tr("<>"));
  mpTable->horizontalHeader()->setLabel(4, tr("Model Object"));
  mpTable->horizontalHeader()->setLabel(5, tr("Hidden"));
  mpBtnOK->setText(tr("&OK"));
  mpBtnOK->setAccel(QKeySequence(tr("Alt+O")));
  mpBtnRevert->setText(tr("&Revert"));
  mpBtnRevert->setAccel(QKeySequence(tr("Alt+R")));
  mpBtnCancel->setText(tr("&Cancel"));
  mpBtnCancel->setAccel(QKeySequence(tr("Alt+C")));
}
