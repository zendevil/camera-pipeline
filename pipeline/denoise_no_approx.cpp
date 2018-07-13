#include <stdio.h>
#include <math.h>
#include <complex>
#include "Halide.h"
#include "halide_image_io.h"
#include "denoise.h"
#include <time.h>



using namespace Halide;
using namespace Halide::ConciseCasts;
using namespace std;


int main(int argc, char **argv) {

	clock_t time;
	time = clock();

	Buffer<uint8_t> input = Tools::load_image(argv[1]);


		
	printf("channels=%d\n", input.channels());
	float sig_s_f = atof(argv[2]);
	float sig_r_f = atof(argv[3]);
	int L = sig_r_f * 3;
	int W = sig_s_f * 3;

	Var x, y, c, t;

	// declare range and spatial filters
	Func g_sig_s, g_sig_r;
	RDom omega(-W, W, -W, W), l_r(-L, L, -L, L);;


	g_sig_s(x, y) = f32(0);
	

	g_sig_s(omega.x, omega.y) = f32(exp(-(omega.x * omega.x + omega.y * omega.y) / (2 * sig_s_f * sig_s_f)));

	g_sig_r(t) = f32(exp(- t * t / (2 * sig_r_f * sig_r_f)));


	Func imp_bi_filter, imp_bi_filter_num, imp_bi_filter_den, imp_bi_filter_num_clamped, imp_bi_filter_den_clamped;


	Func box_filtered;

	box_filtered(x, y, c) = u8((float)(1) / ((2 * L + 1) * (2 * L + 1)) * sum(input(x - l_r.x, y - l_r.y, c)));
	
	// Compute box filtered image
/*	Expr clamped_x = clamp(x, L, input.width() - 2 * L - 1);
	Expr clamped_y = clamp(y, L, input.height() - 2 * L - 1);

	imp_bi_filter_num(x, y, c) = f32(sum(g_sig_s(omega.x, omega.y) * g_sig_r(box_filtered(x - omega.x, y - omega.y, c) - box_filtered(x, y, c)) * input(x - omega.x, y - omega.y, c)));
	imp_bi_filter_den(x, y, c) = f32(sum((g_sig_s(omega.x, omega.y)) * g_sig_r((box_filtered(x - omega.x, y - omega.y, c) - box_filtered(x, y, c)))));
	imp_bi_filter_num_clamped(x, y, c) = imp_bi_filter_num(clamped_x, clamped_y, c);
	imp_bi_filter_den_clamped(x, y, c) = imp_bi_filter_den(clamped_x, clamped_y, c);
	
	imp_bi_filter(x, y, c) = u8(imp_bi_filter_num_clamped(x, y, c) / imp_bi_filter_den_clamped(x, y, c));
	//imp_bi_filter.trace_stores()
*/
	Buffer<uint8_t> shifted(input.width() - 2 * W, input.height() - 2 * W, input.channels());
	shifted.set_min(W, W);
	box_filtered.trace_stores();
	box_filtered.realize(shifted);

	Tools::save_image(shifted, "box_filtered.png");

	time = clock() - time;

	printf("runtime = %f\n", (float) time / CLOCKS_PER_SEC);

}
