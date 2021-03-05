#include "datastructures/Layer.h"

std::string Layer::type = std::string("abstract");
std::string Conv1D::type = std::string("conv1d");
std::string Activation::type = std::string("activation");
std::string Conv2D::type = std::string("conv2d");
std::string Dense::type = std::string("dense");
std::string Flatten::type = std::string("flatten");
std::string MaxPooling1D::type = std::string("maxpool1d");
std::string MaxPooling2D::type = std::string("maxpool2d");
std::string Dropout::type = std::string("dropout");
std::string SimpleRNN::type = std::string("simplernn");
std::string GRU::type = std::string("gru");
std::string LSTM::type = std::string("lstm");
std::string InputLayer::type = std::string("input");

Layer::Layer(std::string id)
{
	identifier = id;
}

Activation::Activation(std::string id) : Layer(id)
{
    layer_type = Activation::type;
}

Dropout::Dropout(std::string id) : Layer(id)
{
    layer_type = Dropout::type;
}

void Dropout::compute_output_shapes(){
    for (int i=0; i<this->input_shape.size(); i++) 
        this->output_shape.push_back(this->input_shape[i]);
}

void Activation::compute_output_shapes(){
    for (int i=0; i<this->input_shape.size(); i++){
        this->output_shape.push_back(this->input_shape[i]); 
    }
}


Conv1D::Conv1D(std::string id) : Layer(id)
{
	layer_type = Conv1D::type;
}

void Conv1D::compute_output_shapes(){
        this->output_shape.push_back((this->input_shape[0] - this->kernel_size[0]) / this->strides + 1); 
        this->output_shape.push_back(this->filters);
}
 

Conv2D::Conv2D(std::string id) : Layer(id)
{
	layer_type = Conv2D::type;
}

void Conv2D::compute_output_shapes(){
    this->output_shape.push_back((this->input_shape[0] - this->kernel_size[0]) / this->strides[0] + 1);
    this->output_shape.push_back((this->input_shape[1] - this->kernel_size[1]) / this->strides[1] + 1);
    this->output_shape.push_back(this->filters);
}

Dense::Dense(std::string id) : Layer(id)
{
	layer_type = Dense::type;
}

void Dense::compute_output_shapes(){
    this->output_shape.clear();
    this->output_shape.push_back(this->units);
}

Flatten::Flatten(std::string id) : Layer(id)
{
	layer_type = Flatten::type;
}

void Flatten::compute_output_shapes(){ for (int i=0; i< this->input_shape.size(); i++) this->output_shape.push_back(input_shape[i]); }

MaxPooling1D::MaxPooling1D(std::string id) : Layer(id)
{
	layer_type = MaxPooling1D::type;
}

void MaxPooling1D::compute_output_shapes(){
    this->output_shape.push_back(ceil((this->input_shape[0] - this->pool_size + 1) / this->strides + 1));
    this->output_shape.push_back(this->input_shape[1]);
               
}

MaxPooling2D::MaxPooling2D(std::string id) : Layer(id)
{
	layer_type = MaxPooling2D::type;
}

void MaxPooling2D::compute_output_shapes(){
    
    if (this->padding.compare("same") > 0){
        this->output_shape.push_back(ceil(this->input_shape[0] / this->strides[0]));
        this->output_shape.push_back(ceil(this->input_shape[1] / this->strides[1]));
        this->output_shape.push_back(this->input_shape[2]);
    } else{
        this->output_shape.push_back(floor((this->input_shape[0]  - this->pool_size[0]) / this->strides[0]) + 1);
        this->output_shape.push_back(floor((this->input_shape[1]  - this->pool_size[1]) / this->strides[1]) + 1);
        this->output_shape.push_back(this->input_shape[2]);
    }
}

SimpleRNN::SimpleRNN(std::string id) : Layer(id)
{
	layer_type = SimpleRNN::type;
    
}

void SimpleRNN::compute_output_shapes(){
    this->output_shape.clear();
    this->output_shape.push_back(this->units);
}

GRU::GRU(std::string id) : Layer(id)
{
	layer_type = GRU::type;
}

void GRU::compute_output_shapes(){}

LSTM::LSTM(std::string id) : Layer(id)
{
	layer_type = LSTM::type;
}

void LSTM::compute_output_shapes(){}

InputLayer::InputLayer(std::string id) : Layer(id)
{
	layer_type = InputLayer::type;
}

void InputLayer::compute_output_shapes(){}
