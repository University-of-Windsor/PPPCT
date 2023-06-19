#include <stdio.h>
#include <map>
#include <sched.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "sgx_eid.h"
#include "sgx_urts.h"
#include "FileIO.h"
#include "float.h"
#include "math.h"
#include <iostream>
#include "EnclaveInitiator_u.h"

//void initialize_worker_input(worker_input_t *input);
void initialize_worker_input(worker_input_t *worker_input, uint32_t rows, uint32_t cols, uint32_t num_workers, uint32_t num_clusters);
void serialize_float_array(float **float_array, int rows, int cols, uint8_t *buffer);
void deserialize_float_array(uint8_t *buffer, int rows, int cols, float **float_array);
float ** initialize_centroids(float ** dataset, uint32_t n_rows, uint32_t n_cols, uint8_t n_clusters);
float * divide_vector(float * vector1, uint32_t divisor, uint32_t vector_length);
float * sum_vectors(float * vector1, float * vector2, uint32_t vector_length);
float ** get_centroids(worker_output_t worker_output);
float ** get_dataset(worker_output_t worker_output);
float ** compute_global_centroids(worker_output_t * worker_output, uint8_t n_workers, uint8_t n_clusters,uint8_t n_cols);
void setup_input(worker_input_t *worker_input, uint32_t rows, uint32_t cols,  uint32_t n_clusters, float ** dataset, float ** new_centroids, uint8_t * labels, uint8_t worker_id);
void print_centroids(float ** centroids,uint8_t n_rows,uint8_t n_cols);
float ** get_centroids_input(worker_input_t worker_output);
void dispose_matrix(float ** matrix,uint32_t n_rows);
float compute_delta(float** old_centroids,float ** new_centroids, uint8_t n_rows, uint8_t n_cols);

#define MAX_PATH FILENAME_MAX
#define MAX_ITER 3000
#define THRESHOLD 1e-20



#define ENCLAVE_INITIATOR_NAME "libenclave_initiator.signed.so"

