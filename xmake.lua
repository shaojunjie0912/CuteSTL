set_project("CuteSTL")
set_xmakever("2.9.7")
set_languages("c++20")

set_defaultmode("debug")
set_warnings("allextra")

add_rules("mode.debug", "mode.release")

add_requires("fmt")
add_packages("fmt")

includes("CuteSTL")
includes("tests")

