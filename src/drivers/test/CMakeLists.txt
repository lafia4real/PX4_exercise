############################################################################
#		TEST
############################################################################
px4_add_module(
	MODULE drivers__test
	MAIN test
	COMPILE_FLAGS
	SRCS
		test.cpp
	DEPENDS
	)