int main(int argc, char* argv[])
{
    int rows_global=0;
    int cols_global=0;
    int update = 0;
    uint32_t ret_status;
    sgx_status_t status;
    sgx_launch_token_t token = {0};
    sgx_enclave_id_t initiator_enclave_id = 0;

    (void)argc;
    (void)argv;


     int rows = 1000;
     int cols = 1200;
     rows_global=cols;
     cols_global=rows;			// Number of data points (input)
    int K = 1;                          //Number of clusters to be formed (input)
    int num_threads=4;
    char input[5];
    int* data_points = &rows;		//Data points (input)
    int* cluster_points = &rows;	//clustered data points (to be computed)
    double*** iter_centroids;              //centroids of each iteration (to be computed)
    int number_of_iterations = 300;      //no of iterations performed by algo (to be computed)
    //---------------------------------------------------------------------
    int x = 1;
    //double start_time = 0, end_time = 0;
    double computation_time = 0;
    //start_time = clock();
    struct timeval start_time, end_time;
    double cpu_time_used;
 gettimeofday(&start_time, NULL);
    //sleep(5);
    float** data_points2;
    int** data_labels;
    char * file=new char [100];
    char * dataset=new char[100];
    char * numberOfClusters=new char [2];
    char * numberOfCells=new char [10];
    char * numberOfGenes=new char [10];

    if(argv[1]!=""){
    	dataset=argv[1];
    }
    if(argv[2]!=""){
    	numberOfClusters=argv[2];
    }
    if(argv[3]!=""){
    	numberOfCells=argv[3];
    }
    if(argv[4]!=""){
    	numberOfGenes=argv[4];
    }
    K = atoi(numberOfClusters);
    rows_global=atoi(numberOfCells);
    cols_global = atoi(numberOfGenes);

/*     printf("\nEnter number of threads to be used: ");
    scanf("%d", &num_threads); */
    //omp_get_wtime();
    FileIO processFile=FileIO();
    std::string python_command {"python3.10 ../preprocessing.py "};
    python_command+=dataset;
    python_command+=" ";
    python_command+=numberOfClusters;
    system(const_cast<char*>(python_command.c_str()));

    std::cout << "python_command: " << python_command;
    data_points2=processFile.readCsvFile("W.csv",rows_global,K+1);

    //end_time = omp_get_wtime();
    //double read_time=end_time-start_time;
    //start_time = omp_get_wtime();

    //std::cout << "\nTime Taken: " << computation_time+read_time  << "\n";


    ////////////////////////////////////////////////////////////////////////////////////

int max_iter=300;
uint32_t num_workers = 1;
worker_input_t worker_input;
uint32_t rows2 = 6000;
uint32_t cols2 = 4;
cols_global = 4;
uint32_t num_clusters = 3;
//start_time = clock();


    // create ECDH initiator enclave
    status = sgx_create_enclave(ENCLAVE_INITIATOR_NAME, SGX_DEBUG_FLAG, &token, &update, &initiator_enclave_id, NULL);
    if (status != SGX_SUCCESS) {
        printf("failed to load enclave %s, error code is 0x%x.\n", ENCLAVE_INITIATOR_NAME, status);
        return -1;
    }
    printf("succeed to load enclave %s\n", ENCLAVE_INITIATOR_NAME);

    // create ECDH session using initiator enclave, it would create ECDH session with responder enclave running in another process
    status = test_create_session(initiator_enclave_id, &ret_status);
    if (status != SGX_SUCCESS || ret_status != 0) {
        printf("failed to establish secure channel: ECALL return 0x%x, error code is 0x%x.\n", status, ret_status);
        sgx_destroy_enclave(initiator_enclave_id);
        return -1;
    }
    printf("succeed to establish secure channel.\n");




printf("n_rows: %d\n",rows_global);
printf("n_cols: %d\n",cols_global);
printf("n_clusters: %d\n",K);
float ** old_centroids=(float**)calloc(K,sizeof(float*));
for(int i=0;i<K;i++) 
{
    old_centroids[i] = (float*)calloc(cols_global,sizeof(float));
}

float ** global_centroids = initialize_centroids(data_points2,rows_global,cols_global,K);

uint8_t ** init_labels = (uint8_t **) calloc(num_workers,sizeof(uint8_t*));
for (int i = 0; i < num_workers; i++)
{
    init_labels[i] = (uint8_t *) calloc(rows_global,sizeof(uint8_t));
}
/*
    Initialize all labels to random values.
*/
for (int i = 0; i < num_workers; i++)
{
    printf("labels: ");
    for (int j = 0; j < rows_global; j++)
    {
        init_labels[i][j]=1;//(j^i)%num_clusters;
         printf(" %d ",init_labels[i][j]);
    }
}

worker_output_t worker_output[num_workers];
float delta=1;
for (size_t i = 0; i < max_iter ; i++)
{
    memcpy(old_centroids,global_centroids,K*sizeof(float*));
    for (uint32_t j = 0; j < K; j++)
    {
        memcpy(old_centroids[j],global_centroids[j],cols_global*sizeof(float));
    }
    
    printf("\n\niteration number: %d \n", i);
    for (uint8_t j = 0; j < num_workers; j++) // do sequential worker calling for now, then we do parallelism
    {
        std::cout << "centroids going to worker : " << i << "\n";
        print_centroids(global_centroids,K,cols_global);

        //initialize_worker_input(&worker_input, rows2, cols2, num_workers, num_clusters);
        //setup_input(&worker_input,rows2,cols2,num_clusters,test_dataset,global_centroids,init_labels[j],j);
        setup_input(&worker_input,rows_global,cols_global,K,data_points2,global_centroids,init_labels[j],j);
        status = modify_test_message_exchange(initiator_enclave_id, &ret_status,&worker_input,&worker_output[j]);
        if (status != SGX_SUCCESS || ret_status != 0) {
            printf("modify_test_message_exchange Ecall failed: ECALL return 0x%x, error code is 0x%x.\n", status, ret_status);
            sgx_destroy_enclave(initiator_enclave_id);
            return -1;
        }
    
        std::cout << "\nCENTROIDS coming from worker: " << j <<"\n";
        print_centroids(get_centroids(worker_output[j]),worker_output[j].n_clusters,worker_output[j].n_cols);
        global_centroids = compute_global_centroids(worker_output, num_workers, worker_input.n_clusters,worker_input.n_cols);
        std::cout << "GLOBAL CENTROIDS \n";
        print_centroids(global_centroids,worker_output[j].n_clusters,worker_output[j].n_cols);
        
        memcpy(init_labels[j], worker_output[j].labels,rows_global * sizeof(uint8_t)); 
      
    }
    delta = compute_delta(old_centroids,global_centroids,K,cols_global);
    if(delta<=THRESHOLD)
    {
        std::cout << "reached delta threshold: " << delta <<"\n";
         break;
    }
}

    char  labels[50]="unsealed_labels.txt";
    processFile.clusters_out(labels,rows_global, worker_output[0].labels);

 // labels
uint32_t rows1=(uint32_t)worker_output[0].n_rows;
uint8_t labels1 [rows1];
memcpy(labels1, worker_output[0].labels, rows1);
std::cout<<"Labels: \n";
for(int i=0;i < worker_output[0].n_rows;i++)
{
    uint32_t data45=(uint32_t)worker_output[0].labels[i];
    std::cout << data45 << "\n";
}

    // close ECDH session
    status = test_close_session(initiator_enclave_id, &ret_status);
    if (status != SGX_SUCCESS || ret_status != 0) {
        printf("test_close_session Ecall failed: ECALL return 0x%x, error code is 0x%x.\n", status, ret_status);
        sgx_destroy_enclave(initiator_enclave_id);
        return -1;
    }
    printf("Succeed to close Session...\n");

    sgx_destroy_enclave(initiator_enclave_id);

gettimeofday(&end_time, NULL);
    cpu_time_used = ((double) (end_time.tv_sec - start_time.tv_sec)) +
                    ((double) (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;

    char time[100]="computation time";

    std::cout << "Computation time:" << cpu_time_used << "\n";
    processFile.computation_time_out(time,computation_time);
    
    return 0;
}
float compute_delta(float** old_centroids,float ** new_centroids, uint8_t n_rows, uint8_t n_cols)
{
    float temp=0;
    for (uint8_t i = 0; i < n_rows; i++)
    {
        for (uint8_t j = 0; j < n_cols; j++)
        {
            temp+=(new_centroids[i][j]-old_centroids[i][j])*(new_centroids[i][j]-old_centroids[i][j]);
        }
    }
    return temp;
}

void print_centroids(float ** centroids,uint8_t n_rows,uint8_t n_cols)
{
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            std::cout << "i: " << i <<"      j: " << j <<", "<<centroids[i][j] << "\n";
        }
    }
}

