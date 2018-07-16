#include <stdio.h>
#include <string>
#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>
#include <vector>
#include <list>
#include <time.h>
#include "Halide.h"
#include "halide_image_io.h"
#include "temporal.h"
#include <unordered_map>

using namespace std;
using namespace Halide;
using namespace Halide::ConciseCasts;

const int width = 50;
const int height = 50;
const int n_channels = 3;
const int n_frames = 5;

const int K = 10; //number of neighbors

typedef vector<short> coord_t;
typedef vector<short> coord_ssd_t;
typedef vector<coord_ssd_t> heap_t;

heap_t* neighbors_h[height][width];


int main(int argc, char** argv) 
{

    clock_t t = clock();
    get_input();
    load_halide_functions(); 
    initiate_neighbors();
    print_neighbors(neighbors_h[1][1]);
    propagate_neighbors();
    
    // random_search();
    
    t = clock() - t;
    cout<<(float)t/CLOCKS_PER_SEC<<" seconds"<<endl;
}

Buffer<uint8_t> input[n_frames];

void get_input() 
{
    string path;

    for(int i = 0; i < n_frames; i++) {

        path = "./frames/f_" + to_string(i + 1) + ".png";
        input[i] = Tools::load_image(path);   

    }

}


Func D, I, Dx_right_in, Dx_left_in, Dy_up_in, Dy_down_in, 
        Dx_right_out, Dx_left_out, Dy_up_out, Dy_down_out;


void load_halide_functions() 
{
        const int s = 10;
    Var x, y, x_i, y_i, c, i, xo, yo, xi, yi;
  
    RDom u(-s, s, -s, s);
    
    RDom dx_left_out(-s - 1, -s, -s, s);
    RDom dx_left_in(-s, -s + 1, -s, s);
    
    RDom dx_right_out(s, s + 1, -s, s); 
    RDom dx_right_in(s - 1, s, -s, s);
    
    RDom dy_up_out(-s, s, -s - 1, -s);
    RDom dy_up_in(-s, s, -s, -s + 1);
    
    RDom dy_down_out(-s, s, s, s + 1);
    RDom dy_down_in(-s, s, s - 1, s);
    
    I(x, y, c) = input[0](clamp(x, s, width - s), clamp(y, s, height - s), c);   
    
    D(x, y, x_i, y_i, c) = i16(sum(pow((I(x + u.x, y + u.y, c) 
                            - I(x_i + u.x, y_i + u.y, c)), 2)));
    
    Dx_left_in(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dx_left_in.x, y + dx_left_in.y, c) 
                                    - I(x_i + dx_left_in.x, y_i + dx_left_in.y, c)), 2)));
    
    Dx_right_in(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dx_right_in.x, y + dx_right_in.y, c) 
                                    - I(x_i + dx_right_in.x, y_i + dx_right_in.y, c)), 2)));
    
    Dy_up_in(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dy_up_in.x, y + dy_up_in.y, c) 
                                    - I(x_i + dy_up_in.x, y_i + dy_up_in.y, c)), 2)));
    
    Dy_down_in(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dy_down_in.x, y + dy_down_in.y, c) 
                                    - I(x_i + dy_down_in.x, y_i + dy_down_in.y, c)), 2)));
    
    Dx_left_out(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dx_left_out.x, y + dx_left_out.y, c) 
                                    - I(x_i + dx_left_out.x, y_i + dx_left_out.y, c)), 2)));
    
    Dx_right_out(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dx_right_out.x, y + dx_right_out.y, c) 
                                    - I(x_i + dx_right_out.x, y_i + dx_right_out.y, c)), 2)));
    
    Dy_up_out(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dy_up_out.x, y + dy_up_out.y, c) 
                                    - I(x_i + dy_up_out.x, y_i + dy_up_out.y, c)), 2)));
    
    Dy_down_out(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dy_down_out.x, y + dy_down_out.y, c) 
                                    - I(x_i + dy_down_out.x, y_i + dy_down_out.y, c)), 2)));
}


