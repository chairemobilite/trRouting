#ifndef TR_ROUTING_RESULT_VISITOR
#define TR_ROUTING_RESULT_VISITOR

namespace TrRouting
{

  class SingleCalculationResult;
  class AlternativesResult;
  class AllNodesResult;

  class BoardingStep;
  class UnboardingStep;
  class WalkingStep;

  /**
   * @brief Base class to visit the routing result hierarchy of classes. Most calculator 
   * methods will return abstract routing result types, the visitor is needed to process 
   * the result, for example to retrieve data from a calculation's steps or return a 
   * response to the user.
   */
  class ResultVisitorBase {
  public:
    virtual void visitSingleCalculationResult(const SingleCalculationResult& result) = 0;
    virtual void visitAlternativesResult(const AlternativesResult& result) = 0;
    virtual void visitAllNodesResult(const AllNodesResult& result) = 0;
  };

  /**
   * @brief A helper visitor class which returns a value of a certain type. As most visitor 
   * needs to return some data, this behavior is explicited with this visitor class
   * 
   * @tparam T The type of the return result
   */
  template <class T>
  class ResultVisitor : public ResultVisitorBase {
  public:
    virtual T getResult() = 0;
  };

  /**
   * @brief Base class to visit a routing result step.
   */
  class StepVisitorBase {
  public:
    virtual void visitBoardingStep(const BoardingStep& step) = 0;
    virtual void visitUnboardingStep(const UnboardingStep& step) = 0;
    virtual void visitWalkingStep(const WalkingStep& step) = 0;
  };

  /**
   * @brief A helper visitor class which returns a value of a certain type. As most visitor 
   * needs to return some data, this behavior is explicited with this visitor class
   * 
   * @tparam T The type of the return result
   */
  template <class T>
  class StepVisitor : public StepVisitorBase {
  public:
    virtual T getResult() = 0;
  };
}

#endif // TR_ROUTING_RESULT_VISITOR