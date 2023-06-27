#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_scanf_override_t {
	char* ms_output;
} ms_scanf_override_t;

typedef struct ms_find_delta_t {
	int ms_Number_of_clusters;
	int ms_Number_of_cols;
	int ms_iter_counter;
	double*** ms_iterate_centroids_global;
	double* ms_delta;
} ms_find_delta_t;

typedef struct ms_find_centroids_t {
	int ms_Number_of_clusters;
	int ms_Number_of_cols;
	int ms_iter_counter;
	int* ms_points_inside_cluster_count;
	float** ms_cluster_points_sum;
	int** ms_iter_cluster_count_global;
	double*** ms_iterate_centroids_global;
} ms_find_centroids_t;

typedef struct ms_find_distance_t {
	int ms_start;
	int ms_end;
	int ms_Number_of_clusters;
	int ms_number_of_cols;
	int ms_Number_of_rows;
	int ms_iter_counter;
	int* ms_myarray;
	int* ms_points_inside_cluster_count;
	float** ms_cluster_points_sum;
	double*** ms_iterate_centroids_global;
} ms_find_distance_t;

typedef struct ms_sealed_centroids_out_t {
	char* ms_filename;
	int ms_K;
	int ms_number_of_iterations;
	double*** ms_iter_centroids;
} ms_sealed_centroids_out_t;

typedef struct ms_sealed_clusters_out_t {
	char* ms_filename;
	int ms_N;
	int* ms_cluster_points;
} ms_sealed_clusters_out_t;

typedef struct ms_get_init_centroids_t {
	int ms_Number_of_clusters;
	int ms_number_of_cols;
	int ms_max_iter;
	double*** ms_iterate_centroids;
} ms_get_init_centroids_t;

typedef struct ms_seal_t {
	sgx_status_t ms_retval;
	uint8_t* ms_plaintext;
	size_t ms_plaintext_len;
	sgx_sealed_data_t* ms_sealed_data;
	size_t ms_sealed_size;
} ms_seal_t;

typedef struct ms_unseal_t {
	sgx_status_t ms_retval;
	sgx_sealed_data_t* ms_sealed_data;
	size_t ms_sealed_size;
	uint8_t* ms_plaintext;
	uint32_t ms_plaintext_len;
} ms_unseal_t;

typedef struct ms_ocall_scan_input_t {
	char* ms_output;
} ms_ocall_scan_input_t;

typedef struct ms_ocall_print_string_t {
	const char* ms_str;
} ms_ocall_print_string_t;

typedef struct ms_ocall_save_dataset_t {
	int ms_retval;
	const uint8_t* ms_sealed_data;
	size_t ms_sealed_size;
} ms_ocall_save_dataset_t;

typedef struct ms_ocall_load_dataset_t {
	int ms_retval;
	uint8_t* ms_sealed_data;
	size_t ms_sealed_size;
} ms_ocall_load_dataset_t;

typedef struct ms_ocall_save_data_t {
	int ms_retval;
	char* ms_filename;
	const uint8_t* ms_sealed_data;
	size_t ms_sealed_size;
} ms_ocall_save_data_t;