void initiate_neighbors() 
{
    srand(17);
    for(short y = 0; y < height; y++) {
        for(short x = 0; x < width; x++) {
            
            heap_t* neighbors;
            neighbors = new heap_t;
            neighbors->reserve(40); 
            generate_random_offsets_and_ssds(x, y, neighbors); 
            sort_neighbors(neighbors);
            neighbors_h[y][x] = neighbors;
                        
         } 
    }
     
    cout<<"initial heap size "<<neighbors_h[0][0]->size()<<endl;
}

void generate_random_offsets_and_ssds(short x, short y, heap_t* neighbors)
{
    for(short i = 0; i < K; i++) {
        coord_ssd_t v_i_ssd = get_neighbor_ssd(x, y);
        neighbors->push_back(v_i_ssd);
    }

}


void print_offset_and_ssd(coord_ssd_t offset_ssd) 
{
    cout<<"x_i "<<offset_ssd.at(0)<<" y_i "<<offset_ssd.at(1)
        <<" ssd "<<offset_ssd.at(2)<<endl;
}

void print_neighbors(heap_t* heap) 
{
    for(uint i = 0; i < heap->size(); i++) {
        print_offset_and_ssd(heap->at(i));
    }
}

coord_t get_random_coord() 
{
    return {get_random_x(), get_random_y()};
}



short get_random_x() 
{
    float random_x;
    for(;;) {
        random_x = width / 3 * box_muller_trans((float) rand() / RAND_MAX);   
        if(random_x < width) 
            return (short) random_x;
    }
}

short get_random_y() 
{
    float random_y;
    for(;;) {
        random_y = height / 3 * box_muller_trans((float) rand() / RAND_MAX);   
        if(random_y < height) 
            return (short) random_y;
    }
}



// converts a uniform random variable into a standard normal variable
float box_muller_trans(float x) 
{
    return sqrt(-2 * log(x)) * cos(2 * M_PI * x);;
}


coord_ssd_t get_neighbor_ssd(short x, short y) 
{

    Buffer<short> pix(1, 1, 1, 1, 3);
    coord_t coord = get_random_coord();
    pix.set_min(x, y, x + coord[0], y + coord[1]);   
    D.realize(pix);
    coord.push_back(pix(x, y, x + coord[0], y + coord[1], 0)); //pushing ssd
    return coord;

}


void propagate_neighbors() 
{
    for(short y = 1; y < height; ++y)     
        for(short x = 1; x < width; ++x) {
            cout<<"y "<<y<<" x "<<x<<endl;
            propagate_scanline(x, y);
            if(x == 1 && y == 1)
                print_neighbors(neighbors_h[1][1]);
        }
   
    for(short y = height - 2; y >= 0; --y)     
        for(short x = width - 2; x >= 0; --x) {
            cout<<"y "<<y<<" x "<<x<<endl;
            propagate_reverse_scanline(x, y);
        }
}



void propagate_scanline(short x, short y)
{
    short offset_x, offset_y, offset_ssd;
    
    for(heap_t::iterator it = neighbors_h[y][x - 1]->begin(); 
        it != neighbors_h[y][x - 1]->end(); ++it) {
        
        offset_x = it->at(0);
        offset_y = it->at(1);
        offset_ssd = it->at(2);    
        
        push_in_heap(neighbors_h[y][x], {(short)(offset_x + 1), offset_y, 
            calculate_new_ssd(x, y, offset_x, offset_y, offset_ssd, 'r')});
    }
    
    for(heap_t::iterator it = neighbors_h[y - 1][x]->begin(); 
        it != neighbors_h[y - 1][x]->end(); ++it) {
        
        offset_x = it->at(0);
        offset_y = it->at(1);
        offset_ssd = it->at(2);
       
        push_in_heap(neighbors_h[y][x], {offset_x, (short)(offset_y + 1), 
            calculate_new_ssd(x, y, offset_x, offset_y, offset_ssd, 'd')}); 
    }
}


