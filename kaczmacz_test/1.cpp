#include "../kazf.h"
#include <iostream>
#include <vector>
#include <array>


// A*x = B

// m*n . n = m

template <typename T>
void print(T begin, T end)
{
    for (auto i = begin; i != end; i++) {
        std::cout << *i << std::endl;
    }   
}

int main()
{
    const int m{3},n{3};

    std::array<float,m*n> A;
    A = 
    {
        1,2,-1,
        2,-3,2,
        3,1,1
    };
    std::array<float,m> B;
    B = 
    {
        2,2,8
    };

    std::array<float,n> X;


    kazf(A.data(), B.data(), X.data(), n, m);

	//print(A.begin(), A.end());
    print(X.begin(), X.end());
    //print(B.begin(), B.end());
}