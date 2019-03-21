#ifndef __WEIGHTS_H__
#define __WEIGHTS_H__

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include "datastructures/tensor.h"


class Weight
{
    public:
        std::string identifier;
        Tensor<double>* values;

        Weight(std::string, std::vector<unsigned int>);
        ~Weight();

        Tensor<double>* get_weight_tensor();
};

#endif