void propagate_reverse_scanline(short x, short y)
{
    for(heap_t::iterator it = neighbors_h[y][x + 1]->begin(); 
        it != neighbors_h[y][x + 1]->end(); ++it) {
        
        short offset_x = it->at(0);
        short offset_y = it->at(1);
        short offset_ssd = it->at(2);    
        
        neighbors_h[y][x]->push_back({(short)(offset_x - 1), offset_y, 
            calculate_new_ssd(x, y, offset_x, offset_y, offset_ssd, 'l')});
        
    }

    for(heap_t::iterator it = neighbors_h[y + 1][x]->begin(); 
        it != neighbors_h[y + 1][x]->end(); ++it) {
        
        short offset_x = it->at(0);
        short offset_y = it->at(1);
        short offset_ssd = it->at(2);    
        
        neighbors_h[y][x]->push_back({offset_x, (short)(offset_y - 1), 
            calculate_new_ssd(x, y, offset_x, offset_y, offset_ssd, 'u')});
    }

}




short calculate_new_ssd(short x, short y, short offset_x, 
                        short offset_y, short offset_ssd, char direction)
{
    Buffer<short> add_b(1, 1, 1, 1, 3);
    Buffer<short> subtract_b(1, 1, 1, 1, 3);
    
    add_b.set_min(x, y, x + offset_x, y + offset_y, 0);
    subtract_b.set_min(x, y, x + offset_x, y + offset_y, 0);
    
    switch(direction) {
        
        case 'r':
            Dx_right_out.realize(add_b); 
            Dx_left_in.realize(subtract_b);
            break;

        case 'd':
            Dy_down_out.realize(add_b);
            Dy_up_in.realize(subtract_b);
            break;

        case 'l':
            Dx_left_out.realize(add_b); 
            Dx_right_in.realize(subtract_b);
            break;

        case 'u':
            Dy_up_out.realize(add_b); 
            Dy_down_in.realize(subtract_b);
            break;
 
        default:
            cerr<<"wrong direction character"<<endl;
    }
    
    return offset_ssd - subtract_b(x, y, x + offset_x, y + offset_y, 0) 
        + add_b(x, y, x + offset_x, y + offset_y, 0);
}


//void random_search() {
//
//    int M = min(log(width/3), (double)K);    
//    cout<<"M "<<M<<endl;
//    for(uint pixel_count = 0; pixel_count < neighbors_h->size(); pixel_count++) {
//        for(int i = 0; i < M; ++i) {
//            coord_t random_guess;
//            short offset_x = get_random_x() * pow(0.5, i);
//            short offset_y = get_random_y() * pow(0.5, i);
//             
//            random_guess.push_back(offset_x);
//            random_guess.push_back(offset_y);
//            Buffer<short> pix(1, 1, 1, 1, 3);
//
//
//            pix.set_min(0, 0, 0 + offset_x, 0 + offset_y, 0);   
//            D.realize(pix);
//
//            short ssd = pix(0, 0, 0 + offset_x, 0 + offset_y, 0);
//        
//
//            random_guess.push_back(ssd);
//            cout<<"i "<<i<<" x_i "<<offset_x<<" y_i "<<offset_y<<" ssd "<<ssd<<endl;
//
//            push_in_heap(neighbors_h[pixel_count], random_guess);
//
//        }
//        for(heap_t::iterator it = neighbors_h[pixel_count].begin(); 
//            it != neighbors_h[pixel_count].end(); ++it) {
//            cout<<"x_i "<<it->at(0)<<" y_i "<<it->at(1)<<" ssd "<<it->at(2)<<endl;         
//        }
//        cout<<endl;
//    }
//    
//    
//
//}
