#include "Halide.h"
#include "halide_image_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define L 1
#define W 1

using namespace Halide;
using namespace Halide::ConciseCasts;

float F_conv_g_sig_s() {return 1;}

float G_conv_g_sig_s() {return 1;}

float choose(int n, int k) { return 1;}


float c_sum(float *c_f, int M, int N) {
	float result = 0;

	for(int i = M; i <= N - M; i++) {
		result += c_f[i];
	}

	return result;
}

float find_largest(float *c_f, float epsi_f) {

	float largest = 0;
	int N = sizeof(c_f);

	for(int M = 0; M <= N; M++) {
		float sum = c_sum(c_f, M, N);
		if(sum > largest) largest = sum;
	}

	return largest;
}


int main(int argc, char**argv) {

	Buffer<uint8_t> input = Tools::load_image(argv[1]);

	float sig_s_f = atof(argv[2]);
	float sig_r_f = atof(argv[3]);
	float epsi_f  = atof(argv[4]);

	Var x, y, c;
	Func P, Q;

	P(x, y, c) = 0;
	Q(x, y, c) = 0;

	Func box_filtered;
	RDom l_r(-L, L, -L, L);
	

	
	//float box_filtered_weight_f = (1 / pow(2 * L + 1, 2));
	float box_filtered_weight_f = 1;
	Expr box_filtered_weight = box_filtered_weight_f;
	
	// Compute box filtered image
	box_filtered(x, y, c) = box_filtered_weight * sum(input(x - l_r.x, y - l_r.y, c));

	int T = 255;
	float N_weight = 0.405;
	//int N = N_weight * (T / sig_r_f) * (T / sig_r_f);
	int N = 1;
	printf("N = %d\n", N);

	int M;
	float *c_f;
	float *w_f;
	
	c_f = new float[N];
	w_f = new float[N];

	

	if(N < 40) {
		M = 0;

		for(int n = 0; n <= N; n++) {

			/*c_f[n] = (1 / pow(2, N)) * choose(N, n);
			w_f[n] = (2 * n - N) / (sig_r_f * sqrt(N));
			*/
			c_f[n] = 1;
			w_f[n] = 1;

		}

	} else if (40 <= N < 100) {

		for(int n = 0; n <= N; n++) {

			/*c_f[n] = (1 / pow(2, N)) * choose(N, n);
			w_f[n] = (2 * n - N) / (sig_r_f * sqrt(N));
			*/
			c_f[n] = 1;
			w_f[n] = 1;
		}

		epsi_f = 0.01;
		M = int(find_largest(c_f, epsi_f));
		

	} else if (N >= 100) {

		epsi_f = 0.1;
		//M = 0.5 * (N - sqrt(4 * N * log(2 / epsi_f)));
		M = 1;
		for(int n = 0; n <= M; n++) {
			/*c_f[n] = (1 / pow(2, N)) * choose(N, n);
			w_f[n] = ((2 * n - N) / sig_r_f) * sqrt(N);  
			*/
			c_f[n] = 1;
			w_f[n] = 1;
		}

		for(int n = N - M; n <= N; n++) {
			/*c_f[n] = (1 / pow(2, N)) * choose(N, n);
			w_f[n] = ((2 * n - N) / sig_r_f) * sqrt(N);  
			*/
			c_f[n] = 1;
			w_f[n] = 1;
		}
	}

	printf("M = %d\n", M);

	Buffer<Handle> G(input.width(), input.height(), input.channels());
	Buffer<Handle> G(input.width(), input.height(), input.channels());
	Buffer<Handle> F(input.width(), input.height(), input.channels());
	Buffer<Handle> F(input.width(), input.height(), input.channels());
	Buffer<Handle> H(input.width(), input.height(), input.channels());
	Buffer<Handle> H(input.width(), input.height(), input.channels());
	Buffer<Handle> P(input.width(), input.height(), input.channels());
	Buffer<Handle> P(input.width(), input.height(), input.channels());
	Buffer<Handle> Q(input.width(), input.height(), input.channels());
	Buffer<Handle> Q(input.width(), input.height(), input.channels());
	


	for(int n = M; n <= N - M; n++) {

		G(x, y, c) = cos(w_f[n] * box_filtered(x, y, c));
		G(x, y, c) = sin(w_f[n] * box_filtered(x, y, c));

		F(x, y, c) = cos(w_f[n] * box_filtered(x, y, c)) * input(x, y, c);
		F(x, y, c) = sin(w_f[n] * box_filtered(x, y, c)) * input(x, y, c);

		H(x, y, c) = c_f[n] * G(x, y, c);
		H(x, y, c) = -(c_f[n] * G(x, y, c));

		P(x, y, c) = P(x, y, c) + H(x, y, c) * F_conv_g_sig_s();
		

		Q(x, y, c) = Q(x, y, c) + H(x, y, c) * F_conv_g_sig_s();
		
	}

	Func output;
	output(x, y, c) = P(x, y, c) / Q(x, y, c);
	
	Buffer<uint8_t> result_img = output.realize(input.width(), input.height(), input.channels()); 

}
