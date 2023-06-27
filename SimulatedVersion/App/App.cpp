
using namespace std;

#include <omp.h>
#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "time.h"
#include "limits.h"
#include "float.h"
#include "math.h"
#include "string.h"
#include <iostream>
#include "FileIO.h"
#include <unistd.h>
#include <pwd.h>
#include <fstream>
#include <getopt.h>

#define MAX_PATH FILENAME_MAX
#define MAX_ITER 300
#define THRESHOLD 1e-6

int number_of_points_global;
int number_of_threads_global;
int number_of_iterations_global;
int K_global;
float** data_points_global;
double*** iter_centroids_global;
int* data_point_cluster_global;
int** iter_cluster_count_global;
int rows_global=0;
int cols_global=0;

// Defined global delta
double delta_global = THRESHOLD + 1;

#include "sgx_urts.h"
#include "App.h"
#include "utils.h"
#include "Enclave_u.h"

void kmeans_omp(int num_threads, int rows,int cols, int K, int* data_points_cluster_id,double*** iter_centroids, int number_of_iterations);

bool is_ecall_successful(sgx_status_t sgx_status, const std::string& err_msg, sgx_status_t ecall_return_value = SGX_SUCCESS);
/* Application entry */
int main(int argc, char* argv[])
{
    (void)(argc);
    (void)(argv);


    /* Initialize the enclave */
    if (initialize_enclave() < 0) {
        printf("Enter a character before exit ...\n");
        getchar();
        return -1;
    }
    //---------------------------------------------------------------------
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
    double start_time = 0, end_time = 0;
    double computation_time = 0;
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

    /*
  * start pre-processing with NMF
  */



    

    K = atoi(numberOfClusters);
    rows_global=atoi(numberOfCells);
    cols_global = atoi(numberOfGenes);

 printf("\nEnter number of threads to be used: ");
    scanf("%d", &num_threads);
start_time = omp_get_wtime();
    FileIO processFile=FileIO();
    std::string python_command {"python3.10 preprocessing.py "};
    python_command+=dataset;
    python_command+=" ";
    python_command+=numberOfClusters;
    system(const_cast<char*>(python_command.c_str()));

    cout << "python_command: " << python_command;
    data_points2=processFile.readCsvFile("W.csv",rows_global,K+1);
end_time = omp_get_wtime();
double read_time=end_time-start_time;

start_time = omp_get_wtime();


    /*
     * Start data sealing of dataset (AES 128 bit Encryption)
     */

        size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(data_points2);
        uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
        sgx_status_t status;
        sgx_status_t ecall_status;
        status = seal(global_eid, &ecall_status,
                (uint8_t*)&data_points2, sizeof(data_points2),
                (sgx_sealed_data_t*)sealed_data, sealed_size);

        if (!is_ecall_successful(status, "Sealing failed :(", ecall_status)) {
            return -1;
        }
        ocall_save_dataset(sealed_data,sealed_size);
        //std::cout << "\nSealing succeed" <<  std::endl;

        /*
         * End sealing of Dataset
         */




     /*
      * Start parallel K-means process
      */



    cols_global=K+1;
    kmeans_omp(num_threads, rows_global,cols_global, K, cluster_points, iter_centroids, number_of_iterations);
    end_time = omp_get_wtime();

    ////////THIS PART IS TO GENERATE UNSEALED LABELS THAT IS SPECIFIC TO iDASH EVALUATORS
    char  labels[50]="unsealed_labels.txt";
    processFile.clusters_out(labels,rows_global, data_point_cluster_global);
    ////////////////////////////////////////////////////////////////////////////////////

    sealed_clusters_out(global_eid,"labels.seal",rows_global, data_point_cluster_global);
    sealed_centroids_out(global_eid,"centroids.seal",K, number_of_iterations, iter_centroids_global);

    computation_time = end_time - start_time;
    char time[100]="computation time";


    processFile.computation_time_out(time,computation_time);

cout << "\nTime processing: " << computation_time  << "\n";
cout << "\nTime Pre-processing : " << read_time  << "\n";
cout << "\nTime Taken: " << computation_time+read_time  << "\n";
//    printf("Time Taken: %f \n", );
    //printf("Cluster centroids sealed and saved\n");
    //printf("Class labels sealed and saved saved\n");
    //printf("Computation time sealed and saved\n");


    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);

    return 0;
}

