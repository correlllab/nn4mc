/**
 * \file HDF5Parser.cpp
 * Code to parse hdf5 file containing neural network information
 *
 * \brief Parser for hdf5 neural network file:
 * This code is meant to declare a neural network (NeuralNetwork.h) 
 * based on the hdf5 file that was output by the training code
 *
 * \author: $Author: Sarah Aguasvivas Manzano $
 *
 * \version $Version: 0.1 $
 *
 * \date $Date: 2019/02/18 $
 *
 * Contact: saag5228@colorado.edu
 *
 * Created on: Thu Feb 07 2019
 * 
 * */
#include "parser/h5_parser.h"

extern "C" herr_t weights_callback(hid_t loc_id, const char *name, const H5L_info_t * linfo, void *opdata);
extern "C" herr_t network_callback(hid_t loc_id, const char *name, const H5L_info_t * linfo, void *opdata);

void HDF5Parser::construct_builder_map(){
    this->BuilderMap["Activation"] = new ActivationFactory();
    this->BuilderMap["Conv1D"]= new Conv1DFactory();
    this->BuilderMap["Conv2D"]= new Conv2DFactory();
    this->BuilderMap["Flatten"]= new FlattenFactory();
    this->BuilderMap["Dense"]= new DenseFactory();
    this->BuilderMap["MaxPooling1D"]= new MaxPooling1DFactory();
    this->BuilderMap["MaxPooling2D"]= new MaxPooling2DFactory();
    this->BuilderMap["Dropout"] = new DropoutFactory();
    this->BuilderMap["LSTM"]= new LSTMFactory();
    this->BuilderMap["GRU"]= new GRUFactory();
    this->BuilderMap["SimpleRNN"]= new SimpleRNNFactory();
}

void HDF5Parser::parse_weights(){
      const H5std_string FILE_NAME(this->file_name);
      //Exception::dontPrint();
      H5File file = H5File(FILE_NAME, H5F_ACC_RDONLY);
      Group group = Group(file.openGroup("model_weights"));
      struct opdataWeights od_weights;
      od_weights.LM = this->layerMap;
      H5Lvisit(group.getId(),H5_INDEX_NAME,H5_ITER_INC,weights_callback,(void*)&od_weights);
      this->layerMap= od_weights.LM;
      std::cout << "PARSER: Weights and biases parsed" << std::endl;
}

NeuralNetwork* HDF5Parser::get_neural_network(){
    NeuralNetwork* NN = new NeuralNetwork();
    Layer* l = new InputLayer("input_1");
    
    NN->addLayer(l);
    NN->addLayer(this->layerMap[this->layer_edges.begin()->first]);
    NN->addEdge(l, this->layerMap[this->layer_edges.begin()->first]);
    l = this->layerMap[this->layer_edges.begin()->first];
      
    for (std::vector<std::pair<std::string, std::string>>::iterator it= this->layer_edges.begin(); it!=this->layer_edges.end(); ++it){
        NN->addLayer(this->layerMap[it->second]);
        NN->addEdge(l, this->layerMap[it->second]);
        l = this->layerMap[it->second];
    }
    return NN;
}

void HDF5Parser::build_layer_shapes(){

    if (nn_input_shape.size()>0 && this->layerMap.begin()->second->input_shape.size() == 0){         
        this->layerMap[this->layer_ids[0]]->input_shape = nn_input_shape;
    }
    
    this->layerMap[this->layer_ids[0]]->compute_output_shapes();
    
    for (std::vector<std::pair<std::string, std::string>>::iterator it= this->layer_edges.begin(); it!=this->layer_edges.end(); ++it){

        int rank = this->layerMap[it->first]->output_shape.size();
        
        int actual_output = 1;
        for (int i=0; i < rank; i++) {
            actual_output *= this->layerMap[it->first]->output_shape[i];
        }            
        
        this->layerMap[it->second]->input_shape.push_back(actual_output);
        this->layerMap[it->second]->compute_output_shapes();
    }
}

void HDF5Parser::call_layer_builders(){
        int i = 0;
        int model_build_size = this->model_config["config"]["build_input_shape"].size();
        int model_build_size1 = this->model_config["config"]["layers"][0]["config"]["batch_input_shape"].size();

        
        if (model_build_size>0){
            for (int i=0; i<model_build_size-1; i++){
                nn_input_shape.push_back(this->model_config["config"]["build_input_shape"][i+1]);
            }  
        }

        if (model_build_size1 > 0){
            nn_input_shape.clear();
            for (int i=0; i<model_build_size1 -1; i++){
                nn_input_shape.push_back(this->model_config["config"]["layers"][0]["config"]["batch_input_shape"][i+1]);
            }
        }
        
        for (auto it: this->model_config["config"]["layers"].items()){
            this->layer_ids.push_back(it.value()["config"]["name"].get<std::string>());
            this->layerBuilderVector.push_back(this->BuilderMap[it.value()["class_name"].get<std::string>()]);

            this->layerBuilderVector[i]->create(it.value()["config"]["name"])->create_from_json(it.value(), it.value()["config"]["name"], this->layerMap); 
             
            i++;
        }
}

void HDF5Parser::build_edges(){
    for (int i=0; i< this->layerBuilderVector.size()-1; ++i){
       this->layer_edges.push_back(std::make_pair(this->layer_ids[i], this->layer_ids[i+1])); 
    }
}

