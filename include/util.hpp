#ifndef TR_UTIL
#define TR_UTIL

#include <string>
#include <vector>
#include <random>
#include <iterator>

namespace TrRouting
{
namespace Util
{
  
  // From https://gist.github.com/cbsmith/5538174
  template <typename RandomGenerator = std::mt19937>
  struct random_selector
  {
    //On most platforms, you probably want to use std::random_device("/dev/urandom")()
    random_selector(RandomGenerator g = RandomGenerator(std::random_device()()))
      : gen(g) {}
  
    template <typename Iter>
    Iter select(Iter start, Iter end) {
      std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
      std::advance(start, dis(gen));
      return start;
    }
    
    double operator()(double min, double max) {
      std::uniform_real_distribution<double> dis(min, max);
      return dis(gen);
    }
    
    //convenience function
    template <typename Iter>
    Iter operator()(Iter start, Iter end) {
      return select(start, end);
    }
  
    //convenience function that works on anything with a sensible begin() and end(), and returns with a ref to the value type
    template <typename Container>
    auto operator()(const Container& c) -> decltype(*begin(c))& {
      return *select(begin(c), end(c));
    }
  
  private:
    RandomGenerator gen;
  };
  
  static random_selector<> randomSelector{};
  
}
}

#endif // TR_UTIL