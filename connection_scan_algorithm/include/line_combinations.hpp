#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <vector>

#include "combinations.hpp"
#include "line.hpp"

namespace TrRouting
{
  using LineVector = std::vector<std::reference_wrapper<const Line>>;
  using CombinationMap = std::map<LineVector, bool>;

  /**
     True if every line of `subset` is present in `lines`. An empty `subset` is
     always contained.
   */
  inline bool containsAllLines(const LineVector & lines, const LineVector & subset)
  {
    for (const auto & line : subset)
    {
      if (std::find(lines.begin(), lines.end(), line) == lines.end())
      {
        return false;
      }
    }
    return true;
  }

  /**
     True if the lines of `candidate` represent a subset of a line combination
     presention in the list of combinations

     For example: if combinations has lines [1,2] and [2,3], the function will
     return true for a candidate of [2,3,4]. It will return false for a candidate
     of [1,3] as in both cases there's a new line in the candidate.
   */
  inline bool isSupersetOfAnyCombinations(const LineVector & candidate,
                                          const std::vector<LineVector> & combinations)
  {
    for (const auto & combination : combinations)
    {
      if (containsAllLines(candidate, combination))
      {
        return true;
      }
    }
    return false;
  }

  /**
     Generate combinations of 1 line, then 2 lines, up to the total amount of
     lines in `foundLines`. Each generated combination is extended with `prefix`
     (the lines already excluded by the alternative that produced `foundLines`,
     empty for the first generation) and sorted.

     A combination is appended to `allCombinations` only if it has not been seen
     before and does not match a failed combination. Combinations rejected by the
     failed check are still recorded in `alreadyCalculatedCombinations`, so a
     later generation does not evaluate them again.

     @param foundLines Lines used by the alternative we are branching from
     @param prefix Lines already excluded, added to every generated combination
     @param failedCombinations Combinations known to yield no route
     @param allCombinations Container the accepted combinations are appended to
     @param alreadyCalculatedCombinations Bookkeeping of every combination seen
   */
  inline void generateCombinations(const LineVector & foundLines,
                                   const LineVector & prefix,
                                   const std::vector<LineVector> & failedCombinations,
                                   std::vector<LineVector> & allCombinations,
                                   CombinationMap & alreadyCalculatedCombinations)
  {
    for (size_t k = 1; k <= foundLines.size(); k++)
    {
      Combinations<std::reference_wrapper<const Line>> combinations(foundLines, k);
      for (auto newCombination : combinations)
      {
        // Copy lines from previous combinations
        std::copy(prefix.begin(), prefix.end(), std::back_inserter(newCombination));
        std::stable_sort(newCombination.begin(), newCombination.end());
        if (!alreadyCalculatedCombinations.emplace(newCombination, true).second)
        {
          continue;
        }
        // Do not add the new line combination if there's a subset of it in a previsouly
        // failed calculation of a combination.
        if (!isSupersetOfAnyCombinations(newCombination, failedCombinations))
        {
          allCombinations.push_back(newCombination);
        }
      }
    }
  }
}
