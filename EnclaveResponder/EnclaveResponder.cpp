
// Enclave2.cpp : Defines the exported functions for the DLL application
#include "sgx_eid.h"
#include "EnclaveResponder_t.h"
#include "EnclaveMessageExchange.h"
#include "error_codes.h"
#include <cstdio>
#include "Utility_E2.h"
#include "sgx_dh.h"
#include "sgx_utils.h"
#include <map>
#define UNUSED(val) (void)(val)
uint8_t * find_labels_for_worker(worker_input_t worker_input);
float ** get_centroids(worker_input_t worker_input);
float ** get_dataset(worker_input_t worker_input);
void set_centroids(worker_output_t * worker_output,float ** centroids);
void set_dataset(worker_input_t * worker_input,float ** dataset);
uint32_t get_new_centroids(float ** current_centroids, uint8_t * labels, 
                            uint8_t number_of_clusters, uint32_t number_of_rows,
                            uint32_t number_of_columns, float ** dataset,
                            float ** new_centroids);
float * divide_vector(float * vector1, uint32_t divisor, uint32_t vector_length);
uint32_t sum_vectors(float * vector1, float * vector2, uint32_t vector_length);
uint32_t average_vectors(float * vector1, float * vector2, uint32_t vector_length,float * average);
uint32_t * membership_counter(uint8_t n_clusters, uint32_t n_rows, uint8_t * labels);
uint32_t membership_matrix(uint32_t n_rows, uint8_t * labels, uint8_t ** membership);
void serialize_2d_array(uint8_t **array_2d, uint8_t *array_1d, int rows, int cols);
void dispose_matrix(float ** matrix,uint32_t n_rows);
uint32_t calculate_membership_sum(uint8_t * labels, uint32_t n_rows, uint8_t n_cols, float ** dataset, float ** sum);
float * product_vector(float * vector1, uint32_t factor, uint32_t vector_length);
std::map<sgx_enclave_id_t, dh_session_t>g_src_session_info_map;

// this is expected initiator's MRSIGNER for demonstration purpose
//Please replace with your project responder enclave's MRSIGNER in your project!!!
//The command to get your signed enclave's MRSIGNER: <SGX_SDK Installation Path>/bin/x64/sgx_sign dump -enclave <Signed Enclave> -dumpfile mrsigner.txt
//Find the signed enclave's MRSIGNER in the mrsigner.txt<mrsigner->value:>, then replace blow value
sgx_measurement_t g_initiator_mrsigner = {
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff    
    }
};

/* Function Description:
 *   this is to verify peer enclave's identity
 * For demonstration purpose, we verify below points:
 *   1. peer enclave's MRSIGNER is as expected
 *   2. peer enclave's PROD_ID is as expected
 *   3. peer enclave's attribute is reasonable that it should be INITIALIZED and without DEBUG attribute (except the project is built with DEBUG option)
 * */
extern "C" uint32_t verify_peer_enclave_trust(sgx_dh_session_enclave_identity_t* peer_enclave_identity)
{
    if(!peer_enclave_identity)
        return INVALID_PARAMETER_ERROR;

    // check peer enclave's MRSIGNER
    // Please enable blow check in your project!!!
    /*
    if (memcmp((uint8_t *)&peer_enclave_identity->mr_signer, (uint8_t*)&g_initiator_mrsigner, sizeof(sgx_measurement_t)))
        return ENCLAVE_TRUST_ERROR;
    */
    if(peer_enclave_identity->isv_prod_id != 0 || !(peer_enclave_identity->attributes.flags & SGX_FLAGS_INITTED))
        return ENCLAVE_TRUST_ERROR;

    // check the enclave isn't loaded in enclave debug mode, except that the project is built for debug purpose
#if defined(NDEBUG)
    if (peer_enclave_identity->attributes.flags & SGX_FLAGS_DEBUG)
        return ENCLAVE_TRUST_ERROR;
#endif

    return SUCCESS;
}

/* Function Description: Operates on the input secret and generates the output secret */
worker_output_t get_message_exchange_response(worker_input_t worker_input)
{
    uint32_t secret_response;
    uint8_t * labels = find_labels_for_worker(worker_input);

    worker_output_t output;
    output.n_rows=worker_input.n_rows;
    output.n_clusters = worker_input.n_clusters;
    output.n_cols = worker_input.n_cols;
    memcpy(output.labels, labels, sizeof(output.labels));

    float ** centroids_data = get_centroids(worker_input);
    
    float ** dataset = get_dataset(worker_input);

float ** new_centroids =  (float **) calloc(worker_input.n_clusters,sizeof(float*)); //new float *[number_of_clusters];
for (size_t i = 0; i < worker_input.n_clusters; i++)
{
    new_centroids[i]=(float *)calloc(worker_input.n_cols,sizeof(float));
}

     uint32_t ret_new_centroids = get_new_centroids(centroids_data,labels,
                                            worker_input.n_clusters,worker_input.n_rows,
                                            worker_input.n_cols,dataset,
                                            new_centroids); 
    set_centroids(&output,new_centroids); 
    //set_centroids(&output,centroids_data);

    dispose_matrix(centroids_data,worker_input.n_clusters);
    dispose_matrix(dataset,worker_input.n_rows);
    dispose_matrix(new_centroids,worker_input.n_clusters);
    //free(&worker_input);
    SAFE_FREE(labels);
return output;

}
void serialize_uint16_t_2d_array(uint16_t** arr, size_t rows, size_t cols, uint8_t* buffer) {
    size_t idx = 0; // Index for buffer
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            buffer[idx++] = (uint8_t)(arr[i][j] >> 8);     // Extract the most significant byte
            buffer[idx++] = (uint8_t)(arr[i][j] & 0xFF); // Extract the least significant byte
        }
    }
}

