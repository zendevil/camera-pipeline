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

extern const int K = 10;
const int n_frames = 5;

Func D, I, Dx_right_in, Dx_left_in, Dy_up_in, Dy_down_in, Dx_right_out, Dx_left_out, Dy_up_out, Dy_down_out;

const int width = 50;
const int height = 50;
int n_channels = 3;
Buffer<uint8_t> input[n_frames];

typedef vector<short> coord_t;
typedef vector<short> coord_ssd_t;
typedef vector<coord_ssd_t> heap_t;
heap_t offsets_ssd[height][width];
cout<<"capacity "<<offsets_ssd[0][0].capacity()<<endl;

Buffer<short> neighbor_b(width, height, n_channels);


Var x, y, x_i, y_i, c, i, xo, yo, xi, yi;


int main(int argc, char** argv) {

    clock_t t = clock();
    get_input();
    
    init_neighbors();
    
    propagate_scanline();
    
    // random_search();
    t = clock() - t;
    cout<<(float)t/CLOCKS_PER_SEC<<" seconds"<<endl;
}

void get_input() {

    string path;

    for(int i = 0; i < n_frames; i++) {

        path = "./frames/f_" + to_string(i + 1) + ".jpg";
        input[i] = Tools::load_image(path);   

    }

    // sig_s = width / 3;

    const int s = 10;
   
    cout<<"s="<<s<<endl;
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
    D(x, y, x_i, y_i, c) = i16(sum(pow((I(x + u.x, y + u.y, c) - I(x_i + u.x, y_i + u.y, c)), 2)));
    
    Dx_left_in(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dx_left_in.x, y + dx_left_in.y, c) - I(x_i + dx_left_in.x, y_i + dx_left_in.y, c)), 2)));
    Dx_right_in(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dx_right_in.x, y + dx_right_in.y, c) - I(x_i + dx_right_in.x, y_i + dx_right_in.y, c)), 2)));
    Dy_up_in(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dy_up_in.x, y + dy_up_in.y, c) - I(x_i + dy_up_in.x, y_i + dy_up_in.y, c)), 2)));
    Dy_down_in(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dy_down_in.x, y + dy_down_in.y, c) - I(x_i + dy_down_in.x, y_i + dy_down_in.y, c)), 2)));

    Dx_left_out(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dx_left_out.x, y + dx_left_out.y, c) - I(x_i + dx_left_out.x, y_i + dx_left_out.y, c)), 2)));
    Dx_right_out(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dx_right_out.x, y + dx_right_out.y, c) - I(x_i + dx_right_out.x, y_i + dx_right_out.y, c)), 2)));
    Dy_up_out(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dy_up_out.x, y + dy_up_out.y, c) - I(x_i + dy_up_out.x, y_i + dy_up_out.y, c)), 2)));
    Dy_down_out(x, y, x_i, y_i, c) = i16(sum(pow((I(x + dy_down_out.x, y + dy_down_out.y, c) - I(x_i + dy_down_out.x, y_i + dy_down_out.y, c)), 2)));




}



void init_neighbors() {
    srand(17);
    for(short y = 0; y < height; y++) {
        for(short x = 0; x < width; x++) {
            
            heap_t unsorted_neighbors;
            heap_t sorted_neighbors; 

            //get coordinates and ssd of each neighbor
            for(short i = 0; i < K; i++) {
                coord_ssd_t v_i_ssd = get_neighbor_ssd(x, y);
                unsorted_neighbors.push_back(v_i_ssd);
            }
            sorted_neighbors = sort_neighbors(unsorted_neighbors);
            offsets_ssd[y][x] = sorted_neighbors;
                        
         } 
    }
        
}


coord_t get_rand_coord() {
    coord_t coord;
    coord.push_back(get_rand_x());
    coord.push_back(get_rand_y());
    return coord;
}



short get_rand_x() {
    float rand_x;
    while(true) {
       
        rand_x = width / 3 * box_muller_trans((float) rand() / RAND_MAX);   
        if(rand_x < width) break;
    }
   
    return (short) rand_x;
}

short get_rand_y() {
    float rand_y;
        while(true) {
           
            rand_y = height / 3 * box_muller_trans((float) rand() / RAND_MAX);   
            if(rand_y < height) break;
        }
       
        return (short) rand_y;
}



// converts a uniform random variable into a standard normal variable
float box_muller_trans(float x) {
    return sqrt(-2 * log(x)) * cos(2 * M_PI * x);;
}


coord_ssd_t get_neighbor_ssd(short x, short y) {

    Buffer<short> pix(1, 1, 1, 1, 3);

    coord_t coord = get_rand_coord();

    pix.set_min(x, y, x + coord[0], y + coord[1]);   
    D.realize(pix);
  
    short d_i = pix(x, y, x + coord[0], y + coord[1], 0);
    coord.push_back(d_i); //pushing ssd
    
    return coord;

}

heap_t sort_neighbors(heap_t neighbors) {
    sort_heap_last_element(&neighbors);
    return neighbors;
}


