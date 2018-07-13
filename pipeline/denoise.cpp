#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Halide.h"
#include "halide_image_io.h"
#include "denoise.h"
#define L 1
#define W 1

using namespace Halide;
using namespace Halide::ConciseCasts;


int main(int argc, char **argv) {
    
    Buffer<uint8_t> input = Tools::load_image(argv[1]);
    Func box_filtered, G, F, H, P, Q, g_sig_s, eq_5_helper_1, fin, w, c, nCk;
    Var x, y, i;
    RDom r(-L, L);
    RDom r1(-W, W);

    Expr sig_s;
    sig_s = atoi(argv[2]);
    printf("sig_s=%d\n", sig_s); 
    Expr sig_r;
    sig_r = atoi(argv[3]);
    printf("sig_r=%d\n", sig_r);
    int M;
   
    Expr T = f32(0); 
    T = atof(argv[5]);    
    float epsilon = atof(argv[4]);
    float weight = 0.405;
    Expr N = weight * pow(T / sig_s, 2);
    printf("T / sig_s = %f\n", T / sig_s);
    printf("N=%f\n", N);
    float eq_5_helper_0 = (1 / pow(2 * L + 1, 2));
    Expr g_sig_r_helper = (2 * pow(sig_s, 2)); 
        
    G  (x, y) = {f32(0), f32(0)};
    H  (x, y) = {f32(0), f32(0)};
    F  (x, y) = {f32(0), f32(0)};
    P  (x, y) = {f32(0), f32(0)};
    Q  (x, y) = {f32(0), f32(0)};
    
    g_sig_s(x, y) = -(pow(x, 2) + pow(y, 2)) / g_sig_r_helper; 
    eq_5_helper_1(x, y) = sum(input(x - r, y - r));
    box_filtered(x, y) = eq_5_helper_0 * eq_5_helper_1(x, y); 
    //Expr T = maximum(box_filtered(x - W, y - W) - box_filtered(x, y));

    c(x) = f32(0);
    w(x) = f32(0);
    RDom comp(0, 1000);
    comp.where(comp ) 
    if(N < 40) {

        M = 0;
        RDom n(0, N);
        w(n) = (2 * n - N) / (sig_r * sqrt(N));
        c(n) = 1 / pow(2, N) * nCk(N, n); 
        /*for(int n = 0; n <= N; n++) {
            c[n] = 1 / pow(2, N) * nCk(N, n);
            w[n] = (2 * n - N) / (sig_r * sqrt(N));
        }*/

    } else if(40 <= N < 100) {
        float temp =  1 / pow(2, N);
        printf("temp=%f\n", temp); 
        c(n) = temp * nCk(N, n);
        w(n) = (2 * n - N) / (sig_r * sqrt(N));
        epsilon = 0.01;
        M = find_largest(epsilon, c, N);
        /*for(int n = 0; n <= N; n++) {
            c[n] = temp * nCk(N, n);

//            printf("1 / temp =%f\n", temp);  
//            printf("c[%d] = %f\n", n, c[n]);

            w[n] = (2 * n - N) / (sig_r * sqrt(N));
            epsilon = 0.01;
            M = find_largest(epsilon, c, N);                
        }    
*/
    } else if(N >= 100) {
    
        epsilon = 0.1;
        M = 0.5 * (N - sqrt(4 * N * log(2 / epsilon)));
    
    }
    printf("M=%d\n", M);
    printf("N-M=%f\n", N-M);
    RDom n(M, N-M);

    G(x, y)[0] = cos(w(n) * box_filtered(x, y));
    G(x, y)[1] = sin(w(n) * box_filtered(x, y));
    F(x, y)[0] = G(x, y)[0] * input(x, y);
    F(x, y)[1] = G(x, y)[1] * input(x, y);

    H(x, y)[0] = c(n) * G(x, y)[0];
    H(x, y)[1] = c(n) * G(x, y)[1];
    P(x, y)[0] = P(x, y)[0] + H(x, y)[0] * (sum(g_sig_s(r1, r1) * F(x - r1, y - r1)[0]));
    P(x, y)[1] = P(x, y)[0] + H(x, y)[0] * (sum(g_sig_s(r1, r1) * F(x - r1, y - r1)[0]));

    Q(x, y)[0] = Q(x, y)[0] + H(x, y)[0] * (sum(g_sig_s(r1, r1) * G(x - r1, y - r1)[0]));
    Q(x, y)[0] = Q(x, y)[0] + H(x, y)[0] * (sum(g_sig_s(r1, r1) * G(x - r1, y - r1)[0]));

    G.parallelize(n);
    F.parallelize(n);
    H.parallelize(n);
    P.parallelize(n);
    Q.parallelize(n);
 
  /*  for(int n = M; n <= N - M; n++) {

        G(x, y, 0) = cos(w[n] * box_filtered(x, y));
        G(x, y, 1) = sin(w[n] * box_filtered(x, y));
        G.compute_root();
        F(x, y, 0) = G(x, y, 0) * input(x, y);
        F(x, y, 1) = G(x, y, 1) * input(x, y);
        F.compute_root();
        H(x, y, 0) = c[n] * G(x, y, 0);
        H(x, y, 1) = c[n] * G(x, y, 1);
        H.compute_root();

        P(x, y, i) = P(x, y, i) + H(x, y, i) * (sum(g_sig_s(r1, r1) * F(x - r1, y - r1, i)));
        Q(x, y, i) = Q(x, y, i) + H(x, y, i) * (sum(g_sig_s(r1, r1) * G(x - r1, y - r1, i)));
        P.compute_root();
        Q.compute_root();
    }*/
    
    fin(x, y) = P(x, y)[1] / Q(x, y)[1];
    //G.trace_stores(); // 1 and 0
    //F.trace_stores(); // ok
    //H.trace_stores(); 
    //P.trace_stores(); 0
    //Q.trace_stores(); 0
    //fin.trace_stores(); 0
    printf("input.width()=%d\n", input.width());
    printf("input.height()=%d\n", input.height());
    Buffer<uint8_t> output = fin.realize(input.width() - 2, input.height() - 2);
   
    Tools::save_image(output, "output1.png");
    
    

}


double nCk(Expr n, Expr k) {

    if(k > n) return 0;
    if(k * 2 > n) k = n-k;
    if(k == 0) return 1;
    
    double result = n;

    for(int i = 2; i <= f32(k); ++i) {
        result *= (n - i + 1);
        result /= i;
    }

    return result;
}


// TODO
int find_largest(double epsilon, Func c, int N) {
    
    double max_M = 0;
    double sum;
    
    for(int m = N; m > 0; m--) {
        sum = 0;
        for(int i = m; i <= N - m; i++) {
            if(sum > 1 - epsilon / 2) return sum;
            sum += c[i];
        } 
    }    
}
