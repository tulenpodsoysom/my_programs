set objects=triangle_node_system.cpp program.cpp
set compile_args=-O3 -g -std=c++20
set includes=-IC:/my_lib/ -IC:/libs/nana/include
set libs=-lgdiplus -lgdi32 -lole32 -luser32 -lshell32 -lComdlg32 -lntdll -lC:\libs\_built_libs\nana -llibopenblas

clang++ %objects% %compile_args% %includes% %libs%