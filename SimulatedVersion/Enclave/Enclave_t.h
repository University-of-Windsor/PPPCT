#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */

#include "sgx_tseal.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

void scanf_override(char output[100]);
void find_delta(int Number_of_clusters, int Number_of_cols, int iter_counter, double*** iterate_centroids_global, double* delta);
void find_centroids(int Number_of_clusters, int Number_of_cols, int iter_counter, int* points_inside_cluster_count, float** cluster_points_sum, int** iter_cluster_count_global, double*** iterate_centroids_global);
void find_distance(int start, int end, int Number_of_clusters, int number_of_cols, int Number_of_rows, int iter_counter, int* myarray, int* points_inside_cluster_count, float** cluster_points_sum, double*** iterate_centroids_global);
void sealed_centroids_out(char* filename, int K, int number_of_iterations, double*** iter_centroids);
void sealed_clusters_out(char* filename, int N, int* cluster_points);
void get_init_centroids(int Number_of_clusters, int number_of_cols, int max_iter, double*** iterate_centroids);
sgx_status_t seal(uint8_t* plaintext, size_t plaintext_len, sgx_sealed_data_t* sealed_data, size_t sealed_size);
sgx_status_t unseal(sgx_sealed_data_t* sealed_data, size_t sealed_size, uint8_t* plaintext, uint32_t plaintext_len);

sgx_status_t SGX_CDECL ocall_scan_input(char output[100]);
sgx_status_t SGX_CDECL ocall_print_string(const char* str);
sgx_status_t SGX_CDECL ocall_save_dataset(int* retval, const uint8_t* sealed_data, size_t sealed_size);
sgx_status_t SGX_CDECL ocall_load_dataset(int* retval, uint8_t* sealed_data, size_t sealed_size);
sgx_status_t SGX_CDECL ocall_save_data(int* retval, char* filename, const uint8_t* sealed_data, size_t sealed_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
