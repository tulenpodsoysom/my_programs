#include <armadillo>

using namespace arma;
int main()
{
    const int n{3},m{3};
    mat A(m,n);
    A = 
    {
        { 1,2,-1},
        {2,-3,2},
        {3,1,1}
    };
    //vec X(n);

    vec B(n);
    B = 
    {
        2,2,8
    };

    auto X = solve(A,B);

    X.print();

}