void propagate_scanline() {
    for(short i = 0; i < height; ++i) {    
        for(short j = 1; j < width; ++j) {
            cout<<"y "<<i<<" x "<<j<<endl;
            
            heap_t prop_offset;
            cout<<"prop_offset.size() "<<prop_offset.size()<<endl;
            cout<<"offsets_ssd[i][j].size() "<<offsets_ssd[i][j].size()<<endl;
            for(heap_t::iterator it_curr = offsets_ssd[i][j].begin(); it_curr != offsets_ssd[i][j].end(); ++it_curr) {
                prop_offset.push_back(*it_curr); 
            }

            //int count = 0;
            for(heap_t::iterator it = offsets_ssd[i][j - 1].begin(); it != offsets_ssd[i][j - 1].end(); ++it) {
                //cout<<" y "<<i<<" x "<<j<<" ";
                //cout<<count++<<endl; 
                short x_i = it->at(0);
                short y_i = it->at(1);
                short ssd = it->at(2);    
                //cout<<"x_i "<<x_i<<" y_i "<<y_i<<" ssd "<<ssd<<endl;
                Buffer<short> patch_loc(1, 1, 1, 1, 3);
                patch_loc.set_min(j, i, j + x_i, i + y_i, 0);
                
                //calculate new ssd
                Dx_left_in.realize(patch_loc);
                short left = patch_loc(j, i, j + x_i, i + y_i, 0);  
                Dx_right_out.realize(patch_loc); 
                short right = patch_loc(j, i, j + x_i, i + y_i, 0);  
                short new_ssd = ssd - left + right;
                 
                prop_offset.push_back({(short)(x_i + 1), y_i, new_ssd});
                
                Dy_up_in.realize(patch_loc);
                short up = patch_loc(j, i, j + x_i, i + y_i, 0);
                Dy_down_out.realize(patch_loc);
                short down = patch_loc(j, i, j + x_i, i + y_i, 0);
                new_ssd = ssd - up + down;
                
                prop_offset.push_back({x_i, (short)(y_i + 1), new_ssd});

            }
        
            //prop_offset = sort_neighbors(prop_offset);
            //offsets_ssd[i][j] = prop_offset;
        
        }
    }
}

//void propagate_r_scanline() {
//    for(uint i = 0; i < offsets_ssd.size(); ++i) {
//        heap_t prop_offset;
//        for(heap_t::iterator it = offsets_ssd[i].begin(); it != offsets_ssd[i].end(); ++it) {
//            prop_offset.push_back(*it); 
//            short x_i = it->at(0);
//            short y_i = it->at(1);
//            short ssd = it->at(2);    
//            Buffer<short> patch_loc(1, 1, 1, 1, 3);
//            patch_loc.set_min(0, 0, x_i, y_i, 0);
//            
//            //calculate new ssd
//            Dx_left_out.realize(patch_loc);
//            short left = patch_loc(0, 0, x_i, y_i, 0);  
//            Dx_right_in.realize(patch_loc); 
//            short right = patch_loc(0, 0, x_i, y_i, 0);  
//            short new_ssd = ssd + left - right;
//             
//            prop_offset.push_back({(short)(x_i - 1), y_i, new_ssd});
//            //push_in_heap(&offsets_ssd[0], {(short)(x_i + 1), y_i, new_ssd});
//            
//            Dy_up_out.realize(patch_loc);
//            short up = patch_loc(0, 0, x_i, y_i, 0);
//            Dy_down_in.realize(patch_loc);
//            short down = patch_loc(0, 0, x_i, y_i, 0);
//            new_ssd = ssd + up - down;
//            
//            prop_offset.push_back({x_i, (short)(y_i - 1), new_ssd});
//            //push_in_heap(&offsets_ssd[0], {x_i,(short)(y_i + 1), new_ssd});
//
//        }
//    
//        prop_offset = sort_neighbors(prop_offset);
//        offsets_ssd[i] = prop_offset;
//    
//    }
//
//}

//void push_in_heap(vector<vector<short>>* heap_loc, vector<short> element) {
//    cout<<"new element ";
//    print_offset_ssd(element); 
//    for(vector<vector<short>>::iterator it = heap_loc->begin(); it != heap_loc->end(); ++it) {    
//        //print_offset_ssd(*it);
//     
//        if(element.at(2) > it->at(2)) {
//            cout<<"element ";
//            print_offset_ssd(element);
//            cout<<"it ";
//            print_offset_ssd(*it);
//            //heap_loc->insert(it, element);
//        }
//    }
//    //for(uint i = 0; i < heap_loc->size(); i++) {
//    //    if(element.at(2) > heap_loc->at(i).at(2)) {
//    //        heap_loc->insert(heap_loc->begin() + i + 1, element);    
//
//
//    //    }
//    //}
//}


void print_offset_ssd(coord_ssd_t offset_ssd) {
    cout<<"x_i "<<offset_ssd[0]<<" y_i "<<offset_ssd[1]<<" ssd "<<offset_ssd[2]<<endl;

}

//void random_search() {
//
//    int M = min(log(width/3), (double)K);    
//    cout<<"M "<<M<<endl;
//    for(uint pixel_count = 0; pixel_count < offsets_ssd.size(); pixel_count++) {
//        for(int i = 0; i < M; ++i) {
//            coord_t random_guess;
//            short offset_x = get_rand_x() * pow(0.5, i);
//            short offset_y = get_rand_y() * pow(0.5, i);
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
//            offsets_ssd[pixel_count].push_back(random_guess);
//            offsets_ssd[pixel_count] = sort_neighbors(offsets_ssd[pixel_count]);
//
//        }
//        for(heap_t::iterator it = offsets_ssd[pixel_count].begin(); it != offsets_ssd[pixel_count].end(); ++it) {
//            cout<<"x_i "<<it->at(0)<<" y_i "<<it->at(1)<<" ssd "<<it->at(2)<<endl;         
//        }
//        cout<<endl;
//    }
//    
//    
//
//}