json HDF5Parser::parse_model_config(){
      H5File filefile = H5File( this->file_name, H5F_ACC_RDONLY );
      Group what = Group(filefile.openGroup("/"));
      Attribute attr= Attribute(what.openAttribute("model_config"));
      DataType type = DataType (attr.getDataType());
    
        std::string test;
        attr.read(type, test);
        
        // define parser callback
        json::parser_callback_t cb = [](int depth, json::parse_event_t event, json & parsed)
        {
            // skip object elements with key "Thumbnail"
            if (event == json::parse_event_t::key and parsed == json("Thumbnail"))
            {
                return false;
            }
            else
            {
                return true;
            }
        };
      
      std::stringstream ss;
      ss << test;
      
      json j_filtered = json::parse(ss, cb);
      
      return j_filtered;
}

// PARSE FUNCTION
int HDF5Parser::parse()
{
      this->construct_builder_map();
    
      // Parse Model Config: 
      this->model_config= this->parse_model_config();
      std::cout << "-------------------------------------------------" << std::endl;
     
      // Assign Config Builders:
      this->call_layer_builders(); 
      
      // Parse Weights:
      std::cout << "-------------------------------------------------" << std::endl;

      // Populate the layer types:
      this->build_edges();
      this->build_layer_shapes(); 
      this->parse_weights();
      
      std::cout<< "PARSER: Parsing complete!" <<std::endl;
   return 0;
}

herr_t 
weights_callback(hid_t loc_id, const char *name, const H5L_info_t * linfo, void *opdata)
{
    H5O_info_t infobuf;
    struct opdataWeights *od = (struct opdataWeights *) opdata;
    hid_t status = H5Oget_info_by_name(loc_id, name, &infobuf, H5P_DEFAULT);

    if (infobuf.type == H5O_TYPE_DATASET){
        std::string s(name);
        std::string delimiter= "/";
        std::string layer_id;

        // Parsing matching layer id:
        size_t pos=0;
        while ((pos=s.find(delimiter)) != std::string::npos){
            layer_id= s.substr(0, pos);
            s.erase(0, pos+delimiter.length());
        }
        hid_t datatype, dataspace, rank; 
                hid_t dset = H5Dopen(loc_id, name, H5P_DEFAULT);
                datatype= H5Dget_type(dset);
                dataspace = H5Dget_space(dset);
                rank= H5Sget_simple_extent_ndims(dataspace); 
                hsize_t dims[rank];
                H5Sget_simple_extent_dims(dataspace, dims, NULL);
                std::vector<unsigned int> tensor_dims;

                //Parsing dimensions for Tensor
                for (int i=0; i<rank; i++) {
                    tensor_dims.push_back((unsigned int)dims[i]);
                }

                Weight * wb = new Weight(layer_id, tensor_dims);
                Tensor<double>* T = wb->get_weight_tensor(); //assuming float

                float *rbuf;
                herr_t ret;
              

		// TODO: Make equalilty operator to clean up the following: 
                int flat=1;
                for (int i=0; i<rank; i++){
                    flat*= dims[i];
                }

                rbuf= new float [flat];
                ret = H5Dread(dset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuf); 
                
		switch(rank){
                    case 1:
                        {
                            for (int i=0; i<dims[0]; i++){
                                (*T)(i) = (double)rbuf[i];
                            }

                        break;
                        }
                    case 2:
                        {
                            for (int i=0; i<tensor_dims[0]; i++){
                                for (int j=0; j<tensor_dims[1]; j++){
                                    int idx= tensor_dims[1]*i+j;
                                    (*T)(i, j) = (double)rbuf[idx];
                                }
                            }
                            break;
                        }
                    case 3:
                        {
                            for (int i=0; i< tensor_dims[0]; i++){
                                for (int j=0; j<tensor_dims[1]; j++){
                                    for (int k=0; k<tensor_dims[2]; k++){
                                        int idx= tensor_dims[2]*tensor_dims[1]*i + tensor_dims[2]*j + k;
                                        (*T)(i, j, k)= (double)rbuf[idx];
                                    }
                                }
                            }
                            
                            break;
                        }

                     case 4:
                        {
                            for (int i=0; i< tensor_dims[0]; i++){
                                for (int j=0; j<tensor_dims[1]; j++){
                                    for (int k=0; k<tensor_dims[2]; k++){
                                        for (int l=0; l<tensor_dims[3]; l++){
                                        int idx= tensor_dims[2]*tensor_dims[1]*tensor_dims[3]*i + tensor_dims[2]*tensor_dims[3]*j + k*tensor_dims[3] + l;
                                        (*T)(i, j, k, l)= (double)rbuf[idx];
                                        }
                                    }
                                }
                            }

                            break;
                        }



                    default:
                        break;
                }
                
                delete []rbuf; 

                // Create weights:
                if (s.compare("bias:0") == 0 ){ // it's a bias 
                    wb->identifier = wb->identifier.append("_b");
                    od->LM[layer_id]->b = wb;

                } if (s.compare("kernel:0") == 0){ // it's a weight
                    wb->identifier=wb->identifier.append("_W");
                    od->LM[layer_id]->w = wb;
                }
                if (s.compare("recurrent_kernel:0") == 0){ // it's a rec weight
                    wb->identifier = wb->identifier.append("_Wrec");
                    od->LM[layer_id]->w_rec = wb;
                }    

                ret= H5Dclose(dset); 
        }
    
    return 0;
 }
