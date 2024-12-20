// combinations.hpp 
// from https://stackoverflow.com/a/25497877
// With later  modifications to adapt for std::reference_wrapper
#include <vector>
#include <algorithm>

template<typename T> class Combinations {
// Combinations(std::vector<T> s, int m) iterate all Combinations without repetition
// from set s of size m s = {0,1,2,3,4,5} all permuations are: {0, 1, 2}, {0, 1,3}, 
// {0, 1, 4}, {0, 1, 5}, {0, 2, 3}, {0, 2, 4}, {0, 2, 5}, {0, 3, 4}, {0, 3, 5},
// {0, 4, 5}, {1, 2, 3}, {1, 2, 4}, {1, 2, 5}, {1, 3, 4}, {1, 3, 5}, {1, 4, 5}, 
// {2, 3, 4}, {2, 3, 5}, {2, 4, 5}, {3, 4, 5}

public:
    Combinations(std::vector<T> s, int m) : M(m), set(s)
    {
        N = s.size(); // unsigned long can't be casted to int in initialization
        
        partial.resize(M); //Preallocate space

        generate(0, N-1, M-1);
    };
    virtual ~Combinations() {}

    typedef typename std::vector<std::vector<T>>::const_iterator const_iterator;
    typedef typename std::vector<std::vector<T>>::iterator iterator;
    iterator begin() { return out.begin(); }
    iterator end() { return out.end(); }    
    std::vector<std::vector<T>> get() { return out; }

private:
    void generate(int i, int j, int m);
    unsigned long long comb(unsigned long long n, unsigned long long k); // C(n, k) = n! / (n-k)!

    int N;
    int M;
    const std::vector<T> set;
    // Use std::optional, so we can resize vector and the beginning and use object than don't have a
    // default value like std::reference_wrapper
    std::vector<std::optional<T>> partial;
    std::vector<std::vector<T>> out;
};

template<typename T> 
void Combinations<T>::generate(int i, int j, int m) {  
    // combination of size m (number of slots) out of set[i..j]
    if (m > 0) { 
        for (int z=i; z<j-m+1; z++) { 
            partial[M-m-1]=set[z]; // add element to permutation
            generate(z+1, j, m-1);
        }
    } else {
        // last position
        for (int z=i; z<j-m+1; z++) { 
            partial[M-m-1] = set[z];
            // copy to output vector after dropping the std::optional
            std::vector<T> fullPartial;
            std::for_each(partial.begin(), partial.end(), [&fullPartial](std::optional<T>& t) {
              fullPartial.push_back(t.value());
            });
            out.push_back(fullPartial);
        }
    }
}
