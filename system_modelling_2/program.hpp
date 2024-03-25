#ifndef PROGRAM_H
#include <physics/physical_system/triangulation_delone.hpp>
#include <physics/physical_system/isolines.hpp>
#include <random>
#include <time_flow_program.hpp>

#include <ranges>

struct noise_applier
{
	std::random_device &rd;
	std::uniform_real_distribution<> dist{-1e-10, 1e-10};

	void operator() (geometry::point2d& p) {p.x() += dist(rd); p.y() += dist(rd);}
	void operator() (double& n) {n += dist(rd);}
};

struct node_generator2d {
	std::random_device &rd;
	geometry::rect<double> r;
	unsigned n_x{}, n_y{};
	noise_applier noise{rd};

	std::function<geometry::point2d(int)> equdistant_grid_create = [&](int i) {
		if (i > n_x * n_y)
			return geometry::point2d();
		int j = i % n_x;
		i = i / n_x;

		auto p = r.from_rect_coord(double(j) / n_x, double(i) / n_y);
		noise(p);
		return p;
	};

	std::function<bool(geometry::point2d &)> inside_system =
		[](geometry::point2d &p) { return true; };

	std::vector<geometry::point2d> operator()() {
		std::vector<geometry::point2d> nodes;
		nodes.reserve(n_x * n_y);
		for (int i = 0; i < n_x * n_y; i++) {
			auto p = equdistant_grid_create(i);
			if (inside_system(p)) {
				nodes.push_back(p);
			}
		}
		return nodes;
	}

	int _2d_to_1d(int i, int j) { return j + i * n_x; }

	std::pair<int, int> _1d_to_2d(int i) { return {i / n_x, i % n_x}; }
};

struct parameters {
	geometry::rect<double> real_rect{0.0, 0.0, 3.0, 1.0};

	size_t nodes_size_width{60};
	size_t nodes_size_height{20};

	double temparature = 273;

	const double elemental_charge = 1.602176634e-19;
	const double electron_mass = 9.1093837015e-31;

	const double boltzmann_constant = 1.380649e-23;
	const double epsilon_0 = 8.85418781762039e-12;

	const double PI = 3.14159265358979323;
};

struct variables {
	struct particle
	{
		double x{},y{}; double vx{},vy{};
	};


	std::vector<particle> particles;
	std::vector<geometry::point2d> nodes;
	std::vector<double> node_values;

	triangulation_delone delone;
	std::vector<triangulation_delone::triangulation_data> triang_data;
	std::vector<geometry::polygon2d> dirichlet_cells;
};

struct electrolamp_sim : public time_flow_program<parameters, variables> {
	std::random_device rd;
	noise_applier noise{rd};

	void prepare() override;
	void cycle_function() override {};
	void evaluate();


	// calculates square of debye length
	// inline double evaluate_debye_length2() {}


	// the inverse of plasma frequency approximation
	inline double evaluate_time_period (double n,double epsilon = 1.0) {
		return sqrt((epsilon*p.epsilon_0*p.electron_mass)/(p.elemental_charge*p.elemental_charge*n));
	}

	inline double maxwell_projection_distribution()
	{
		double SKO = sqrt(p.electron_mass/(2*p.boltzmann_constant*p.temparature));
		std::normal_distribution<double>dist (0,SKO);
		return dist(rd);
	}


};

#endif // PROGRAM_H
#define PROGRAM_H
