/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/DataModel.txt.h,v $
   $Revision: 1.20.2.1 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2006/02/10 19:08:09 $
   End CVS Header */

static char DataModeltxt[] =
  "0:1:Model\n"
  "1:11:Biochemical\n"
  "11:111:Compartments\n"
  "11:112:Metabolites\n"
  "11:113:Moiety\n"
  "11:114:Reactions\n"
  "11:115:Global parameters\n"
  "11:116:Parameter overview\n"
  "1:12:Mathematical\n"
  "12:121:Compartment Symbols\n"
  "12:122:Function Symbols\n"
  "12:123:Constant Symbols\n"
  "12:124:Fixed Metabolite Symbols\n"
  "12:125:Metabolite Symbols\n"
  "12:126:Differential Equations\n"
  "0:2:Tasks\n"
  "2:21:Steady-State\n"
  "21:211:Result\n"
  "2:22:Stoichiometry\n"
  "22:221:Elementary Modes\n"
  "22:222:Mass Conservation\n"
  "2:23:Time Course\n"
  "23:231:Result\n"
  "2:24:Metabolic Control Analysis\n"
  "24:241:Result\n"
#ifdef COPASI_TSS
  "2:25:Time scale separation\n"
#endif
  "0:3:Multiple Task\n"
  "3:31:Parameter Scan\n"
  "3:32:Optimization\n"
  "32:321:Result\n"
  "3:33:Parameter Estimation\n"
#ifdef COPASI_SENS
  "3:34:Sensitivities\n"
#endif
  "0:4:Output\n"
  "4:42:Plots\n"
  "4:43:Reports\n"
  "0:5:Functions\n"
;
