target("Physics2DDemo")
	set_group("Examples")
	set_kind("binary")
	add_deps("NazaraGraphics", "NazaraPhysics2D")
	add_packages("entt")
	add_files("main.cpp")