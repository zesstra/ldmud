#pragma strong_types, save_types, pedantic
inherit "ti-argtypes";

// Adding an argument should be okay, as long the types match.
varargs void fun_varargs_xvarargs(string first, object second, int third, varargs int* rest) {}

int run_test() { return 1; }