void serialize_float_array(float **float_array, int rows, int cols, uint8_t *buffer) {
    int num_floats = rows * cols;
    int buffer_size = num_floats * sizeof(float);
    // serialize the float array into the buffer
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float current_float = float_array[i][j];
            uint8_t *float_ptr = (uint8_t *) &current_float;
            int buffer_index = (i * cols + j) * sizeof(float);
            memcpy(buffer + buffer_index, float_ptr, sizeof(float));
        }
    }
}

void serialize_2d_array(uint8_t **array_2d, uint8_t *array_1d, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            array_1d[i * cols + j] = array_2d[i][j];
        }
    }
}

uint8_t * find_labels_for_worker(worker_input_t worker_input)
{ 
    uint8_t * labels = (uint8_t *) calloc(worker_input.n_rows,sizeof(uint8_t)); //new uint8_t[worker_input.n_rows];
    
    float ** centroids = get_centroids(worker_input);
    float ** dataset = get_dataset(worker_input);
    uint32_t dataset_rows = worker_input.n_rows;
    uint8_t dataset_cols = worker_input.n_cols;
    uint8_t number_of_clusters = worker_input.n_clusters;
    
    for(int i=0; i<dataset_rows; i++)
    {
        float global_dist=999990;
        for(int j=0;j<number_of_clusters;j++)
        {
            float distance = computeEuclideanDistance(centroids[j],dataset[i],dataset_cols);
            if(distance < global_dist){
                global_dist = distance;
                labels[i]=j;
            }
        }
    }
    dispose_matrix(centroids,worker_input.n_clusters); 
    dispose_matrix(dataset,worker_input.n_rows); 
    return labels;

}
void dispose_matrix(float ** matrix,uint32_t n_rows){
    for (size_t i = 0; i < n_rows; i++)
    {
        SAFE_FREE(matrix[i]);
    }
    SAFE_FREE(matrix);
}
uint32_t membership_matrix(uint32_t n_rows, uint8_t * labels, uint8_t ** membership)
{
    for(int i=0;i<n_rows;i++){
        membership[i][labels[i]]=1;
    }
    return 0;
} 
uint32_t * membership_counter(uint8_t n_clusters, uint32_t n_rows, uint8_t * labels){
    uint32_t * membership_count = (uint32_t *) calloc(n_clusters, sizeof(uint32_t)); //new uint32_t[n_clusters]; // (uint32_t *) malloc(n_clusters* sizeof(uint32_t));
    for(int i=0;i<n_rows;i++){
        membership_count[labels[i]]+=1;
    }
    return membership_count;
}
uint32_t average_vectors(float * vector1, float * vector2, uint32_t vector_length,float * average){
    for(int i=0;i<vector_length;i++){
        average[i]=(float)((vector1[i]+vector2[i])/2);
    }
    return 0;
}

uint32_t sum_vectors(float * vector1, float * vector2, uint32_t vector_length){
    //float * result = (float *) calloc(vector_length , sizeof(float)); //new float[vector_length];//(float *) malloc(vector_length * sizeof(float));
    for(int i=0;i<vector_length;i++){
        vector2[i]+=(float)(vector1[i]);
    }
    return 0;
}

float * divide_vector(float * vector1, uint32_t divisor, uint32_t vector_length){
    float * result = (float *) calloc(vector_length , sizeof(float)); //new float [vector_length];// malloc(vector_length * sizeof(float));
    for(int i=0;i<vector_length;i++){
        result[i]=(float)(vector1[i]/divisor);
    }
    return result;
}
float * product_vector(float * vector1, uint32_t factor, uint32_t vector_length){
    float * result = (float *) calloc(vector_length , sizeof(float)); //new float [vector_length];// malloc(vector_length * sizeof(float));
    for(int i=0;i<vector_length;i++){
        result[i]=(float)(vector1[i]*factor);
    }
    return result;
}

/* Function Description: Generates the response from the request message
 * Parameter Description:
 * [input] decrtyped_data: pointer to decrypted data
 * [output] resp_buffer: pointer to response message, which is allocated in this function
 * [output] resp_length: this is response length */
