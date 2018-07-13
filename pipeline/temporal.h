#include <vector>
extern const int K;

using namespace std;

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
vector<short> get_neighbor_ssd(short x, short y);
short get_rand_x_y();
vector<short> get_rand_coord();
void print_v_i(vector<short> v_i);
void print_coord(short* coord);
void init_neighbors();
void print_buffer(short x, short y, short x_i, short y_i, short c);
short get_rand_x();
short get_rand_y();
void fill_buffer(short x, short y, vector<vector<short>> neighbors);
void print_neighbors(short x, short y);
vector<vector<short>> sort_neighbors(vector<vector<short>> neighbors); 
void propagate_scanline();
void propagate_r_scanline();
void push_in_heap(vector<vector<short>>* heap_loc, vector<short> element); 
void print_offset_ssd(vector<short> offset_ssd);
void random_search();