void kmeans_omp(int num_threads, int rows, int cols, int K, int* data_points_cluster_id,double*** iter_centroids, int number_of_iterations)
{

    // Initialize global variables
    number_of_points_global = rows;
    number_of_threads_global = num_threads;
    number_of_iterations_global = 0;
    K_global = K;

    data_points_cluster_id = (int*)malloc(rows * sizeof(int));   //Allocating space of 4 units each for N data points
    data_point_cluster_global = data_points_cluster_id;


    iter_centroids_global = new double ** [MAX_ITER+1];
    for(int i=0;i<MAX_ITER+1;i++){
        iter_centroids_global[i] = new double * [K];
        for (int j = 0; j < K; j++){
            iter_centroids_global[i][j] = new double [cols];
            for (int k = 0; k < cols; k++) iter_centroids_global[i][j][k] = 0;
        }
    }

    get_init_centroids(global_eid,K,cols,MAX_ITER,iter_centroids_global);

    iter_cluster_count_global = (int**)malloc(MAX_ITER * sizeof(int*));
    for (int i = 0; i < MAX_ITER; i++)
    {
        iter_cluster_count_global[i] = (int*)calloc(K, sizeof(int));
    }

    omp_set_nested(1);
    // Creating threads
    omp_set_num_threads(num_threads);

    //printf("\nProcessing...Please wait!\n");

#pragma omp parallel //shared(cols,rows,data_point_cluster_global,iter_centroids_global,delta_global) //shared(number_of_iterations_global,iter_centroids_global)
    {
    	int ID=omp_get_thread_num();
        //printf("Thread: %d created!\n", ID);
       Fit_Transform2(&ID,K,cols,rows,
    		   delta_global,number_of_iterations_global,data_point_cluster_global,iter_centroids_global);
    }

    // Record number_of_iterations
    number_of_iterations = number_of_iterations_global;

    // Record number of iterations and store iter_centroids_global data into iter_centroids
    int iter_centroids_size = number_of_iterations + 1;
    iter_centroids=new double **[iter_centroids_size];
    for (int i = 0; i < iter_centroids_size; i++)
    {
        iter_centroids[i]=new double *[K];
        for(int j=0;j<K;j++){
            iter_centroids[i][j]=new double[cols];
            for(int l=0;l<cols;l++)
                iter_centroids[i][j][l]=iter_centroids_global[i][j][l];
        }
    }
}




void Fit_Transform2(int * tid, int Number_of_clusters, int Number_of_cols, int Number_of_rows,
		double &delta,int& number_of_iterations_global,int* data_point_cluster_global, double *** iterate_centroids_global)
{
    int* id = (int*)tid;

    // Assigning data points range to each thread
    int data_length_per_thread = number_of_points_global / number_of_threads_global;
    int start = (*id) * data_length_per_thread;
    int end = start + data_length_per_thread;
    if (end + data_length_per_thread > number_of_points_global)
    {
        //To assign last undistributed points to this thread for computation, change end index to number_of_points_global
        end = number_of_points_global;
        data_length_per_thread = number_of_points_global - start;
    }


    double min_dist=99999;
    double current_dist=0;

    //float cluster_points_sum[Number_of_clusters][Number_of_rows];
    float ** cluster_points_sum =new float *[Number_of_clusters];
    for(int i=0;i<Number_of_clusters;i++){
    	cluster_points_sum[i]=new float[Number_of_rows];
    }
    // Cluster id associated with each point
    int *myarray=new int[data_length_per_thread];
    int point_to_cluster_id[data_length_per_thread]={};
    int points_inside_cluster_count[Number_of_clusters];
    int iter_counter = 0;
    int i = 0, j = 0;
   // Initialize cluster_points_sum or centroid to 0.0
    for (i = 0; i < Number_of_clusters; i++)
       for(j=0; j< Number_of_cols ;j++)
            cluster_points_sum[i][j] = 0;
   // Initialize number of points for each cluster to 0
   for (i = 0; i < Number_of_clusters; i++)
       points_inside_cluster_count[i] = 0;

   while ((delta > THRESHOLD) && (iter_counter < MAX_ITER))
   {
	   find_distance(global_eid,start,end,Number_of_clusters,Number_of_cols,Number_of_rows,iter_counter,myarray,points_inside_cluster_count,cluster_points_sum,iterate_centroids_global);


	#pragma omp critical
        {
        	find_centroids(global_eid,Number_of_clusters,Number_of_cols,iter_counter,points_inside_cluster_count,cluster_points_sum, iter_cluster_count_global,iterate_centroids_global);
        }

		#pragma omp barrier
        if ((*id)== 0)
        {
        	find_delta(global_eid,Number_of_clusters,Number_of_cols,iter_counter,iterate_centroids_global,&delta);
            number_of_iterations_global++;
        }

        // Wait for all thread to arrive and update the iter_counter by +1
		#pragma omp barrier
        iter_counter++;
   }   //End of while loop

        for (int i = start; i < end; i++)
        {
            // Assign points to clusters
            data_point_cluster_global[i] = myarray[i - start];
            assert(myarray[i - start] >= 0 && myarray[i - start] < Number_of_clusters);
        }
}



bool is_ecall_successful(sgx_status_t sgx_status, const std::string& err_msg,
        sgx_status_t ecall_return_value) {
    if (sgx_status != SGX_SUCCESS || ecall_return_value != SGX_SUCCESS) {
        printf("%s\n", err_msg.c_str());
        print_error_message(sgx_status);
        print_error_message(ecall_return_value);
        return false;
    }
    return true;
}
int ocall_save_dataset(const uint8_t* sealed_data, const size_t sealed_size) {
    ofstream file(DATASET_FILE, ios::out | ios::binary);
    if (file.fail()) {return 1;}
    file.write((const char*) sealed_data, sealed_size);
    file.close();
    return 0;
}
int ocall_save_data(char * FILENAME,const uint8_t* sealed_data, const size_t sealed_size) {
    ofstream file(FILENAME, ios::out | ios::binary);
    if (file.fail()) {return 1;}
    file.write((const char*) sealed_data, sealed_size);
    file.close();
    return 0;
}
int ocall_load_dataset(uint8_t* sealed_data, const size_t sealed_size) {
    ifstream file(DATASET_FILE, ios::in | ios::binary);
    if (file.fail()) {return 1;}
    file.read((char*) sealed_data, sealed_size);
    file.close();
    return 0;
}
void ocall_print_string(const char* str)
{
    printf("%s", str);
}

void ocall_scan_input(char a[100]) {
    scanf("%s", a);
}
