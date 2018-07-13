#include <vector>
#include "Halide.h"
extern const int K;

using namespace std;
using namespace Halide;
void get_input();
float box_muller_trans(float x);
int i_parent(int i);
int i_left_child(int i);
int i_right_child(int i);
void swap_element(vector<vector<short>>* a, int b, int c);
void sift_down(vector<vector<short>> *a, int start, int end);
void heapify(vector<vector<short>>* a);
void sort_heap_last_element(vector<vector<short>>* a);
void print_heap(vector<vector<short>> heap, int x, int y);
void set_v_i(ushort x, ushort y, vector<short>* c);
void clear_v_i(vector<short>* c);
short get_rand_x_y();
short* get_rand_coord(short* coord);
void print_v_i(vector<short> v_i);
void print_coord(short* coord);
void init_neighbors(Buffer<short> buffer);
void print_buffer(Buffer<short> buffer, short x, short y, short x_i, short y_i, short c);
void clear_neighbors(vector<vector<short>>* neighbors);
short get_rand_x();
short get_rand_y();
void fill_buffer(Buffer<short> neighbor_b, short x, short y, vector<vector<short>> neighbors);
