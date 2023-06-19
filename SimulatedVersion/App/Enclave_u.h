#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */

#include "sgx_tseal.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OCALL_SCAN_INPUT_DEFINED__
#define OCALL_SCAN_INPUT_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_scan_input, (char output[100]));
#endif
#ifndef OCALL_PRINT_STRING_DEFINED__
#define OCALL_PRINT_STRING_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_string, (const char* str));
#endif
#ifndef OCALL_SAVE_DATASET_DEFINED__
#define OCALL_SAVE_DATASET_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_save_dataset, (const uint8_t* sealed_data, size_t sealed_size));
#endif
#ifndef OCALL_LOAD_DATASET_DEFINED__
#define OCALL_LOAD_DATASET_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_load_dataset, (uint8_t* sealed_data, size_t sealed_size));
#endif
#ifndef OCALL_SAVE_DATA_DEFINED__
#define OCALL_SAVE_DATA_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_save_data, (char* filename, const uint8_t* sealed_data, size_t sealed_size));
#endif

sgx_status_t scanf_override(sgx_enclave_id_t eid, char output[100]);
sgx_status_t find_delta(sgx_enclave_id_t eid, int Number_of_clusters, int Number_of_cols, int iter_counter, double*** iterate_centroids_global, double* delta);
sgx_status_t find_centroids(sgx_enclave_id_t eid, int Number_of_clusters, int Number_of_cols, int iter_counter, int* points_inside_cluster_count, float** cluster_points_sum, int** iter_cluster_count_global, double*** iterate_centroids_global);
sgx_status_t find_distance(sgx_enclave_id_t eid, int start, int end, int Number_of_clusters, int number_of_cols, int Number_of_rows, int iter_counter, int* myarray, int* points_inside_cluster_count, float** cluster_points_sum, double*** iterate_centroids_global);
sgx_status_t sealed_centroids_out(sgx_enclave_id_t eid, char* filename, int K, int number_of_iterations, double*** iter_centroids);
sgx_status_t sealed_clusters_out(sgx_enclave_id_t eid, char* filename, int N, int* cluster_points);
sgx_status_t get_init_centroids(sgx_enclave_id_t eid, int Number_of_clusters, int number_of_cols, int max_iter, double*** iterate_centroids);
sgx_status_t seal(sgx_enclave_id_t eid, sgx_status_t* retval, uint8_t* plaintext, size_t plaintext_len, sgx_sealed_data_t* sealed_data, size_t sealed_size);
sgx_status_t unseal(sgx_enclave_id_t eid, sgx_status_t* retval, sgx_sealed_data_t* sealed_data, size_t sealed_size, uint8_t* plaintext, uint32_t plaintext_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
