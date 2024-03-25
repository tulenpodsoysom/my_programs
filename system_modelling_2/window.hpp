#include <Windows.h>
#include <gdiplus.h>

#include "renderer.hpp"
#include "program.hpp"

#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/timer.hpp>

using namespace std;
using namespace nana;

struct form_widget : public panel<false> {
	place place_;

	label label_;
	textbox value_;

	form_widget(window w, string l = {}, string v = {})
		: panel<false>(w), place_(*this), label_(*this, l), value_(*this, v) {
        label_.text_align(align::right);
		place_.div("<label>|60%<value>");

		place_["label"] << label_;
		place_["value"] << value_;

		place_.collocate();
	}
};

struct main_window : public form {
    #define create_form(NAME,INIT_VALUE) form_widget NAME{*this,#NAME + std::string(":"),#INIT_VALUE}
	drawing drawer_{*this};
	place place_{*this};

    electrolamp_sim program;
	renderer_widget renderer_{*this};


    button update_button{*this,"Обновить"};
    button startstop_button{*this,"Старт"};

    timer render_timer;
	textbox info_textbox{*this};


    create_form(offset_1_x, 0.1);
    create_form(offset_1_y, 0.1);
    create_form(offset_2_x, 0.1);
    create_form(offset_2_y, -0.1);

    create_form(triangle_width_1, 0.1);
    create_form(triangle_height_1, 0.3);
	create_form(triangle_width_2, 0.1);
	create_form(triangle_height_2, 0.3);
    create_form(triangle_amount, 8);

	create_form(nodes_amount_x, 20);
	create_form(nodes_amount_y, 20);

	void update_program()
    {
		program.clear();
        auto& p = program.p;
        p.triangle_offset_1 = {offset_1_x.value_.to_double(),offset_1_y.value_.to_double()};
        p.triangle_offset_2 = {offset_2_x.value_.to_double(),offset_2_y.value_.to_double()};
		p.triangle_width_1  = triangle_width_1.value_.to_double();
		p.triangle_height_1 = triangle_height_1.value_.to_double();
		p.triangle_width_2  = triangle_width_2.value_.to_double();
        p.triangle_height_2 = triangle_height_2.value_.to_double();

        p.triangles_amount_1 = triangle_amount.value_.to_double();
        p.triangles_amount_2 = triangle_amount.value_.to_double();
		p.nodes_size_width = nodes_amount_x.value_.to_int();
		p.nodes_size_height = nodes_amount_y.value_.to_int();

		std::thread([&]{program.prepare();}).detach();
        
    };

	main_window() : form(API::make_center(800, 800)) {
        renderer_.observer = &program;

		

		place_.div("<vert<renderer>|25%<addinfo>>|30%<vert <vert "
				   "parameters>|25%<vert buttons>> margin = 10");
		place_["renderer"] << renderer_;
        place_["parameters"] << offset_1_x << offset_1_y << offset_2_x
							 << offset_2_y << triangle_width_1
							 << triangle_height_1 << triangle_width_2
							 << triangle_height_2 << triangle_amount << nodes_amount_x << nodes_amount_y;
		place_["buttons"] << startstop_button << update_button;
		place_["addinfo"] << info_textbox;

		startstop_button.events().click([&]() {
			// static bool running = false;
			// running = !running;
			// if (running) {
			// 	startstop_button.caption("Стоп");
            //    program.start_loop();
			// } else {
			// 	startstop_button.caption("Старт");
            //    program.stop_loop();
			// }
			std::thread([&]{program.evaluate();}).detach();
		});
		update_button.events().click([&] {
			std::thread([&]{update_button.enabled(false); update_program(); update_button.enabled(true);}).detach();
		});
		drawer_.draw(
			[&](paint::graphics g) { API::refresh_window(renderer_); });

		render_timer.elapse([&] {
			API::refresh_window(*this);

			std::string s;
			//info_textbox.append()
			//std::string s;
			
			//for (auto& i : program.v.nodes) {
			//	s += i.to_string() + " ";
			//}

			//info_textbox.reset(s);
		});
		render_timer.interval(std::chrono::milliseconds(1000/30));
        render_timer.start();
        place_.collocate();
	}
};