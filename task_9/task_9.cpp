#include <vector>
#include <random>
#include <iostream>

using ull = unsigned long long;

struct operation_simulator
{
    std::random_device& rd;
    ull N;
    ull counter{};
    ull operator() ()
    {
        ++counter;
        std::uniform_int_distribution<ull> dist(0,N-1);
        auto a = dist(rd);
        return a;
    }
};

bool all_true(std::vector<bool>& a)
{
    return std::all_of(a.begin(),a.end(),[](bool a) {return a == true;});
}

std::random_device rd;

ull experiment(ull N)
{
    // compresses 8 different colors in one byte
    std::vector<bool> found_colors(N);

    operation_simulator sim{rd,N};

    do {
        found_colors[sim()] = true;
    } while (!all_true(found_colors));
    return sim.counter;
}


// this function probably can be simplified with a mathematical expression 
// Sum(N/(N-i), {i=0,N-1})
// see F_math(N)
auto F(ull N , double iterations = 1000)
{
    double sum = {};
    double a = {} , b = {};

    // Выборочное среднее  
    for (auto i = 0; i < iterations; i++) {
        sum += experiment(N);
    }   sum /= double(iterations);

    //auto l = -1.96/sqrt(iterations*N) ,r = 1.96/sqrt(iterations*N);

    return sum;
}


// precision mostly defined by DBL_EPSILON
auto F_math(ull N)
{
    double sum{};
    for (ull i = 1; i <= N; i++) {
        sum += 1.0/double(i);
    } 
    return N*sum;
}

// vector must be sorted
std::pair<ull,ull> find_interval(std::vector<ull>& v, double percent = 0.99)
{
    auto n = v.size();
    size_t points_in = ceil(n*percent);

    std::vector<std::pair<ull,ull>> intervals(n - points_in);

    for (size_t i = 0; i < n - points_in; i++) {
		intervals[i] = {v[i],v[i+points_in]};
	}
	return *std::min_element(
		intervals.begin(), intervals.end(), [](auto &a, auto &b) {
			return (a.second - a.first) < (b.second - b.first);
		});
}

int main()
{
    ull N;
    size_t selection_size;
    double percent = 0.99;
    std::cout << "Enter N, selection_size, percent (as a fraction: 0.99)" << std::endl;
    std::cin >> N >> selection_size >> percent;

    std::cout <<"F_math(N): " <<  F_math(N) << std::endl;
    std::cout <<"F(N): " <<  F(N,selection_size) << std::endl;

    std::vector<ull> fs(selection_size);
    std::generate(fs.begin(),fs.end(),[&]{return experiment(N);});

    std::sort(fs.begin(),fs.end());
    auto [a,b] = find_interval(fs,percent);

    std::cout << "interval with " << percent*100 <<  "%: " << a << ' ' << b << std::endl;
    std::cout << "interval length: " << b - a << std::endl;
    std::cout << std::endl;
}