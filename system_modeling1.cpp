

#include "nana/gui.hpp"
#include "nana/gui/basis.hpp"
#include "nana/gui/widgets/form.hpp"
#include "nana/paint/graphics.hpp"
#include <Windows.h>
#include <chrono>
#include <cstddef>
#include <gdiplus.h>

#include <basetsd.h>
#include <iostream>
#include <random>
#include <vector>

#include <geometry/rect.hpp>
#include <geometry/triangle.hpp>
#include <physics/physical_system/node_generator.hpp>
#include <physics/physical_system/triangulation_delone.hpp>
#include <time_flow_program.hpp>

struct parameters {
	geometry::rect<double> system_window{0.0, 0.0, 1.0, 1.0};

	double triangle_height_1{0.1};
	double triangle_width_1{0.05};
	double triangle_height_2{0.1};
	double triangle_width_2{0.05};
	geometry::point2d triangle_offset_1{0.1, 0.1};
	geometry::point2d triangle_offset_2{0.1, -0.1};
	size_t triangles_amount_1{10};
	size_t triangles_amount_2{10};

	size_t nodes_size_width{20};
	size_t nodes_size_height{20};

	double capacitor_charge = 1;
};

struct variables {
	std::vector<geometry::point2d> nodes;
	std::vector<double> node_values;
	std::vector<triangulation_delone<geometry::point2d>::triangle_link>
};

struct electrostatic_sim : public time_flow_program<parameters, variables> {
	void cycle_function() override {}
};

struct main_window : public nana::form {
	main_window() : nana::form(nana::API::make_center(800, 800)) {
		GdiplusStartup(&token, &gdiplus_startup_input, nullptr);
		transform_matrix.Reset();
		transform_matrix.Scale(this->size().width / 1.0,
							   this->size().height / -1.0);
		transform_matrix.Translate(0.05, -0.95);
		this->div("<vert<renderer>|20<info>>|30<buttons>");

		(*this)["renderer"] << render_window;
	};

	Gdiplus::GdiplusStartupInput gdiplus_startup_input;
	ULONG_PTR token;
	Gdiplus::Matrix transform_matrix;

	nana::window render_window;

	electrostatic_sim *p_sim;

	void render_function(nana::paint::graphics g) {
		Gdiplus::Graphics gdip((HDC)g.context());
		Gdiplus::Pen pen(Gdiplus::Color(255, 0, 0), 0);
		Gdiplus::Pen pen_2(Gdiplus::Color(31, 0, 0, 255), 0);
		Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0));
		Gdiplus::FontFamily font_family(L"Arial");
		Gdiplus::Font font(&font_family, 11);
		Gdiplus::StringFormat format;
		format.SetAlignment(Gdiplus::StringAlignmentNear);
		gdip.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

		Gdiplus::Status s = gdip.SetTransform(&transform_matrix);

		if (p_sim == nullptr)
			return;
		auto &sim = *p_sim;
		auto &v = sim.v;

		for (int i = 0; i < v.nodes.size(); i++) {
			float r = 0.005f;
			gdip.DrawEllipse(&pen, v.nodes[i].x - r, v.nodes[i].y - r, 2 * r,
							 2 * r);
		}

		for (auto &&i : triangles) {
			auto link = i.second;
			Gdiplus::PointF p1 = {(float)nodes[link[0]].x,
								  (float)nodes[link[0]].y};
			Gdiplus::PointF p2 = {(float)nodes[link[1]].x,
								  (float)nodes[link[1]].y};
			Gdiplus::PointF p3 = {(float)nodes[link[2]].x,
								  (float)nodes[link[2]].y};

			gdip.DrawLine(&pen, p1, p2);
			gdip.DrawLine(&pen, p1, p3);
			gdip.DrawLine(&pen, p2, p3);
		}
	}
};

int main() {
	using namespace std::chrono;

	std::cout << "Generating grid: ";

	system_clock clk;
	auto tp1 = clk.now();
	auto triangles = triang.simple_enumerate(nodes.begin(), nodes.end());
	auto tp2 = clk.now();

	std::cout << duration_cast<milliseconds>(tp2 - tp1).count() / 1000.0
			  << " ms" << std::endl;
	std::cout << "Generated nodes: " << nodes.size() << std::endl;
	std::cout << "Generated triangles: " << triangles.size() << std::endl;

	fm.show();
	nana::exec();
}