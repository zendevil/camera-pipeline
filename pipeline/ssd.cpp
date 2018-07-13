#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <vector>
#include "Halide.h"
#include "halide_image_io.h"
#include "temporal.h"

using namespace std;
using namespace Halide;



int width = 100;
int height = 100;
int n_channels = 3;



int main(int argc, char** argv) {
    vector<short> v;
    v.push_back(0);

    for(uint i = 0; i < v.size(); i++) {
        
    
        if(7 > v[i]) {
            v.insert(v.begin() + i, 7);
            break;
        }
    } 
    
    for(uint i = 0; i < v.size(); i++) {
        cout<<v[i]<<endl;    
    }
    
}

