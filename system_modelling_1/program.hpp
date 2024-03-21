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
};

struct parameters {
	geometry::rect<double> real_rect{0.0, 0.0, 1.0, 1.0};

	double triangle_height_1{0.1};
	double triangle_width_1{0.05};
	double triangle_height_2{0.1};
	double triangle_width_2{0.05};
	geometry::point2d triangle_offset_1{0.1, 0.1};
	geometry::point2d triangle_offset_2{0.1, -0.1};
	size_t triangles_amount_1{8};
	size_t triangles_amount_2{8};

	size_t nodes_size_width{20};
	size_t nodes_size_height{20};

	double capacitor_charge = 0.1;
};

struct variables {
	std::vector<geometry::triangle2<double>> lower_triangles;
	std::vector<geometry::triangle2<double>> upper_triangles;
	std::vector<geometry::point2d> nodes;
	std::vector<geometry::point2d> triangle_edge_nodes;
	std::vector<double> node_values;

	triangulation_delone delone;

	std::vector<triangulation_delone::triangulation_data> triang_data;

	std::vector<std::pair<geometry::point2d,geometry::point2d>> isolines;

	//auto all_nodes_view() {return std::ranges::join_view({nodes,triangle_edge_nodes});}
};

struct electrostatic_sim : public time_flow_program<parameters, variables> {
	std::random_device rd;
	//std::mt19937 mt{rd()};

	noise_applier noise{rd};

	void prepare() override;
	void cycle_function() override {};
	void evaluate();
};

#endif // PROGRAM_H
#define PROGRAM_H
