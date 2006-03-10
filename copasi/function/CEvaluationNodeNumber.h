/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/function/CEvaluationNodeNumber.h,v $
   $Revision: 1.11 $
   $Name:  $
   $Author: ssahle $ 
   $Date: 2006/03/10 15:09:43 $
   End CVS Header */

#ifndef COPASI_CEvaluationNodeNumber
#define COPASI_CEvaluationNodeNumber

/**
 * This is class for nodes presenting numbers used in an evaluation trees
 */
class CEvaluationNodeNumber : public CEvaluationNode
  {
  public:
    /**
     * Enumeration of possible node types.
     */
    enum SubType
    {
      INVALID = 0x00FFFFFF,
      DOUBLE = 0x00000000,
      INTEGER = 0x00000001,
      ENOTATION = 0x00000002,
      RATIONALE = 0x00000003
    };

    // Operations
  private:
    /**
     * Default constructor
     */
    CEvaluationNodeNumber();

  public:
    /**
     * Default constructor
     * @param const SubType & subType
     * @param const Data & data
     */
    CEvaluationNodeNumber(const SubType & subType,
                          const Data & data);

    /**
     * Copy constructor
     * @param const CEvaluationNodeNumber & src
     */
    CEvaluationNodeNumber(const CEvaluationNodeNumber & src);

    /**
     * Destructor
     */
    virtual ~CEvaluationNodeNumber();

    /**
     * Creates a new CEvaluationNodeNumber from an ASTNode.
     * @param const ASTNode* node
     * @return CEvaluationNode* return a pointer to the newly created node;
     */
    static CEvaluationNode* createNodeFromASTTree(const ASTNode& node);

    /**
     * Create a new ASTNode corresponding to this NumberNode.
     * @return ASTNode* return a pointer to the newly created node;
     */
    ASTNode* toAST() const;

    /**
     * generate display MathML recursively
     */
    virtual void writeMathML(std::ostream & out,
                             const std::vector<std::vector<std::string> > & env,
                             bool expand = true,
                             unsigned C_INT32 l = 0) const;
  };

#endif // COPASI_CEvaluationNodeNumber
