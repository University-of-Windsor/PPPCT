#include <stdio.h>      /* vsnprintf */
#include "sgx_trts.h"
#include "sgx_tseal.h"
#include "Enclave.h"
#include "Enclave_t.h"  /* print_string */
#include "string.h"
#include <cfloat>

#define MAX_ITER 300
#define THRESHOLD 1e-6
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int number_of_points_global;
int number_of_iterations_global;
double delta_global = THRESHOLD + 1;
int K_global;
int *data_points_global;
float *iter_centroids_global;
int *data_point_cluster_global;


/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}
void scanf_override(char output[100]){
    //char ** out;
    ocall_scan_input(output);
}
void printf_helloworld(char * input)
{
    	//---------------------------------------------------------------------
	int N;					// Number of data points (input)
	int K;					//Number of clusters to be formed (input)
	int* data_points;		//Data points (input)
	int* cluster_points;	//clustered data points (to be computed)
	float* iter_centroids;		//centroids of each iteration (to be computed)
	int number_of_iterations;     //no of iterations performed by algo (to be computed)
	//---------------------------------------------------------------------

	double start_time, end_time;
	double computation_time;

    //scanf_override(info);
    printf("Enter No. of Clusters: %s",input);
}

void find_delta(int Number_of_clusters,int Number_of_cols,int iter_counter,double ***iterate_centroids_global,double *delta)
{

    double temp_delta=0;
    for (int i = 0; i < Number_of_clusters; i++)
    {
        for (int j = 0; j < Number_of_cols; j++) {
        	temp_delta += (iterate_centroids_global[iter_counter + 1][i][j] - iterate_centroids_global[iter_counter][i][j])
                            * (iterate_centroids_global[iter_counter + 1][i][j] - iterate_centroids_global[iter_counter][i][j]);
       }
    }
    *delta=temp_delta;
}


void find_centroids(int Number_of_clusters,int Number_of_cols,int iter_counter, int * points_inside_cluster_count,float ** cluster_points_sum, int **iter_cluster_count_global,double ***iterate_centroids_global)
{
    for (int i = 0; i < Number_of_clusters; i++)
    {
        if (points_inside_cluster_count[i] == 0)
        {
            continue;
        }
        for (int j = 0; j < Number_of_cols; j++) {
        	iterate_centroids_global[iter_counter + 1][i][j] = (iterate_centroids_global[iter_counter + 1][i][j]
                * iter_cluster_count_global[iter_counter][i] + cluster_points_sum[i][j])
                / (float)(iter_cluster_count_global[iter_counter][i] + points_inside_cluster_count[i]);
        }
     }
}

void find_distance(int start,int end,int Number_of_clusters, int number_of_cols,int Number_of_rows,int iter_counter, int * myarray, int * points_inside_cluster_count,float ** cluster_points_sum,double ***iterate_centroids_global)
{
	/* Load encrypted dataset.
	 *
	 */
	size_t sealed_size = sizeof(sgx_sealed_data_t) + Number_of_clusters*sizeof(float);
    uint8_t* sealed_data2=(uint8_t*)malloc(sealed_size);
    int output;
    sgx_status_t ecall_status;
    ocall_load_dataset(&output,sealed_data2,sealed_size);
    float ** dataset3;
    sgx_status_t status;

        status = unseal((sgx_sealed_data_t*)sealed_data2, sealed_size,
                (uint8_t*)&dataset3, sizeof(dataset3));


    for (int i = start; i < end; i++)
     {
      double min_dist = DBL_MAX;
      for (int j = 0; j < Number_of_clusters; j++)
      {
    		//printf("\n dataset[%d][%d]:%f",i,j,dataset3[i][j]);
          double global_dist=0;
          for (int k = 0; k < number_of_cols; k++) {
              double value=(iterate_centroids_global[iter_counter][j][k] - (double)dataset3[i][k]);
              global_dist+=value * value;
          }
          if (global_dist < min_dist)
          {
              min_dist = global_dist;
              myarray[i-start]=j;
          }
      }
      points_inside_cluster_count[myarray[i - start]] += 1;
      for (int j = 0; j < number_of_cols; j++)
     	 cluster_points_sum[myarray[i - start]][j] += (float)dataset3[i][j];
     }
}


