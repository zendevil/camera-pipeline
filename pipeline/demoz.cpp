#include "Halide.h" 
#include <stdio.h>
#include <stdlib.h>
#include "halide_image_io.h"

using namespace Halide;
using namespace Halide::ConciseCasts;
// Color channels: 0-red, 1-green, 2-blue

/* Bayer filter: 
    
    b g
    g r
*/ 



/*
http://www.arl.army.mil/arlreports/2010/arl-tr-5061.pdf
*/ 

int main(int argc, char **argv) {
    

/* Excerpt from the paper: "Following interpolation of all missing G values, B/R values are interpolated at R/B pixels using the full green color pane."*/
    Buffer<uint8_t> input = Tools::load_image(argv[1]);
    Func fin("fin");
    Func b_ne, b_se, b_sw, b_nw, w_ne, w_se, w_sw, w_nw, b_ne_est, b_se_est, b_sw_est, b_nw_est, g_n, g_e, g_s, g_w, w_n, w_e, w_s, w_w, g_n_est, g_e_est, g_s_est, g_w_est;
            
    RDom r(2, input.width() - 4, 2, input.height() - 4);
    r.where((r.x % 2 == 0 && r.y % 2 == 0) || (r.x % 2 == 1 && r.y % 2 == 1));

    RDom r_p(2, input.width() - 4, 2, input.height() - 4);
    r_p.where(r_p.x % 2 == 1 && r_p.y % 2 == 1);

    RDom b_p(2, input.width() - 4, 2, input.height() - 4);
    b_p.where(b_p.x % 2 == 0 && b_p.y % 2 == 0);
        
    Var x("x"), y("y"), c("c");
    

    Buffer<int16_t> input_16(input.width(), input.height());
    input_16(x, y) = i16(input(x, y));
 
    // for rgb values for red green and blue bayer pixels
    
    fin(x, y, c) = input(x, y);

    g_n(x, y) = u16 (0);
    g_e(x, y) = u16 (0);
    g_s(x, y) = u16 (0);
    g_w(x, y) = u16 (0);
 
    w_n(x, y) = f32 (0);
    w_e(x, y) = f32 (0);
    w_s(x, y) = f32 (0);
    w_w(x, y) = f32 (0);
    
    g_n_est(x, y) = i16 (0);
    g_e_est(x, y) = i16 (0);
    g_s_est(x, y) = i16 (0);
    g_w_est(x, y) = i16 (0);

        
    b_ne(x, y) = u16 (0);
    b_se(x, y) = u16 (0);
    b_sw(x, y) = u16 (0);
    b_nw(x, y) = u16 (0);

    w_ne(x, y) = f32 (0);
    w_se(x, y) = f32 (0);
    w_sw(x, y) = f32 (0);
    w_nw(x, y) = f32 (0);

    b_ne_est(x, y) = i16 (0);
    b_se_est(x, y) = i16 (0);
    b_sw_est(x, y) = i16 (0);
    b_nw_est(x, y) = i16 (0);

        

    printf("width: %d\n", input.width()); 
    printf("height: %d\n", input.height());
    printf("channel: %d\n", input.channels());

    g_n(r.x, r.y) = abs((input_16(r.x, r.y + 1) - input_16(r.x, r.y - 1))) + abs((input_16(r.x, r.y) - input_16(r.x, r.y - 2)));
    g_e(r.x, r.y) = abs((input_16(r.x - 1, r.y) - input_16(r.x + 1, r.y))) + abs((input_16(r.x, r.y) - input_16(r.x + 2, r.y)));
    g_s(r.x, r.y) = abs((input_16(r.x, r.y - 1) - input_16(r.x, r.y + 1))) + abs((input_16(r.x, r.y) - input_16(r.x, r.y + 2)));
    g_w(r.x, r.y) = abs((input_16(r.x + 1, r.y) - input_16(r.x - 1, r.y))) + abs((input_16(r.x, r.y) - input_16(r.x - 2, r.y)));

    w_n(r.x, r.y) = f32(1 / (1 + g_n(r.x, r.y)));
    w_e(r.x, r.y) = f32(1 / (1 + g_e(r.x, r.y)));
    w_s(r.x, r.y) = f32(1 / (1 + g_s(r.x, r.y)));
    w_w(r.x, r.y) = f32(1 / (1 + g_w(r.x, r.y)));

    g_n_est(r.x, r.y) = (input_16(r.x, r.y - 1) + (input_16(r.x, r.y) - input_16(r.x, r.y - 2))) / 2;
    g_e_est(r.x, r.y) = (input_16(r.x + 1, r.y) + (input_16(r.x, r.y) - input_16(r.x + 2, r.y))) / 2;
    g_s_est(r.x, r.y) = (input_16(r.x, r.y + 1) + (input_16(r.x, r.y) - input_16(r.x, r.y + 2))) / 2;
    g_w_est(r.x, r.y) = (input_16(r.x - 1, r.y) + (input_16(r.x, r.y) - input_16(r.x - 2, r.y))) / 2;

    
    /*g_n.compute_at(fin, r.y);
    g_e.compute_at(fin, r.y);
    g_s.compute_at(fin, r.y);
    g_w.compute_at(fin, r.y);

    w_n.compute_at(fin, r.y); 
    w_e.compute_at(fin, r.y);
    w_s.compute_at(fin, r.y);
    w_w.compute_at(fin, r.y);

    g_n_est.compute_at(fin, r.y);
    g_e_est.compute_at(fin, r.y);
    g_s_est.compute_at(fin, r.y);
    g_w_est.compute_at(fin, r.y);
  
*/
    Expr green, red, blue;
    green =  ((w_n(r.x, r.y) * g_n_est(r.x, r.y)+ w_e(r.x, r.y) * g_e_est(r.x, r.y) + w_s(r.x, r.y) * g_s_est(r.x, r.y)+ w_w(r.x, r.y) * g_w_est(r.x, r.y)) / (w_n(r.x, r.y) + w_e(r.x, r.y) + w_s(r.x, r.y) + w_w(r.x, r.y)));
    fin(r.x, r.y, 1) = u8(green);
         
    // Var x_outer, y_outer, x_inner, y_inner, tile_index, y_inner_outer, y_pairs, x_vectors, x_inner_outer;
    
    /*green
        .tile(x, y, x_outer, y_outer, x_inner, y_inner, 64, 64)
        .fuse(x_outer, y_outer, tile_index)
        .parallel(tile_index);

    green
        .tile(x_inner, y_inner, x_inner_outer, y_inner_outer, x_vectors, y_pairs, 6, 4)
        .vectorize(x_vectors)
        .unroll(y_pairs);
*/

    Buffer<int16_t> buff(input.width(), input.height()); 
    buff(x,y) = i16(green);
    assert(buff.type() == Int(16));
    b_ne(r.x, r.y) = abs((input_16(r.x - 1, r.y + 1) - input_16(r.x + 1, r.y - 1))) + abs((buff(r.x, r.y) - buff(r.x + 1, r.y - 1)));
    b_se(r.x, r.y) = abs((input_16(r.x - 1, r.y - 1) - input_16(r.x + 1, r.y + 1))) + abs((buff(r.x, r.y) - buff(r.x + 1, r.y + 1)));
    b_sw(r.x, r.y) = abs((input_16(r.x + 1, r.y - 1) - input_16(r.x - 1, r.y + 1))) + abs((buff(r.x, r.y) - buff(r.x - 1, r.y + 1)));
    b_nw(r.x, r.y) = abs((input_16(r.x + 1, r.y + 1) - input_16(r.x - 1, r.y - 1))) + abs((buff(r.x, r.y) - buff(r.x - 1, r.y - 1)));
    
    //buff.compute_at(b_ne, r.y);
    //buff.compute_at(b_se, r.y);
    //buff.compute_at(b_sw, r.y);
    //buff.compute_at(b_nw, r.y);




    w_ne(r.x, r.y) = f32(1 / (1 + b_ne(r.x, r.y)));
    w_se(r.x, r.y) = f32(1 / (1 + b_se(r.x, r.y)));
    w_sw(r.x, r.y) = f32(1 / (1 + b_nw(r.x, r.y)));
    w_nw(r.x, r.y) = f32(1 / (1 + b_sw(r.x, r.y)));

    /*b_ne.compute_at(w_ne, y);
    b_se.compute_at(w_ne, y);
    b_sw.compute_at(w_ne, y);
    b_nw.compute_at(w_ne, y);
*/

    b_ne_est(r.x, r.y) = (input(r.x    , r.y    )) + (buff(r.x, r.y) - buff(r.x + 1, r.y - 1)) / 2;
    b_se_est(r.x, r.y) = (input(r.x + 1, r.y + 1)) + (buff(r.x, r.y) - buff(r.x + 1, r.y + 1)) / 2;
    b_sw_est(r.x, r.y) = (input(r.x - 1, r.y + 1)) + (buff(r.x, r.y) - buff(r.x - 1, r.y + 1)) / 2;
    b_nw_est(r.x, r.y) = (input(r.x - 1, r.y - 1)) + (buff(r.x, r.y) - buff(r.x - 1, r.y - 1)) / 2;

    //green.compute_at(b_ne_est, r.y);
    //green.compute_at(b_se_est, r.y);
    //green.compute_at(b_nw_est, r.y);
    //green.compute_at(b_sw_est, r.y);



    
    red = u8((w_ne(r_p.x, r_p.y) * b_ne_est(r_p.x, r_p.y) + w_se(r_p.x, r_p.y) * b_se_est(r_p.x, r_p.y) + w_sw(r_p.x, r_p.y) * b_sw_est(r_p.x, r_p.y) + w_nw(r_p.x, r_p.y) 
        * b_nw_est(r_p.x, r_p.y)) / (w_ne(r_p.x, r_p.y) + w_se(r_p.x, r_p.y) + w_sw(r_p.x, r_p.y) + w_nw(r_p.x, r_p.y)));

    fin(r_p.x, r_p.y, 0) = red;
    blue = u8((w_ne(b_p.x, b_p.y) * b_ne_est(b_p.x, b_p.y) + w_se(b_p.x, b_p.y) * b_se_est(b_p.x, b_p.y) + w_sw(b_p.x, b_p.y) * b_sw_est(b_p.x, b_p.y) + w_nw(b_p.x, b_p.y) 
        * b_nw_est(b_p.x, b_p.y)) / (w_ne(b_p.x, b_p.y) + w_se(b_p.x, b_p.y) + w_sw(b_p.x, b_p.y) + w_nw(b_p.x, b_p.y)));
    
    fin(b_p.x, b_p.y, 2) = blue;
    
    /*b_ne.compute_at(fin, r.y);
    b_se.compute_at(fin, r.y);
    b_nw.compute_at(fin, r.y);
    b_sw.compute_at(fin, r.y);
    
    b_ne_est.compute_at(fin, r.y);
    b_se_est.compute_at(fin, r.y);
    b_nw_est.compute_at(fin, r.y);
    b_sw_est.compute_at(fin, r.y);

    w_ne.compute_at(fin, r.y);
    w_se.compute_at(fin, r.y);
    w_nw.compute_at(fin, r.y);
    w_sw.compute_at(fin, r.y);
*/
    fin.trace_stores();
   
    Buffer<uint8_t> result = fin.realize(input.width(), input.height(), input.channels());

    Tools::save_image(result, "result.png");
    printf("result.png saved\n");

    


   
} 