void initialize_worker_input(worker_input_t *worker_input, uint32_t rows, uint32_t cols, uint32_t num_workers, uint32_t num_clusters) {
    worker_input_t* input = (worker_input_t*)malloc(sizeof(worker_input_t));
    input->worker_id = 1;
    input->n_clusters = num_clusters;
    input->n_rows = rows;
    input->n_cols = cols;
    float ** dataset = (float **) calloc (rows, sizeof(float *)); 
for(int i=0;i<rows;i++) 
{
    dataset[i] = (float *) calloc(cols, sizeof(float)); 
    for(int j=0;j<cols;j++){
        dataset[i][j] = (i^2) + j^(5); 
        printf("dataset going out from initiator: %f \n",dataset[i][j]);
    } 
}


float ** centroids = initialize_centroids(dataset,rows,cols,num_clusters);
 for(int i=0;i<num_clusters;i++){
     for(int j=0;j<cols;j++){
        centroids[i][j] = (i+j+1) * 2.2; 
        printf("centroids going out from initiator: %f \n",centroids[i][j]);
    } 
}


serialize_float_array(centroids,num_clusters,cols,input->serialized_centroids);
serialize_float_array(dataset,rows,cols,input->serialized_dataset);

 memcpy(worker_input, input, sizeof(worker_input_t));
}

void setup_input(worker_input_t *worker_input, uint32_t rows, uint32_t cols, 
                    uint32_t n_clusters, float ** dataset,
                    float ** new_centroids, uint8_t * labels, uint8_t worker_id) 
{

    worker_input_t* input = (worker_input_t*)malloc(sizeof(worker_input_t));
    input->worker_id = worker_id;
    input->n_clusters = n_clusters;
    input->n_rows = rows;
    input->n_cols = cols;
    memcpy(input->labels, labels, sizeof(uint8_t) * rows);
    serialize_float_array(new_centroids,n_clusters,cols,input->serialized_centroids);
    serialize_float_array(dataset,rows,cols,input->serialized_dataset);

    memcpy(worker_input, input, sizeof(worker_input_t));
    SAFE_FREE(input);
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

void deserialize_float_array(uint8_t *buffer, int rows, int cols, float **float_array) 
{
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int buffer_index = (i * cols + j) * sizeof(float);
            uint8_t *float_ptr = buffer + buffer_index;
            memcpy(&float_array[i][j], float_ptr, sizeof(float));
        }
    }
}

