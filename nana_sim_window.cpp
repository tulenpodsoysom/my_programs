#include <Windows.h>
#include <Gdiplus.h>
#include "nana/gui/basis.hpp"
#include "nana/gui/widgets/label.hpp"
#include "nana/gui/widgets/panel.hpp"
#include <nana/gui/widgets/button.hpp>

#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/group.hpp>
#include <nana/gui/widgets/widget.hpp>
#include <nana/gui.hpp>

#include <iostream>
#include <exception>
#include <string>
#include <utility>
#include <variant>
#include <vector>
// #include <nana/gui.hpp>



// holds name and default value pairs
struct system_parameters : public std::vector<std::pair<std::string, std::variant<int, double, std::string>>>
{

};

struct form_widget : public nana::panel<false>
{
	nana::place place_;

	nana::label label_;
	nana::textbox value_;

	form_widget(nana::window w, std::string l = {},std::string v = {}) : nana::panel<false>(w),place_(*this),label_(*this, l) , value_(*this,v)
	{
		place_.div("<label>|60%<value>");

		place_["label"] << label_;
		place_["value"] << value_;

		place_.collocate();
	}
};


struct system_control_window : public nana::form {

	Gdiplus::GdiplusStartupInput gdiplus_startup_input{};
  	ULONG_PTR token;
  	Gdiplus::Matrix transform_matrix;

	Gdiplus::RectF real_rect{0,0,1,1};

    nana::place placer{*this};

	nana::panel<true> render_window{*this};
	nana::drawing render_drawing;

	nana::textbox info{*this};


	std::vector<std::unique_ptr<form_widget>> parameter_inputs;
	nana::group parameters_group{*this};

	nana::button start_button{*this,"Старт"};
	nana::button set_values_button{*this,"Установить значения"};

	system_control_window()
		: nana::form(nana::API::make_center(800, 600)),
		  render_drawing(render_window) 
	{
		//Gdiplus::Status s = GdiplusStartup(&token, &gdiplus_startup_input, nullptr);

		

		placer.div("<vert<renderer>|25%<addinfo>>|30%<vert <vert parameters>|25%<vert buttons>> margin = 10");
		placer["renderer"] << render_window;
		placer["addinfo"] << info;
		placer["buttons"] << start_button << set_values_button;

        placer.collocate();

		
		transform_matrix.Reset();
		auto tr_y = -(int)render_window.size().height;

		auto sc_x = (float)render_window.size().width / real_rect.Width;
		auto sc_y = (float)render_window.size().height / -real_rect.Height;

		transform_matrix.Translate(0.00, tr_y);
		transform_matrix.Scale(sc_x, sc_y);

		start_button.events().click([&](nana::arg_click arg){
			static bool started = false;
			started = !started;
			if (started) start_button.caption("Стоп");
			else start_button.caption("Старт");
		});

		render_window.bgcolor(nana::colors::white);
		render_drawing.draw([&](nana::paint::graphics g) {
			Gdiplus::Graphics gdip((HDC)g.context());
			Gdiplus::Pen pen(Gdiplus::Color(255, 0, 0), 0);
			Gdiplus::Pen pen_2(Gdiplus::Color(31, 0, 0, 255), 0);
			Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0));
			Gdiplus::FontFamily font_family(L"Arial");
			Gdiplus::Font font(&font_family, 11);
			Gdiplus::StringFormat format;
			format.SetAlignment(Gdiplus::StringAlignmentNear);
			gdip.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

			gdip.SetTransform(&transform_matrix);
			//gdip.DrawEllipse(&pen,real_rect);
			gdip.DrawEllipse(&pen,Gdiplus::RectF{0,0,10,10});
		});

		events().resized([&](const nana::arg_resized &arg) {
			transform_matrix.Reset();
			transform_matrix.Translate(0.00, -render_window.size().height);
			transform_matrix.Scale((float)render_window.size().width / real_rect.Width,
								   (float)render_window.size().height /
									   -real_rect.Height);

			Gdiplus::REAL m[12];
			transform_matrix.GetElements(m);
			std::string s;
			for (int i = 0; i < 12; i++)
				s += std::to_string(m[i]) + ' ';
			info.reset(s, false);

			nana::API::refresh_window(render_window);
		});
	}

	~system_control_window()
	{
		Gdiplus::GdiplusShutdown(token);
		//nana::form::~form();
	}

	void parse_system_parameters(system_parameters& sp)
	{
		for (auto& i : sp) {
			std::string a;

			switch (i.second.index()) {
			case 0:
				a = std::to_string(std::get<0>(i.second));
				break;
			case 1:
				a = std::to_string(std::get<1>(i.second));
				break;
			case 2:
				a = std::get<2>(i.second);
				break;
			}
			parameter_inputs.emplace_back(new form_widget((*this),i.first,a)); 	
			placer["parameters"] << *parameter_inputs.back();
		}
		
		
		placer.collocate();
	}
};

int main() {
	
	Gdiplus::GdiplusStartupInput gdiplus_startup_input{};
  	ULONG_PTR token;
	Gdiplus::Status s = GdiplusStartup(&token, &gdiplus_startup_input, nullptr);




	try{
	system_control_window w;

	system_parameters sp({{"Alpha", 1}, {"Beta", -1.0}, {"Gamma", "test 3.0"}});

	w.parse_system_parameters(sp);
	w.show();
	nana::exec();
	}
	catch (std::exception a)
	{
		std::cout << a.what() << std::endl;
	}
}