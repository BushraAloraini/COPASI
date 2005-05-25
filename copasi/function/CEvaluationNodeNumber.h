/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/function/CEvaluationNodeNumber.h,v $
   $Revision: 1.3 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/05/25 17:56:03 $
   End CVS Header */

#ifndef COPASI_CEvaluationNodeNumber
#define COPASI_CEvaluationNodeNumber

/**
 * This is class for nodes presenting numbers used in an evaluation trees
 */
class CEvaluationNodeNumber : public CEvaluationNode
  {
    /**
     * friend declaration
     */
    friend
    CEvaluationNode * CEvaluationNode::create(const Type & type,
        const Data & data);

  public:
    /**
     * Enumeration of possible node types.
     */
    enum SubType
    {
      INVALID = 0x00FFFFFF,
      DOUBLE = 0x00000000,
      INTEGER = 0x00000001
    };

    // Operations
  private:
    /**
     * Default constructor
     */
    CEvaluationNodeNumber();

    /**
     * Default constructor
     * @param const Type & subType
     * @param const Data & data
     */
    CEvaluationNodeNumber(const Type & subType,
                          const Data & data);

  public:
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
     * Retrieve the value of the node
     * @return const C_FLOAT64 & value
     */
    virtual inline const C_FLOAT64 & value() const {return mValue;}
  };

#endif // COPASI_CEvaluationNodeNumber
