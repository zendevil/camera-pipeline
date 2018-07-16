#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <vector>
#include "Halide.h"
#include "halide_image_io.h"
//#include "temporal.h"

using namespace std;
using namespace Halide;



int width = 100;
int height = 100;
int n_channels = 3;

void print_vector(vector<vector<short>>* my_vector)
{
    for(vector<vector<short>>::iterator it = my_vector->begin(); it != my_vector->end(); ++it) {
        cout<<it->at(2)<<endl;
    }
}


void push_in_heap(vector<vector<short>>* my_vector, vector<short> element)
{
    vector<vector<short>>::iterator it = my_vector->begin();
    for(uint i = 0; i < my_vector->size() - 1; ++i) {
        
        if((it + i)->at(2) < element.at(2) && (it + i + 1)->at(2) > element.at(2)) {    
            my_vector->insert(it + i + 1, element);
            return;
        }
    }
    
    my_vector->insert(my_vector->end(), element);
     
}


int main(int argc, char** argv) 
{
    vector<vector<short>>* my_vector = new vector<vector<short>>;
    
    my_vector->push_back({0, 1, 1});
    my_vector->push_back({0, 0, 3});
    my_vector->push_back({1, 1, 7});
    my_vector->push_back({0, 2, 10});

    push_in_heap(my_vector, {2, 2, 22}); 

    print_vector(my_vector);
}