extern "C" uint32_t message_exchange_response_generator(char* decrypted_data,
                                              char** resp_buffer,
                                               size_t* resp_length)
{

 worker_output_t output; 

    ms_in_msg_exchange_t *ms;
    worker_input_t  inp_secret_data;

    if(!decrypted_data || !resp_length){
        return INVALID_PARAMETER_ERROR;
    }
    ms = (ms_in_msg_exchange_t *)decrypted_data;
    if(umarshal_message_exchange_request(&inp_secret_data,ms) != SUCCESS){
            return ATTESTATION_ERROR;  
    }
    output = get_message_exchange_response(inp_secret_data);
    if(marshal_message_exchange_response(resp_buffer, resp_length, output) != SUCCESS)
        return MALLOC_ERROR;

    return SUCCESS;
}

void deserialize_float_array(uint8_t *buffer, int rows, int cols, float **float_array) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int buffer_index = (i * cols + j) * sizeof(float);
            uint8_t *float_ptr = buffer + buffer_index;
            memcpy(&float_array[i][j], float_ptr, sizeof(float));
        }
    }
}
float ** get_centroids(worker_input_t worker_input){
    float ** centroids_data = new float * [worker_input.n_clusters]; //(uint16_t**)malloc(3 * sizeof(uint16_t*)); //create_centroids(rows,cols);
    for(int i=0;i<worker_input.n_clusters;i++) centroids_data[i] = new float [worker_input.n_cols]; //(uint16_t*)malloc(4 * sizeof(uint16_t));
    deserialize_float_array(worker_input.serialized_centroids,worker_input.n_clusters,worker_input.n_cols,centroids_data);
    return centroids_data;
}

float ** get_dataset(worker_input_t worker_input){
    float ** dataset_data = new float * [worker_input.n_rows]; 
    for(int i=0;i<worker_input.n_rows;i++) dataset_data[i] = new float [worker_input.n_cols]; 
    deserialize_float_array(worker_input.serialized_dataset,worker_input.n_rows,worker_input.n_cols,dataset_data);
    return dataset_data;
}

void set_centroids(worker_output_t * worker_output,float ** centroids){
        serialize_float_array(centroids,worker_output->n_clusters,worker_output->n_cols,worker_output->serialized_centroids);
}
void set_dataset(worker_input_t * worker_input,float ** dataset){
        serialize_float_array(dataset,worker_input->n_rows,worker_input->n_cols,worker_input->serialized_dataset);
}

uint32_t calculate_membership_sum(uint8_t * labels, uint32_t n_rows, uint8_t n_cols, float ** dataset, float ** sum)
{
    for (uint32_t i = 0; i < n_rows; i++)
    {
      sum_vectors(dataset[i],sum[labels[i]],n_cols);  
    }
    return 0;
}

uint32_t get_new_centroids(float ** current_centroids, uint8_t * labels, 
                            uint8_t number_of_clusters, uint32_t number_of_rows,
                            uint32_t number_of_columns, float ** dataset,
                            float ** new_centroids){
    uint32_t * mem_counter = membership_counter(number_of_clusters,number_of_rows,labels);
/*     float ** membership_sum = (float **)calloc(number_of_clusters,sizeof(float()));
    for (size_t i = 0; i < number_of_clusters; i++)
    {
        membership_sum[i]=(float *)calloc(number_of_columns,sizeof(float));
    }
    calculate_membership_sum(labels,number_of_rows,number_of_columns,dataset,membership_sum); */
    
    uint8_t ** bin_mem_matrix = (uint8_t**)calloc(number_of_rows,sizeof(uint8_t*));
    for (uint32_t i = 0; i < number_of_rows; i++)
    {
        bin_mem_matrix[i]=(uint8_t*)calloc(number_of_clusters,sizeof(uint8_t));
    }
    
    membership_matrix(number_of_rows,labels,bin_mem_matrix);

    for (int i = 0; i < number_of_clusters; i++)
    {
        float * sum = (float *)calloc(number_of_columns,sizeof(float)); //new float [number_of_columns];
        if (mem_counter[i] > 0)
        {
            for (int j = 0; j < number_of_rows; j++)
            {
                if(bin_mem_matrix[j][i] == 1)
                { //indicates row i belong to cluster j
                    float * average=(float*)calloc(number_of_columns,sizeof(float));
                    average_vectors(dataset[j],current_centroids[i],number_of_columns,average);
                    sum_vectors(average, sum, number_of_columns);
                    SAFE_FREE(average);
                }
            }
            new_centroids[i] = divide_vector(sum, mem_counter[i]+10,number_of_columns);
        }   
        else
        {
            memcpy(new_centroids[i],current_centroids[i],number_of_columns * sizeof(float));
        }
        SAFE_FREE(sum);
    }

    SAFE_FREE(mem_counter);

    for (size_t i = 0; i < number_of_rows; i++)
    {
        SAFE_FREE(bin_mem_matrix[i]);
    }
    SAFE_FREE(bin_mem_matrix);
    return 0;//new_centroids;
}