static sgx_status_t SGX_CDECL Enclave_ocall_scan_input(void* pms)
{
	ms_ocall_scan_input_t* ms = SGX_CAST(ms_ocall_scan_input_t*, pms);
	ocall_scan_input(ms->ms_output);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_print_string(void* pms)
{
	ms_ocall_print_string_t* ms = SGX_CAST(ms_ocall_print_string_t*, pms);
	ocall_print_string(ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_save_dataset(void* pms)
{
	ms_ocall_save_dataset_t* ms = SGX_CAST(ms_ocall_save_dataset_t*, pms);
	ms->ms_retval = ocall_save_dataset(ms->ms_sealed_data, ms->ms_sealed_size);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_load_dataset(void* pms)
{
	ms_ocall_load_dataset_t* ms = SGX_CAST(ms_ocall_load_dataset_t*, pms);
	ms->ms_retval = ocall_load_dataset(ms->ms_sealed_data, ms->ms_sealed_size);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_save_data(void* pms)
{
	ms_ocall_save_data_t* ms = SGX_CAST(ms_ocall_save_data_t*, pms);
	ms->ms_retval = ocall_save_data(ms->ms_filename, ms->ms_sealed_data, ms->ms_sealed_size);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[5];
} ocall_table_Enclave = {
	5,
	{
		(void*)Enclave_ocall_scan_input,
		(void*)Enclave_ocall_print_string,
		(void*)Enclave_ocall_save_dataset,
		(void*)Enclave_ocall_load_dataset,
		(void*)Enclave_ocall_save_data,
	}
};
sgx_status_t scanf_override(sgx_enclave_id_t eid, char output[100])
{
	sgx_status_t status;
	ms_scanf_override_t ms;
	ms.ms_output = (char*)output;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t find_delta(sgx_enclave_id_t eid, int Number_of_clusters, int Number_of_cols, int iter_counter, double*** iterate_centroids_global, double* delta)
{
	sgx_status_t status;
	ms_find_delta_t ms;
	ms.ms_Number_of_clusters = Number_of_clusters;
	ms.ms_Number_of_cols = Number_of_cols;
	ms.ms_iter_counter = iter_counter;
	ms.ms_iterate_centroids_global = iterate_centroids_global;
	ms.ms_delta = delta;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t find_centroids(sgx_enclave_id_t eid, int Number_of_clusters, int Number_of_cols, int iter_counter, int* points_inside_cluster_count, float** cluster_points_sum, int** iter_cluster_count_global, double*** iterate_centroids_global)
{
	sgx_status_t status;
	ms_find_centroids_t ms;
	ms.ms_Number_of_clusters = Number_of_clusters;
	ms.ms_Number_of_cols = Number_of_cols;
	ms.ms_iter_counter = iter_counter;
	ms.ms_points_inside_cluster_count = points_inside_cluster_count;
	ms.ms_cluster_points_sum = cluster_points_sum;
	ms.ms_iter_cluster_count_global = iter_cluster_count_global;
	ms.ms_iterate_centroids_global = iterate_centroids_global;
	status = sgx_ecall(eid, 2, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t find_distance(sgx_enclave_id_t eid, int start, int end, int Number_of_clusters, int number_of_cols, int Number_of_rows, int iter_counter, int* myarray, int* points_inside_cluster_count, float** cluster_points_sum, double*** iterate_centroids_global)
{
	sgx_status_t status;
	ms_find_distance_t ms;
	ms.ms_start = start;
	ms.ms_end = end;
	ms.ms_Number_of_clusters = Number_of_clusters;
	ms.ms_number_of_cols = number_of_cols;
	ms.ms_Number_of_rows = Number_of_rows;
	ms.ms_iter_counter = iter_counter;
	ms.ms_myarray = myarray;
	ms.ms_points_inside_cluster_count = points_inside_cluster_count;
	ms.ms_cluster_points_sum = cluster_points_sum;
	ms.ms_iterate_centroids_global = iterate_centroids_global;
	status = sgx_ecall(eid, 3, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t sealed_centroids_out(sgx_enclave_id_t eid, char* filename, int K, int number_of_iterations, double*** iter_centroids)
{
	sgx_status_t status;
	ms_sealed_centroids_out_t ms;
	ms.ms_filename = filename;
	ms.ms_K = K;
	ms.ms_number_of_iterations = number_of_iterations;
	ms.ms_iter_centroids = iter_centroids;
	status = sgx_ecall(eid, 4, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t sealed_clusters_out(sgx_enclave_id_t eid, char* filename, int N, int* cluster_points)
{
	sgx_status_t status;
	ms_sealed_clusters_out_t ms;
	ms.ms_filename = filename;
	ms.ms_N = N;
	ms.ms_cluster_points = cluster_points;
	status = sgx_ecall(eid, 5, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t get_init_centroids(sgx_enclave_id_t eid, int Number_of_clusters, int number_of_cols, int max_iter, double*** iterate_centroids)
{
	sgx_status_t status;
	ms_get_init_centroids_t ms;
	ms.ms_Number_of_clusters = Number_of_clusters;
	ms.ms_number_of_cols = number_of_cols;
	ms.ms_max_iter = max_iter;
	ms.ms_iterate_centroids = iterate_centroids;
	status = sgx_ecall(eid, 6, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t seal(sgx_enclave_id_t eid, sgx_status_t* retval, uint8_t* plaintext, size_t plaintext_len, sgx_sealed_data_t* sealed_data, size_t sealed_size)
{
	sgx_status_t status;
	ms_seal_t ms;
	ms.ms_plaintext = plaintext;
	ms.ms_plaintext_len = plaintext_len;
	ms.ms_sealed_data = sealed_data;
	ms.ms_sealed_size = sealed_size;
	status = sgx_ecall(eid, 7, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t unseal(sgx_enclave_id_t eid, sgx_status_t* retval, sgx_sealed_data_t* sealed_data, size_t sealed_size, uint8_t* plaintext, uint32_t plaintext_len)
{
	sgx_status_t status;
	ms_unseal_t ms;
	ms.ms_sealed_data = sealed_data;
	ms.ms_sealed_size = sealed_size;
	ms.ms_plaintext = plaintext;
	ms.ms_plaintext_len = plaintext_len;
	status = sgx_ecall(eid, 8, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

