#ifndef RENDERER_H

#include <Windows.h>
#include <gdiplus.h>
#include <geometry/geometry.hpp>

#include <nana/gui.hpp>
#include <nana/gui/widgets/panel.hpp>

#include "program.hpp"

using namespace Gdiplus;
using namespace nana;


template <class observer_type>
struct renderer_widget_base : public panel<true> {

	ULONG_PTR token;
	GdiplusStartupInput input;
	

	drawing drawer{*this};

	geometry::rect<double> world_rect{0, 0, 1, 1};
	Matrix* transform_matrix;

	std::vector<Pen *> pens;
	std::vector<SolidBrush *> brushes;
	SolidBrush* background_brush;

	StringFormat* format;

	observer_type *observer = nullptr;

	renderer_widget_base(window w) : panel(w) {
		GdiplusStartup(&token, &input, NULL);
		transform_matrix = new Matrix;
		accommodate(world_rect);

		pens.push_back(new Pen(Color::Red,0.0));
		pens.push_back(new Pen(Color::Green,0.0));
		pens.push_back(new Pen(Color::Blue,0.0));

		background_brush = new SolidBrush(Color::White);

		format = new StringFormat;
		format->SetAlignment(Gdiplus::StringAlignmentCenter);
		drawer.draw([&](paint::graphics g){draw_widget(g);});

		events().click(
			[&](){
				API::refresh_window(*this);
			}
		);
	}

	void accommodate(const geometry::rect<double> &world_rect) {
		int offset = 20;
		auto s = this->size();
		s.height -= offset*2;
		s.width -= offset*2;

		

		transform_matrix->Reset();
		transform_matrix->Translate(offset, this->size().height - offset);
		transform_matrix->Scale(REAL(s.width) / world_rect.width,
								REAL(s.height) / (-world_rect.height));
		// transform_matrix->Scale(REAL(world_rect.width) / this->size().width,
		// 			   REAL(-world_rect.height) /
		// 				   (this->size().height));
		transform_matrix->Translate(-world_rect.left, -world_rect.bottom);
	}
	void accommodate() { accommodate(world_rect); }

	inline virtual void back_render(Graphics* graph) {}
	inline virtual void render(Graphics* graph) {}
	inline virtual void front_render(Graphics* graph) {}

	void draw_widget(paint::graphics& g) {
		if (g.empty()) return;
		Graphics graph((HDC)g.context());
		graph.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		accommodate();
		graph.SetTransform(transform_matrix);


		back_render(&graph);
		render(&graph);
		front_render(&graph);
	}
};


struct renderer_widget : public renderer_widget_base<electrostatic_sim> {
	using renderer_widget_base<electrostatic_sim>::renderer_widget_base;
	inline void render(Gdiplus::Graphics* graph) override {
		graph->Clear(Gdiplus::Color::White);
		
		if (observer == nullptr)
			return;

		if (observer->variables_mutex.try_lock() == false) return;
		

		auto [z_min,z_max] = std::minmax_element(observer->v.node_values.begin(),observer->v.node_values.end());

		for (auto i = 0; i < observer->v.nodes.size(); i++) {
			auto &n = observer->v.nodes;
			// for (auto i : observer->v.nodes) {
			auto [x,y] = n[i].as_tuple();
			double r = 0.0025;
			RectF rect(x - r, y - r, 2 * r, 2 * r);
			graph->DrawEllipse(pens[0], rect);
		}

		for (auto& i : observer->v.triang_data)
		{
			auto& ind = i.indexes;


			PointF p[3] = {
				{(REAL)observer->v.nodes[ind[0]].x(),(REAL)observer->v.nodes[ind[0]].y()},
				{(REAL)observer->v.nodes[ind[1]].x(),(REAL)observer->v.nodes[ind[1]].y()},
				{(REAL)observer->v.nodes[ind[2]].x(),(REAL)observer->v.nodes[ind[2]].y()}};

			Pen pen(Color(31,0,0,255),0.0);

			graph->DrawPolygon(&pen,p,3);
			if ((z_min != observer->v.node_values.end()) &&
				(z_max != observer->v.node_values.end())) {
				double mean = observer->v.node_values[ind[0]] + observer->v.node_values[ind[1]] + observer->v.node_values[ind[2]];
				mean /= 3.0;
				int b = 255 * (mean - *z_min) / (*z_max - *z_min);
				SolidBrush brush(Color(127,255,255,b));
				graph->FillPolygon(&brush,p,3);
			}
		}


		Pen pen(Color(31,255,0,0),0.0);
		for (auto &i : observer->v.lower_triangles) {
			PointF triangle[3] = {PointF(i.p1.x(), i.p1.y()),
								  PointF(i.p2.x(), i.p2.y()),
								  PointF(i.p3.x(), i.p3.y())};
			graph->DrawPolygon(&pen, triangle, 3);
		}
		for (auto &i : observer->v.upper_triangles) {
			PointF triangle[3] = {PointF(i.p1.x(), i.p1.y()),
								  PointF(i.p2.x(), i.p2.y()),
								  PointF(i.p3.x(), i.p3.y())};
			graph->DrawPolygon(&pen, triangle, 3);
		}

		for (auto& i : observer->v.isolines) {
			PointF p1(i.first.x(),i.first.y());
			PointF p2(i.second.x(),i.second.y());
			graph->DrawLine(pens[1],p1,p2);
		}



		observer->variables_mutex.unlock();
	}
};

#endif //RENDERER_H
#define RENDERER_H