void get_init_centroids(int Number_of_clusters, int number_of_cols,int max_iter, double ***centroids_global)
{

	/* Load encrypted dataset.
	 *
	 */
	size_t sealed_size = sizeof(sgx_sealed_data_t) + Number_of_clusters*sizeof(float);
    uint8_t* sealed_data2=(uint8_t*)malloc(sealed_size);
    int output;
    sgx_status_t ecall_status;
    ocall_load_dataset(&output,sealed_data2,sealed_size);
    float ** dataset3;
    sgx_status_t status;

        status = unseal((sgx_sealed_data_t*)sealed_data2, sealed_size,
                (uint8_t*)&dataset3, sizeof(dataset3));

    // Assigning first K points to be initial centroids
    for (int i = 0; i < Number_of_clusters; i++)
    {
        for (int j = 0; j < number_of_cols; j++) {
            //cout << "Random value: " << mean_values[j] << endl;
            //int random_value = rand() % 10;
            centroids_global[0][i][j] = (double) dataset3[i][j];//random_value;//(i + 1) * mean_values[j]/rows;
           // printf("%d %d = %f \n", i , j , centroids_global[0][i][j]);
        }
    }
}


/**
 * @brief      Seals the plaintext given into the sgx_sealed_data_t structure
 *             given.
 *
 * @details    The plaintext can be any data. uint8_t is used to represent a
 *             byte. The sealed size can be determined by computing
 *             sizeof(sgx_sealed_data_t) + plaintext_len, since it is using
 *             AES-GCM which preserves length of plaintext. The size needs to be
 *             specified, otherwise SGX will assume the size to be just
 *             sizeof(sgx_sealed_data_t), not taking into account the sealed
 *             payload.
 *
 * @param      plaintext      The data to be sealed
 * @param[in]  plaintext_len  The plaintext length
 * @param      sealed_data    The pointer to the sealed data structure
 * @param[in]  sealed_size    The size of the sealed data structure supplied
 *
 * @return     Truthy if seal successful, falsy otherwise.
 */
sgx_status_t seal(uint8_t* plaintext, size_t plaintext_len, sgx_sealed_data_t* sealed_data, size_t sealed_size) {
    sgx_status_t status = sgx_seal_data(0, NULL, plaintext_len, plaintext, sealed_size, sealed_data);
    return status;
}

/**
 * @brief      Unseal the sealed_data given into c-string
 *
 * @details    The resulting plaintext is of type uint8_t to represent a byte.
 *             The sizes/length of pointers need to be specified, otherwise SGX
 *             will assume a count of 1 for all pointers.
 *
 * @param      sealed_data        The sealed data
 * @param[in]  sealed_size        The size of the sealed data
 * @param      plaintext          A pointer to buffer to store the plaintext
 * @param[in]  plaintext_max_len  The size of buffer prepared to store the
 *                                plaintext
 *
 * @return     Truthy if unseal successful, falsy otherwise.
 */
sgx_status_t unseal(sgx_sealed_data_t* sealed_data, size_t sealed_size, uint8_t* plaintext, uint32_t plaintext_len) {
    sgx_status_t status = sgx_unseal_data(sealed_data, NULL, NULL, (uint8_t*)plaintext, &plaintext_len);
    return status;
}

void sealed_centroids_out(char *filename,int K,int number_of_iterations, double*** iter_centroids)
{
    size_t sealed_size = sizeof(sgx_sealed_data_t) + (sizeof(iter_centroids));
    uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
    sgx_status_t status;
    int output;
    sgx_status_t ecall_status;
    status = seal((uint8_t*)&iter_centroids, sizeof(iter_centroids),
            (sgx_sealed_data_t*)sealed_data, sealed_size);

    ocall_save_data(&output,filename,sealed_data,sealed_size);

}

void sealed_clusters_out(char *filename,int N, int* cluster_points)
{
    size_t sealed_size = sizeof(sgx_sealed_data_t) + (sizeof(cluster_points));
    uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
    sgx_status_t status;
    int output;
    sgx_status_t ecall_status;
    status = seal((uint8_t*)&cluster_points, sizeof(cluster_points),
            (sgx_sealed_data_t*)sealed_data, sealed_size);
    ocall_save_data(&output,filename,sealed_data,sealed_size);

}
