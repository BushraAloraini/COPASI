/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/utilities/messages.h,v $
   $Revision: 1.47 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/09/15 18:45:24 $
   End CVS Header */

const MESSAGES Messages[] =
  {
    // CReadConfig
    {MCReadConfig + 1, "CReadConfig (1): Variable '%s' not found in '%s(%d)'."},
    {MCReadConfig + 2, "CReadConfig (2): Cannot open file '%s'."},
    {MCReadConfig + 3, "CReadConfig (3): Cannot read file '%s'."},
    {MCReadConfig + 4, "CReadConfig (4): Cannot close file '%s'."},
    {MCReadConfig + 5, "CReadConfig (5): Invalid type '%s' for  Variable '%s'."},

    // CKinFunction
    {MCKinFunction + 1, "CKinFunction (1): Cannot find identifier '%s'"
     " in function '%s'."},
    {MCKinFunction + 2, "CKinFunction (2): Missing operant in function '%s'."},

    // CRange
    {MCRange + 1, "CRange (1): Invalid range ('%d', '%d')."},

    // CCopasiVector
    {MCCopasiVector + 1, "CCopasiVector (1): Object '%s' not found."},
    {MCCopasiVector + 2, "CCopasiVector (2): Object '%s' already exists."},

    // CFunctionParameters
    {MCFunctionParameters + 1, "CFunctionParameters (1): The usage '%s' "
     "is not unique for a vector type parameter ('%d')."},
    {MCFunctionParameters + 2, "CFunctionParameters (2): No parameter with "
     "usage '%s' with index >= '%ld'."},

    // CMassAction
    {MCMassAction + 1, "CMassAction (1): The function Mass Action reversibility "
     "must be either TRUE or FALSE."},

    // CCopasiMethod
    {MCCopasiMethod + 1, "CCopasiMethod (1): No parameter list "
     "found for name '%s' and type '%s'."},
    {MCCopasiMethod + 2, "CCopasiMethod (2): pProblem == NULL"},
    {MCCopasiMethod + 3, "CCopasiMethod (3): Model is not set in problem"},

    // CReaction
    {MCReaction + 1, "CReaction (1): Function '%s' not found."},
    {MCReaction + 2, "CReaction (2): In Reaction '%s' the compartment could not be guessed."},
    {MCReaction + 3, "CReaction (3): Reaction '%s' has no substrates and no products."},
    {MCReaction + 4, "CReaction (4): Only Metabolites, Compartments or Parameter object nodes are allowed in kinetic functions."},
    {MCReaction + 5, "CReaction (5): Nodes of type %s are not implemented yet."},
    {MCReaction + 6, "CReaction (6): Nodes of type VARIABLE must not appear in an expression."},
    {MCReaction + 7, "CReaction (7): CMetab object '%s' is neither substrate, product nor modifier to the reaction."},
    {MCReaction + 8, "CReaction (8): Could not find variable with name \"%s\"."},
    {MCReaction + 9, "CReaction (9): Could not find object for key \"%s\"."},
    {MCReaction + 10, "CReaction (10): Parameter \"%s\" is a vector."},

    // CChemEq
    {MCChemEq + 1, "CChemEq (1): No Substrates and no Products."},
    {MCChemEq + 2, "CChemEq (2): Substrates in different Compartments."},
    {MCChemEq + 3, "CChemEq (3): No Substrates and Products are in different compartments."},

    // CMethodParameter
    {MCParameter + 1, "CMethodParameter (1): Invalid value '%f' for '%s' "
     "which is of type '%s'."},

    // CTrajectoryMethod
    {MCTrajectoryMethod + 1, "CTrajectoryMethod (1): Hybrid simulation not "
     "applicable, since the stoichiometry contains a non-integer."},
    {MCTrajectoryMethod + 2, "CTrajectoryMethod (2): Hybrid simulation not "
     "applicable, since reversible reactions exists."},
    {MCTrajectoryMethod + 3, "CTrajectoryMethod (3): Hybrid simulation not "
     "applicable, since more than one compartment is involved."},
    {MCTrajectoryMethod + 4, "CTrajectoryMethod (4): mLowerStochLimit ('%.2f') is greater than mUpperStochLimit ('%.2f')."},
    {MCTrajectoryMethod + 6, "CTrajectoryMethod (6): Deterministic integration failed. "
     "LSODA reported: %s \n\nPlease see result for indications of numerical instability."},
    {MCTrajectoryMethod + 7, "CTrajectoryMethod (7): pProblem == NULL"},
    {MCTrajectoryMethod + 8, "CTrajectoryMethod (8): Problem is not a trajectory problem."},
    {MCTrajectoryMethod + 9, "CTrajectoryMethod (9): Negative time steps not possible with stochastic simulation."},
    {MCTrajectoryMethod + 10, "CTrajectoryMethod (10): The tau-Leap Method encountered numerical problems.\nYou can try to reduce the tau-value."},
    {MCTrajectoryMethod + 11, "CTrajectoryMethod (11): Invalid tau-value ('%.2f'). Tau must have a positive value."},
    {MCTrajectoryMethod + 12, "CTrajectoryMethod (12): Internal step limit exceeded."},

    // XML Package
    {MCXML + 1, "XML (1): Required attribute '%s' not found (line: '%d')."},
    {MCXML + 2, "XML (2): XML error (line: '%d', column: '%d'): '%s'."},
    {MCXML + 3, "XML (3): Unknown element '%s' encountered at line '%d'."},
    {MCXML + 4, "XML (4): Unknown parameter '%s' encountered at line '%d'."},
    {MCXML + 5, "XML (5): Unknown task type '%s' encountered at line '%d'."},

    // CCopasiMessage
    {MCCopasiMessage + 1, "Message (1): No more messages."},

    // Configuration
    {MCConfiguration + 1, "Configuration (1): COPASI directory is not set. Some features might not be working correctly.\n"
     "                   Please set the environment variable COPASIDIR or use the\n"
     "                   commandline options -c COPASIDIR or --copasidir COPASIDIR\n"
     "                   to point to the COPASI installation directory."},

    // Optimization
    {MCOptimization + 1, "Optimization (1): Object '%s' not found."},
    {MCOptimization + 2, "Optimization (2): Lower Bound '%s' not found."},
    {MCOptimization + 3, "Optimization (3): Upper Bound '%s' not found."},
    {MCOptimization + 4, "Optimization (4): Invalid Relationship: '%s'."},
    {MCOptimization + 5, "Optimization (5): Invalid Objective Functions."},
    {MCOptimization + 6, "Optimization (6): No adjustable Parameters specified."},

    // SBML
    // {MCSBML + 1, "SBML (1): Available"},
    {MCSBML + 2, "SBML (2): The SBML document contains no model."},
    {MCSBML + 3, "SBML (3): The SBML document contains rules that were ignored."},
    {MCSBML + 4, "SBML (4): The SBML document contains events that were ignored."},
    {MCSBML + 5, "SBML (5): Compartment %s referenced by species %s does not exist."},
    {MCSBML + 6, "SBML (6): Annotations in SBase element of SBML Level 1 files, this is invalid and has been ignored."},
    {MCSBML + 7, "SBML (7): One or more model values have not been set."},
    {MCSBML + 8, "SBML (8): Expression tree for kinetic of reaction '%s' could not be converted."},
    {MCSBML + 9, "SBML (9): Could not set function from expression in reaction '%s'."},
    {MCSBML + 10, "SBML (10): Copasi does not support stoichiometric expressions yet."},
    {MCSBML + 11, "SBML (11): Function \"%s\" does not start with a Lambda element."},
    {MCSBML + 12, "SBML (12): Function \"%s\" contains invalid parameter list."},
    {MCSBML + 13, "SBML (13): Could not read function definition for function \"%s\"."},
    {MCSBML + 14, "SBML (14): Could not import function with id \"%s\"."},
    {MCSBML + 15, "SBML (15): Could not find function with id \"%s\"."},
    {MCSBML + 16, "SBML (16): Loop found in function. That means a function calls itself either directly or indirectly."},
    {MCSBML + 17, "SBML (17): Single compartment reaction found where the kinetic law does not include a multiplication by the volume."},

    {MCTrajectoryProblem + 1, "CTrajectoryProblem (1): Invalid step size = '%f'."},
    {MCTrajectoryProblem + 2, "CTrajectoryProblem (2): The step number '%f' "
     "exceeds the limit. The step number and step size have been adjusted to avoid over flow."},
    {MCTrajectoryProblem + 3, "CTrajectoryProblem (3): The step size '%f' "
     "is to small for the machine accuracy. The step number and step size have been adjusted."},

    // Directory Entries
    {MCDirEntry + 1, "DirEntry (1): Directory entry '%s' already exists."},
    {MCDirEntry + 2, "DirEntry (2): Directory entry '%s' is read-only."},

    // MathML
    {MCMathML + 1, "MathML (1): Unsupported element '%s'."},
    {MCMathML + 2, "MathML (2): Unknown element in MathML expression."},
    {MCMathML + 3, "MathML (3): MINUS operator can only have one or two arguments."},
    {MCMathML + 4, "MathML (4): DIVIDE and POWER operator can only have 2 arguments."},

    // Function
    {MCFunction + 1, "Function (1): Parser error after position: '%d'."},
    {MCFunction + 2, "Function (2): Lexer error after position: '%d'."},
    {MCFunction + 3, "Function (3): Compile error after position: '%d'."},

    {MCEvaluationNodeObject + 1, "CEvaluationNodeObject (1): Only references to compartment, species, parameters and reaction are allowed in expression."},

    {MCCopasiTask + 1, "CCopasiTask (1): No problem defined for Task '%s'."},
    {MCCopasiTask + 2, "CCopasiTask (2): No model associated for Task '%s'."},
    {MCCopasiTask + 3, "CCopasiTask (3): No method defined for Task '%s'."},
    {MCCopasiTask + 4, "CCopasiTask (4): Error compiling Model '%s'."},
    {MCCopasiTask + 5, "CCopasiTask (5): No target defined for Report '%s'."},
    {MCCopasiTask + 6, "CCopasiTask (6): Problems compiling Report '%s'."},

    // This must be the last element of the message list! Do not delete!
    {0, NULL}
  };
