#pragma strong_types, save_types, pedantic
inherit "ti-argtypes";

// Making an argument xvarargs should be okay.
void fun(string first, varargs object* second) {}

int run_test() { return 1; }