float ** initialize_centroids(float ** dataset, uint32_t n_rows, uint32_t n_cols, uint8_t n_clusters)
{
    srand(time(NULL));
    float ** new_centroids=(float **) calloc(n_clusters, sizeof(float *));
    for (size_t i = 0; i < n_clusters; i++)
    {
        new_centroids[i] = (float *) calloc(n_cols,sizeof(float));
        
        int random_value = rand() % n_rows;
        printf("random value: %d",random_value);
         for (int j = 0; j < n_cols; j++) {
            new_centroids[i][j] = dataset[random_value][j];
        } 
    }
    /*debug
    memcpy(new_centroids[0],dataset[0],n_cols*sizeof(float));
    memcpy(new_centroids[1],dataset[2],n_cols*sizeof(float));
    memcpy(new_centroids[2],dataset[28],n_cols*sizeof(float));*/
    return new_centroids;
}
void dispose_matrix(float ** matrix,uint32_t n_rows){
    for (size_t i = 0; i < n_rows; i++)
    {
        SAFE_FREE(matrix[i]);
    }
    SAFE_FREE(matrix);
}
float ** compute_global_centroids(worker_output_t * worker_output, uint8_t n_workers, uint8_t n_clusters,uint8_t n_cols)
{
    float ** new_centroids = (float **) calloc(n_clusters,sizeof(float*));
    for (size_t i = 0; i < n_clusters; i++)
    {
        new_centroids[i] = (float *) calloc(n_cols,sizeof(float));
        float * sum = (float *) calloc(n_cols,sizeof(float));
        for (size_t j = 0; j < n_workers; j++)
        {
            float ** current_centroids = get_centroids(worker_output[j]);
            sum = sum_vectors(sum,current_centroids[i],n_cols);
            dispose_matrix(current_centroids,n_clusters);
        }
        new_centroids[i] = divide_vector(sum, n_workers, n_cols);
        SAFE_FREE(sum);
    }
    return new_centroids;
}

float ** get_dataset(worker_output_t worker_output){
    float ** dataset_data = new float * [worker_output.n_rows]; 
    for(int i=0;i<worker_output.n_rows;i++) dataset_data[i] = new float [worker_output.n_cols]; 
    deserialize_float_array(worker_output.serialized_centroids,worker_output.n_rows,worker_output.n_cols,dataset_data);
    return dataset_data;
}
float ** get_centroids(worker_output_t worker_output){
    float ** centroids_data = new float * [worker_output.n_clusters]; //(uint16_t**)malloc(3 * sizeof(uint16_t*)); //create_centroids(rows,cols);
    for(int i=0;i<worker_output.n_clusters;i++) centroids_data[i] = new float [worker_output.n_cols]; //(uint16_t*)malloc(4 * sizeof(uint16_t));
    deserialize_float_array(worker_output.serialized_centroids,worker_output.n_clusters,worker_output.n_cols,centroids_data);
    return centroids_data;
}
float ** get_centroids_input(worker_input_t worker_output){
    float ** centroids_data = new float * [worker_output.n_clusters]; //(uint16_t**)malloc(3 * sizeof(uint16_t*)); //create_centroids(rows,cols);
    for(int i=0;i<worker_output.n_clusters;i++) centroids_data[i] = new float [worker_output.n_cols]; //(uint16_t*)malloc(4 * sizeof(uint16_t));
    deserialize_float_array(worker_output.serialized_centroids,worker_output.n_clusters,worker_output.n_cols,centroids_data);
    return centroids_data;
}

float * sum_vectors(float * vector1, float * vector2, uint32_t vector_length){
    float * result = (float *) calloc(vector_length , sizeof(float)); //new float[vector_length];//(float *) malloc(vector_length * sizeof(float));
    for(int i=0;i<vector_length;i++){
        result[i]=(float)(vector1[i]+vector2[i]);
    }
    return result;
}

float * divide_vector(float * vector1, uint32_t divisor, uint32_t vector_length){
    float * result = (float *) calloc(vector_length , sizeof(float)); //new float [vector_length];// malloc(vector_length * sizeof(float));
    for(int i=0;i<vector_length;i++){
        result[i]=(float)(vector1[i]/divisor);
    }
    return result;
}
