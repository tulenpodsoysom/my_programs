
#include <Windows.h>

#include <algorithm>
#include <cmath>
#include <format>
#include <gdiplus.h>


#include <iostream>
#include <ostream>
#include <physics/physical_system/triangulation_delone.hpp>
#include <nana/gui.hpp>

#include "nana/gui/detail/general_events.hpp"
#include "nana/gui/drawing.hpp"
#include "nana/gui/programming_interface.hpp"
#include "nana/gui/widgets/form.hpp"
#include "nana/paint/graphics.hpp"
#include <string>
#include <winuser.h>

int main()
{
    nana::form fm(nana::API::make_center(800,800));

    Gdiplus::GdiplusStartupInput gdiplus_startup_input;
    ULONG_PTR token;
    GdiplusStartup(&token,&gdiplus_startup_input,nullptr);

    geometry::point2d points[3] = {{1.0,3.0},{2.0,1.0},{3.0,-1.30}};

    auto circle = triangulation_delone<geometry::point2d>::generate_outer_circle(points[0], points[1], points[2]);


    auto inner_circle = triangulation_delone<geometry::point2d>::generate_inner_circle(points[0], points[1], points[2]);

    Gdiplus::Matrix transform_matrix;

    transform_matrix.Reset();
    transform_matrix.Scale(fm.size().width/10.0, fm.size().height/-10.0);
    transform_matrix.Translate(0, -7);
    geometry::point2d last_mouse_point;

    fm.events().resized([&](const nana::arg_resized& arg)
    {
        transform_matrix.Reset();
        transform_matrix.Scale(arg.width/10.0, arg.height/-10.0);
        transform_matrix.Translate(0, -7);
    });

    fm.events().mouse_move([&](const nana::arg_mouse& arg)
    {
        last_mouse_point = geometry::point2d{(double)arg.pos.x,(double)arg.pos.y};
        if (arg.left_button)
        {
            Gdiplus::PointF p = {(float)arg.pos.x,(float)arg.pos.y};
            transform_matrix.Invert();

            transform_matrix.TransformPoints(&p);

            transform_matrix.Invert();

            //std::cout << p.X << " " << p.Y << std::endl;
            float dist[3]{};

            for (int i = 0; i < 3; i++) {
                dist[i] = geometry::distance(points[i], geometry::point2d{p.X,p.Y});
            }
            auto ind = std::min_element(dist,dist+3) - dist;
            points[ind] = geometry::point2d{double(p.X),double(p.Y)};

            circle = triangulation_delone<geometry::point2d>::generate_outer_circle(points[0], points[1], points[2]);
            inner_circle = triangulation_delone<geometry::point2d>::generate_inner_circle(points[0], points[1], points[2]);
        }
        nana::API::refresh_window(arg.window_handle);
    });

    


    nana::drawing(fm).draw([&](nana::paint::graphics g)
    {
        Gdiplus::Graphics gdip((HDC)g.context());   
        Gdiplus::Pen pen(Gdiplus::Color(255, 0, 0), 0);
        Gdiplus::SolidBrush brush(Gdiplus::Color(255,0,0));
        Gdiplus::FontFamily font_family(L"Arial");
        Gdiplus::Font font(&font_family,11);
        Gdiplus::StringFormat format;
        format.SetAlignment(Gdiplus::StringAlignmentNear);

        std::wstring str = L"Mouse position: " + std::to_wstring(last_mouse_point.x()) + L" " + std::to_wstring(last_mouse_point.y()) + L"\r\n";
        str += L"point_1: " + std::to_wstring(points[0].x()) + L" " + std::to_wstring(points[0].y()) + L"\r\n";
        str += L"point_2: " + std::to_wstring(points[1].x()) + L" " + std::to_wstring(points[1].y()) + L"\r\n";
        str += L"point_3: " + std::to_wstring(points[2].x()) + L" " + std::to_wstring(points[2].y()) + L"\r\n";

        str += L"outer_circle_center: " + std::to_wstring(circle.center.x()) + L" " + std::to_wstring(circle.center.y()) + L" radius: " + std::to_wstring(sqrt(circle.R2)) + L"\r\n";
        str += L"inner_circle_center: " + std::to_wstring(inner_circle.center.x()) + L" " + std::to_wstring(inner_circle.center.y()) + L" radius: " + std::to_wstring(sqrt(inner_circle.R2)) + L"\r\n";
        //str += L"triangle_goodness: " + std::to_wstring(std::sqrt(circle.R2) - 2*std::sqrt(inner_circle.R2)) + L"\r\n";     
        str += L"triangle_goodness: " + std::to_wstring(2*std::sqrt(inner_circle.R2/circle.R2)) + L"\r\n";     
        gdip.DrawString(str.c_str(),str.length(),&font,Gdiplus::RectF{0,0,400,200},&format,&brush);
        Gdiplus::Status s = gdip.SetTransform(&transform_matrix);
        
        for (int i = 0; i < 3; i++) {
            float r = 0.05f;
            gdip.DrawEllipse(&pen, points[i].x()-r,points[i].y()-r,2*r,2*r);
        }
        gdip.DrawLine(&pen,Gdiplus::PointF{(float)points[0].x(),(float)points[0].y()},{(float)points[1].x(),(float)points[1].y()});
        gdip.DrawLine(&pen,Gdiplus::PointF{(float)points[0].x(),(float)points[0].y()},{(float)points[2].x(),(float)points[2].y()});
        gdip.DrawLine(&pen,Gdiplus::PointF{(float)points[1].x(),(float)points[1].y()},{(float)points[2].x(),(float)points[2].y()});

        float r = sqrtf(circle.R2);
        gdip.DrawEllipse(&pen,circle.center.x()-r,circle.center.y()-r,2*r,2*r);
        r = sqrtf(inner_circle.R2);
        gdip.DrawEllipse(&pen,inner_circle.center.x()-r,inner_circle.center.y()-r,2*r,2*r);
    });



    fm.show();
    nana::exec();

    Gdiplus::GdiplusShutdown(token);
    return 0;
}

