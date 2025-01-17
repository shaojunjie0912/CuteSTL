set_project("CuteSTL")
set_xmakever("2.9.7")

set_languages("cxx23")
set_defaultmode("debug")

add_rules("plugin.compile_commands.autoupdate")
add_rules("mode.debug", "mode.release")

add_requires("fmt")

includes("CuteSTL")
includes("tests")

