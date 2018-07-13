#include<stdio.h>
#include "Halide.h"
#include "halide_image_io.h"
#include <random>
using namespace Halide;
using namespace Halide::ConciseCasts;

int main(int argc, char** argv) {
	Buffer<uint8_t> input = Tools::load_image("input.png");
	Buffer<int32_t> b(input.width(), input.height(), input.channels());

	Var x, y, c;
	Func F, S;

	S(x, y, c)  = int32_t(0);
	
	for(int y_ = 0; y_ < input.height(); y_++) {
		for(int x_ = 0; x_ < input.width(); x_++) {
	
			int32_t t = rand();
			int32_t u = rand();
			int32_t v = rand();
		
			S(x_, y_, 0) = t;  
			S(x_, y_, 1) = u;
			S(x_, y_, 2) = v;
		}	

	}
	// S.trace_stores();
	S.realize(b);
}