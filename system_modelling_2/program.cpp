#include "program.hpp"
#include <armadillo>
#include "../kazf.h"

void electrolamp_sim::prepare() {
	variables_mutex.lock();
	node_generator2d n_gen{rd, p.real_rect, (unsigned)p.nodes_size_width,
						   (unsigned)p.nodes_size_height};

	logger_stream << "Starting generating nodes" << std::endl;
	v.nodes = n_gen();
	logger_stream << "Generated " << v.nodes.size() << " nodes " <<  std::endl; 


	v.delone.create_square_superstructure(
		p.real_rect.left-1, p.real_rect.right()+1, p.real_rect.top()+1,
		p.real_rect.bottom-1,rd);

	v.triang_data = v.delone.recursive_enumerate(
		v.nodes.begin(), v.nodes.end(), v.delone.superstructure_nodes.begin(),
		v.delone.superstructure_nodes.end());

	v.dirichlet_cells = v.delone.get_cells(v.nodes.begin(),v.nodes.end(),v.triang_data);
	v.node_values.resize(v.nodes.size());
	variables_mutex.unlock();
}
void electrolamp_sim::evaluate() {

	// Sum (c_i * P_ik) = F_k

	arma::mat P(v.nodes.size(),v.nodes.size());
	// std::vector<double> P(v.nodes.size()*v.nodes.size());

	for (auto i = 0; i < v.nodes.size(); i++) {
		for (auto k = 0; k < v.nodes.size(); k++) {
			for (auto &d : v.triang_data) {
				auto& ind = d.indexes;

				int i_point = (ind[0] == i)? 0 : (ind[1] == i)? 1 : (ind[2] == i)? 2 : -1;
				if (i_point == -1) continue;
				int k_point = (ind[0] == k)? 0 : (ind[1] == k)? 1 : (ind[2] == k)? 2 : -1;
				if (k_point == -1) continue;

				auto &p1 = v.nodes[ind[0]];
				auto &p2 = v.nodes[ind[1]];
				auto &p3 = v.nodes[ind[2]];

				auto area = geometry::triangle2<double>{p1, p2, p3}.area();
				geometry::plane3d plane_i(
					{p1.x(), p1.y(), (i_point == 0) ? 1.0 : 0.0},
					{p2.x(), p2.y(), (i_point == 1) ? 1.0 : 0.0},
					{p3.x(), p3.y(), (i_point == 2) ? 1.0 : 0.0});

				geometry::plane3d plane_k(
					{p1.x(), p1.y(), (k_point == 0) ? 1.0 : 0.0},
					{p2.x(), p2.y(), (k_point == 1) ? 1.0 : 0.0},
					{p3.x(), p3.y(), (k_point == 2) ? 1.0 : 0.0});

				auto d_dx_i = plane_i.A;
				auto d_dx_k = plane_k.A;

				auto d_dy_i = plane_i.B;
				auto d_dy_k = plane_k.B;


				P.at(i,k) += - (d_dx_i * d_dx_k + d_dy_i * d_dy_k) * area;
				// P.at(i * v.nodes.size() + k) += - (d_dx_i * d_dx_k + d_dy_i * d_dy_k) * area;
			}
		}
	}

	arma::vec F(v.nodes.size());
	// std::vector<double> F(v.nodes.size());

	for (int k = 0; k < v.nodes.size(); k++) {

		// going through nodes with positive capacitor charge
		for (int i = v.nodes.size() - v.triangle_edge_nodes.size(); i < v.nodes.size() - v.triangle_edge_nodes.size() / 2; i++) {
			for (auto &tr : v.triang_data) {
				auto &ind = tr.indexes;
				int i_point = (ind[0] == i)? 0 : (ind[1] == i) ? 1: (ind[2] == i) ? 2: -1;
				if (i_point == -1) continue;
				int k_point = (ind[0] == k)? 0 : (ind[1] == k)? 1 : (ind[2] == k)? 2 : -1;
				if (k_point == -1) continue;

				if (i_point == k_point) continue;

				auto &p1 = v.nodes[ind[0]];
				auto &p2 = v.nodes[ind[1]];
				auto &p3 = v.nodes[ind[2]];

				auto area = geometry::triangle2<double>{p1, p2, p3}.area();
				geometry::plane3d plane_i(
					{p1.x(), p1.y(), (i_point == 0) ? 1.0 : 0.0},
					{p2.x(), p2.y(), (i_point == 1) ? 1.0 : 0.0},
					{p3.x(), p3.y(), (i_point == 2) ? 1.0 : 0.0});

				geometry::plane3d plane_k(
					{p1.x(), p1.y(), (k_point == 0) ? 1.0 : 0.0},
					{p2.x(), p2.y(), (k_point == 1) ? 1.0 : 0.0},
					{p3.x(), p3.y(), (k_point == 2) ? 1.0 : 0.0});

				auto d_dx_i = plane_i.A;
				auto d_dx_k = plane_k.A;

				auto d_dy_i = plane_i.B;
				auto d_dy_k = plane_k.B;

				F.at(k) += p.capacitor_charge * (d_dx_i * d_dx_k + d_dy_i * d_dy_k) * area;
			}
		}

		// going through nodes with negative capacitor charge
		for (int i = v.nodes.size() - v.triangle_edge_nodes.size()/2; i < v.nodes.size(); i++) {
			for (auto &tr : v.triang_data) {
				auto &ind = tr.indexes;
				int i_point = (ind[0] == i)? 0 : (ind[1] == i) ? 1: (ind[2] == i) ? 2: -1;
				if (i_point == -1) continue;
				int k_point = (ind[0] == k)? 0 : (ind[1] == k)? 1 : (ind[2] == k)? 2 : -1;
				if (k_point == -1) continue;

				if (i_point == k_point) continue;

				auto &p1 = v.nodes[ind[0]];
				auto &p2 = v.nodes[ind[1]];
				auto &p3 = v.nodes[ind[2]];

				auto area = geometry::triangle2<double>{p1, p2, p3}.area();
				geometry::plane3d plane_i(
					{p1.x(), p1.y(), (i_point == 0) ? 1.0 : 0.0},
					{p2.x(), p2.y(), (i_point == 1) ? 1.0 : 0.0},
					{p3.x(), p3.y(), (i_point == 2) ? 1.0 : 0.0});

				geometry::plane3d plane_k(
					{p1.x(), p1.y(), (k_point == 0) ? 1.0 : 0.0},
					{p2.x(), p2.y(), (k_point == 1) ? 1.0 : 0.0},
					{p3.x(), p3.y(), (k_point == 2) ? 1.0 : 0.0});

				auto d_dx_i = plane_i.A;
				auto d_dx_k = plane_k.A;

				auto d_dy_i = plane_i.B;
				auto d_dy_k = plane_k.B;


				
				// auto& ip   = (i_point == 0)? p1 : (i_point == 1)? p2 : p3;
				// auto& ip_  = (i_point == 0)? p2 : (i_point == 1)? p3 : p1;
				// auto& ip__ = (i_point == 0)? p3 : (i_point == 1)? p1 : p2;

				// auto& kp   = (k_point == 0)? p1 : (k_point == 1)? p2 : p3;
				// auto& kp_  = (k_point == 0)? p2 : (k_point == 1)? p3 : p1;
				// auto& kp__ = (k_point == 0)? p3 : (k_point == 1)? p1 : p2;

				// auto [A1, B1] = A_B(ip, ip_, ip__);
				// auto [A2, B2] = A_B(kp, kp_, kp__);
				// auto d_dx_i = A1;
				// auto d_dx_k = A2;
				// auto d_dy_i = B1;
				// auto d_dy_k = B2;

				// the only difference from above one is a sign
				F.at(k) += -p.capacitor_charge * (d_dx_i * d_dx_k + d_dy_i * d_dy_k) * area;
			}
		}
	}

	logger_stream << "evaluated P_ik and F_k" << std::endl;
	
	// std::vector<double> C(v.nodes.size());
	// kazf(P.data(), F.data(), C.data(), v.nodes.size(), v.nodes.size());

	arma::vec C = arma::solve(P, F, arma::solve_opts::force_approx);

	logger_stream << "passed solving system of equations" << std::endl;
	v.node_values = arma::conv_to<std::vector<double>>::from(std::move(C));
	// v.node_values = std::move(C);

	isoline_generator gen{20,v.node_values,v.nodes,v.triang_data};

	v.isolines = gen.get_isolines();
}
