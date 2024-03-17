#include "nana/gui.hpp"
#include "nana/gui/drawing.hpp"
#include "nana/gui/widgets/form.hpp"
#include <Windows.h>
#include <chrono>
#include <gdiplus.h>

#include <basetsd.h>
#include <iostream>
#include <physics/physical_system/triangulation_delone.hpp>
#include <random>
#include <vector>

struct node_generator {
	std::random_device rd;

	std::vector<geometry::point2d> operator()() {
		const int width = 10;
		const int height = 10;
		std::vector<geometry::point2d> nodes(width * height);
		std::uniform_real_distribution<double> dist(-1e-6, 1e-6);
		for (int i = 0; i < width; ++i) {
			for (int j = 0; j < height; ++j) {
				nodes[width * i + j] = geometry::point2d{
					i / float(width) + dist(rd), j / float(height) + dist(rd)};

				// double r = (i+1)/20. + dist(rd);
				// double phi = 2*3.14159265358979323* j/10. + dist(rd);
				// nodes[10*i + j] =
				// geometry::point2d{0.5+r*cos(phi),0.5+r*sin(phi) };
			}
		}
		return nodes;
	}
};

int main() {
	// setlocale(LC_ALL,"ru");
	nana::form fm(nana::API::make_center(800, 800));

	Gdiplus::GdiplusStartupInput gdiplus_startup_input;
	ULONG_PTR token;
	GdiplusStartup(&token, &gdiplus_startup_input, nullptr);
	Gdiplus::Matrix transform_matrix;

	transform_matrix.Reset();
	transform_matrix.Scale(fm.size().width / 1.0, fm.size().height / -1.0);
	transform_matrix.Translate(0.05, -0.95);
	// transform_matrix.Translate(0.0, -1);

	triangulation_delone<geometry::point2d> triang;
	auto nodes = node_generator{}();

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
	nana::drawing(fm).draw([&](nana::paint::graphics g) {
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

		for (int i = 0; i < nodes.size(); i++) {
			float r = 0.005f;
			gdip.DrawEllipse(&pen, nodes[i].x() - r, nodes[i].y() - r, 2 * r,
							 2 * r);
		}

		for (auto &&i : triangles) {
			auto link = i.second;
			Gdiplus::PointF p1 = {(float)nodes[link[0]].x(),
								  (float)nodes[link[0]].y()};
			Gdiplus::PointF p2 = {(float)nodes[link[1]].x(),
								  (float)nodes[link[1]].y()};
			Gdiplus::PointF p3 = {(float)nodes[link[2]].x(),
								  (float)nodes[link[2]].y()};

			gdip.DrawLine(&pen, p1, p2);
			gdip.DrawLine(&pen, p1, p3);
			gdip.DrawLine(&pen, p2, p3);

			// auto c = i.first;
			// auto p = c.center;
			// float r = sqrtf(c.R2);
			// gdip.DrawEllipse(&pen_2,(float)p.x()- r,(float)p.y() - r,
			// 2*r,2*r);
		}
		// float r = sqrtf(circle.R2);
		// gdip.DrawEllipse(&pen,circle.center.x()-r,circle.center.y()-r,2*r,2*r);
		// r = sqrtf(inner_circle.R2);
		// gdip.DrawEllipse(&pen,inner_circle.center.x()-r,inner_circle.center.y()-r,2*r,2*r);
	});

	fm.show();
	nana::exec();
}
