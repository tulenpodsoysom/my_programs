#include "program.hpp"

void electrostatic_sim::prepare() {
	variables_mutex.lock();

	v.lower_triangles.resize(p.triangles_amount_1);
	v.upper_triangles.resize(p.triangles_amount_2);

	for (auto i = 0; i < v.lower_triangles.size(); i++) {
		auto p1 = p.real_rect.left_bottom() + p.triangle_offset_1;
		auto p2 = p.real_rect.left_bottom() + p.triangle_offset_1;
		auto p3 = p.real_rect.left_bottom() + p.triangle_offset_1;

		p1[0] += i * p.triangle_width_2;
		p2[0] += (i + 1) * p.triangle_width_2;
		p3[0] += (i + 0.5) * p.triangle_width_2;

		p3[1] += p.triangle_height_2;

		v.lower_triangles[i] = geometry::triangle2<double>{p1, p2, p3};
	}
	for (auto i = 0; i < v.upper_triangles.size(); i++) {
		auto p1 = p.real_rect.left_top() + p.triangle_offset_2;
		auto p2 = p.real_rect.left_top() + p.triangle_offset_2;
		auto p3 = p.real_rect.left_top() + p.triangle_offset_2;

		p1[0] += i * p.triangle_width_1;
		p2[0] += (i + 1) * p.triangle_width_1;
		p3[0] += (i + 0.5) * p.triangle_width_1;

		p3[1] -= p.triangle_height_1;
		
		v.upper_triangles[i] = geometry::triangle2<double>{p1, p2, p3};
	}
	node_generator2d n_gen{rd, p.real_rect, (unsigned)p.nodes_size_width,
						   (unsigned)p.nodes_size_height};

	n_gen.inside_system = [&](geometry::point2d &p) {
		for (auto &i : v.lower_triangles) {
			if (i.contains(p.x(), p.y()))
				return false;
		}
		for (auto &i : v.upper_triangles) {
			if (i.contains(p.x(), p.y()))
				return false;
		}
		return true;
	};

	auto sub_points = [](geometry::triangle2<double> t,int n = 10) {
		std::vector<geometry::point2d> v;
		for (auto j = 0; j < n; j++) {
			v.push_back(geometry::lerp(t.p1, t.p2, j/double(n)));
		}
		for (auto j = 0; j < n; j++) {
			v.push_back(geometry::lerp(t.p1, t.p3, j/double(n)));
		}
		for (auto j = 0; j < n; j++) {
			v.push_back(geometry::lerp(t.p2, t.p3, j/double(n)));
		}
		return v;
	} ;

	// int n = 10;
	v.triangle_edge_nodes = {};
	v.triangle_edge_nodes.reserve(10 * 6);
	for (auto &i : v.upper_triangles) {
		auto a = sub_points(i);
		v.triangle_edge_nodes.insert(v.triangle_edge_nodes.end(),a.begin(),a.end());

		// for (auto j = 0; j < n; j++) {
		// 	v.triangle_edge_nodes.push_back(geometry::lerp(i.p1, i.p2, j/double(n)));
		// }
		// for (auto j = 0; j < n; j++) {
		// 	v.triangle_edge_nodes.push_back(geometry::lerp(i.p1, i.p3, j/double(n)));
		// }
		// for (auto j = 0; j < n; j++) {
		// 	v.triangle_edge_nodes.push_back(geometry::lerp(i.p2, i.p3, j/double(n)));
		// }
	}
	for (auto &i : v.lower_triangles) {
		auto a = sub_points(i);
		v.triangle_edge_nodes.insert(v.triangle_edge_nodes.end(),a.begin(),a.end());
		
		// for (auto j = 0; j < n; j++) {
		// 	v.triangle_edge_nodes.push_back(geometry::lerp(i.p1, i.p2, j/double(n)));
		// }
		// for (auto j = 0; j < n; j++) {
		// 	v.triangle_edge_nodes.push_back(geometry::lerp(i.p1, i.p3, j/double(n)));
		// }
		// for (auto j = 0; j < n; j++) {
		// 	v.triangle_edge_nodes.push_back(geometry::lerp(i.p2, i.p3, j/double(n)));
		// }
	}
	

	logger_stream << "Starting generating nodes" << std::endl;
	v.nodes = n_gen();
	logger_stream << "Generated " << v.nodes.size() << " nodes out of " << n_gen.n_x*n_gen.n_y << std::endl; 


	v.delone.create_square_superstructure(
		p.real_rect.left-1, p.real_rect.right()+1, p.real_rect.top()+1,
		p.real_rect.bottom-1,rd);

	


	for (auto &i : v.upper_triangles) {
		// auto t = geometry::expand_triangle(i, 0.85);
		// auto a = sub_points(t,5);
		

		// noise(t.p1);
		// noise(t.p2);
		// noise(t.p3);

		auto p = geometry::middle_point({i.p1, i.p2, i.p3});
		//noise(p);
		// v.delone.superstructure_nodes.insert(v.delone.superstructure_nodes.end(),
			// a.begin(),a.end());
		v.delone.superstructure_nodes.push_back(p);
		// v.delone.superstructure_nodes.insert(
			// v.delone.superstructure_nodes.end(), {t.p1, t.p2, t.p3});
	}
	for (auto &i : v.lower_triangles) {
		// auto t = geometry::expand_triangle(i, 0.85);
		// auto a = sub_points(t,5);
		// noise(t.p1);
		// noise(t.p2);
		// noise(t.p3);

		auto p = geometry::middle_point({i.p1, i.p2, i.p3});
		//noise(p);
				// v.delone.superstructure_nodes.insert(v.delone.superstructure_nodes.end(),
			// a.begin(),a.end());
		v.delone.superstructure_nodes.push_back(p);
		// v.delone.superstructure_nodes.insert(
			// v.delone.superstructure_nodes.end(), {t.p1, t.p2, t.p3});
	}

	//auto all_nodes = std::ranges::join_view(std::vector{v.nodes,v.triangle_edge_nodes});

	size_t before_insert = v.nodes.size();
	v.nodes.insert(v.nodes.end(),v.triangle_edge_nodes.begin(),v.triangle_edge_nodes.end());

	v.triang_data = v.delone.recursive_enumerate(
		v.nodes.begin(), v.nodes.end(), v.delone.superstructure_nodes.begin(),
		v.delone.superstructure_nodes.end());

	{
		std::vector<triangulation_delone::triangulation_data> buffer;
		buffer.reserve(v.triang_data.size());
		for (auto& i : v.triang_data) {
			auto& ind = i.indexes;
			if ((ind[0] > before_insert) && (ind[1] > before_insert) && (ind[2] > before_insert))
			{}
			else
				buffer.push_back(i);
		}	
		v.triang_data = std::move(buffer);
	}
	

	

	variables_mutex.unlock();
}
void electrostatic_sim::cycle_function() {}
