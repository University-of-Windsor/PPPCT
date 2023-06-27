#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */
#include "sgx_lfence.h" /* for sgx_lfence */

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_within_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (	\
	((a) += (b)) < (b)	\
)


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

static sgx_status_t SGX_CDECL sgx_scanf_override(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_scanf_override_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_scanf_override_t* ms = SGX_CAST(ms_scanf_override_t*, pms);
	ms_scanf_override_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_scanf_override_t), ms, sizeof(ms_scanf_override_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	char* _tmp_output = __in_ms.ms_output;
	size_t _len_output = 100 * sizeof(char);
	char* _in_output = NULL;

	CHECK_UNIQUE_POINTER(_tmp_output, _len_output);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_output != NULL && _len_output != 0) {
		if ( _len_output % sizeof(*_tmp_output) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		if ((_in_output = (char*)malloc(_len_output)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_output, 0, _len_output);
	}
	scanf_override(_in_output);
	if (_in_output) {
		if (memcpy_verw_s(_tmp_output, _len_output, _in_output, _len_output)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_output) free(_in_output);
	return status;
}

static sgx_status_t SGX_CDECL sgx_find_delta(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_find_delta_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_find_delta_t* ms = SGX_CAST(ms_find_delta_t*, pms);
	ms_find_delta_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_find_delta_t), ms, sizeof(ms_find_delta_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	double*** _tmp_iterate_centroids_global = __in_ms.ms_iterate_centroids_global;
	double* _tmp_delta = __in_ms.ms_delta;
	size_t _len_delta = sizeof(double);
	double* _in_delta = NULL;

	CHECK_UNIQUE_POINTER(_tmp_delta, _len_delta);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_delta != NULL && _len_delta != 0) {
		if ( _len_delta % sizeof(*_tmp_delta) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_delta = (double*)malloc(_len_delta);
		if (_in_delta == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_delta, _len_delta, _tmp_delta, _len_delta)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	find_delta(__in_ms.ms_Number_of_clusters, __in_ms.ms_Number_of_cols, __in_ms.ms_iter_counter, _tmp_iterate_centroids_global, _in_delta);
	if (_in_delta) {
		if (memcpy_verw_s(_tmp_delta, _len_delta, _in_delta, _len_delta)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_delta) free(_in_delta);
	return status;
}

static sgx_status_t SGX_CDECL sgx_find_centroids(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_find_centroids_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_find_centroids_t* ms = SGX_CAST(ms_find_centroids_t*, pms);
	ms_find_centroids_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_find_centroids_t), ms, sizeof(ms_find_centroids_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int* _tmp_points_inside_cluster_count = __in_ms.ms_points_inside_cluster_count;
	float** _tmp_cluster_points_sum = __in_ms.ms_cluster_points_sum;
	int** _tmp_iter_cluster_count_global = __in_ms.ms_iter_cluster_count_global;
	double*** _tmp_iterate_centroids_global = __in_ms.ms_iterate_centroids_global;


	find_centroids(__in_ms.ms_Number_of_clusters, __in_ms.ms_Number_of_cols, __in_ms.ms_iter_counter, _tmp_points_inside_cluster_count, _tmp_cluster_points_sum, _tmp_iter_cluster_count_global, _tmp_iterate_centroids_global);


	return status;
}

static sgx_status_t SGX_CDECL sgx_find_distance(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_find_distance_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_find_distance_t* ms = SGX_CAST(ms_find_distance_t*, pms);
	ms_find_distance_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_find_distance_t), ms, sizeof(ms_find_distance_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int* _tmp_myarray = __in_ms.ms_myarray;
	int* _tmp_points_inside_cluster_count = __in_ms.ms_points_inside_cluster_count;
	float** _tmp_cluster_points_sum = __in_ms.ms_cluster_points_sum;
	double*** _tmp_iterate_centroids_global = __in_ms.ms_iterate_centroids_global;


	find_distance(__in_ms.ms_start, __in_ms.ms_end, __in_ms.ms_Number_of_clusters, __in_ms.ms_number_of_cols, __in_ms.ms_Number_of_rows, __in_ms.ms_iter_counter, _tmp_myarray, _tmp_points_inside_cluster_count, _tmp_cluster_points_sum, _tmp_iterate_centroids_global);


	return status;
}

static sgx_status_t SGX_CDECL sgx_sealed_centroids_out(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_sealed_centroids_out_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_sealed_centroids_out_t* ms = SGX_CAST(ms_sealed_centroids_out_t*, pms);
	ms_sealed_centroids_out_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_sealed_centroids_out_t), ms, sizeof(ms_sealed_centroids_out_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	char* _tmp_filename = __in_ms.ms_filename;
	double*** _tmp_iter_centroids = __in_ms.ms_iter_centroids;


	sealed_centroids_out(_tmp_filename, __in_ms.ms_K, __in_ms.ms_number_of_iterations, _tmp_iter_centroids);


	return status;
}

static sgx_status_t SGX_CDECL sgx_sealed_clusters_out(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_sealed_clusters_out_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_sealed_clusters_out_t* ms = SGX_CAST(ms_sealed_clusters_out_t*, pms);
	ms_sealed_clusters_out_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_sealed_clusters_out_t), ms, sizeof(ms_sealed_clusters_out_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	char* _tmp_filename = __in_ms.ms_filename;
	int* _tmp_cluster_points = __in_ms.ms_cluster_points;
	size_t _len_cluster_points = sizeof(int);
	int* _in_cluster_points = NULL;

	CHECK_UNIQUE_POINTER(_tmp_cluster_points, _len_cluster_points);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_cluster_points != NULL && _len_cluster_points != 0) {
		if ( _len_cluster_points % sizeof(*_tmp_cluster_points) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_cluster_points = (int*)malloc(_len_cluster_points);
		if (_in_cluster_points == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_cluster_points, _len_cluster_points, _tmp_cluster_points, _len_cluster_points)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	sealed_clusters_out(_tmp_filename, __in_ms.ms_N, _in_cluster_points);

err:
	if (_in_cluster_points) free(_in_cluster_points);
	return status;
}

static sgx_status_t SGX_CDECL sgx_get_init_centroids(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_get_init_centroids_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_get_init_centroids_t* ms = SGX_CAST(ms_get_init_centroids_t*, pms);
	ms_get_init_centroids_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_get_init_centroids_t), ms, sizeof(ms_get_init_centroids_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	double*** _tmp_iterate_centroids = __in_ms.ms_iterate_centroids;
	size_t _len_iterate_centroids = sizeof(double**);
	double*** _in_iterate_centroids = NULL;

	CHECK_UNIQUE_POINTER(_tmp_iterate_centroids, _len_iterate_centroids);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_iterate_centroids != NULL && _len_iterate_centroids != 0) {
		if ( _len_iterate_centroids % sizeof(*_tmp_iterate_centroids) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_iterate_centroids = (double***)malloc(_len_iterate_centroids);
		if (_in_iterate_centroids == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_iterate_centroids, _len_iterate_centroids, _tmp_iterate_centroids, _len_iterate_centroids)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	get_init_centroids(__in_ms.ms_Number_of_clusters, __in_ms.ms_number_of_cols, __in_ms.ms_max_iter, _in_iterate_centroids);
	if (_in_iterate_centroids) {
		if (memcpy_verw_s(_tmp_iterate_centroids, _len_iterate_centroids, _in_iterate_centroids, _len_iterate_centroids)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_iterate_centroids) free(_in_iterate_centroids);
	return status;
}

static sgx_status_t SGX_CDECL sgx_seal(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_seal_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_seal_t* ms = SGX_CAST(ms_seal_t*, pms);
	ms_seal_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_seal_t), ms, sizeof(ms_seal_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_plaintext = __in_ms.ms_plaintext;
	size_t _tmp_plaintext_len = __in_ms.ms_plaintext_len;
	size_t _len_plaintext = _tmp_plaintext_len;
	uint8_t* _in_plaintext = NULL;
	sgx_sealed_data_t* _tmp_sealed_data = __in_ms.ms_sealed_data;
	size_t _tmp_sealed_size = __in_ms.ms_sealed_size;
	size_t _len_sealed_data = _tmp_sealed_size;
	sgx_sealed_data_t* _in_sealed_data = NULL;
	sgx_status_t _in_retval;

	CHECK_UNIQUE_POINTER(_tmp_plaintext, _len_plaintext);
	CHECK_UNIQUE_POINTER(_tmp_sealed_data, _len_sealed_data);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_plaintext != NULL && _len_plaintext != 0) {
		if ( _len_plaintext % sizeof(*_tmp_plaintext) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_plaintext = (uint8_t*)malloc(_len_plaintext);
		if (_in_plaintext == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_plaintext, _len_plaintext, _tmp_plaintext, _len_plaintext)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_sealed_data != NULL && _len_sealed_data != 0) {
		if ((_in_sealed_data = (sgx_sealed_data_t*)malloc(_len_sealed_data)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_sealed_data, 0, _len_sealed_data);
	}
	_in_retval = seal(_in_plaintext, _tmp_plaintext_len, _in_sealed_data, _tmp_sealed_size);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}
	if (_in_sealed_data) {
		if (memcpy_verw_s(_tmp_sealed_data, _len_sealed_data, _in_sealed_data, _len_sealed_data)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_plaintext) free(_in_plaintext);
	if (_in_sealed_data) free(_in_sealed_data);
	return status;
}

static sgx_status_t SGX_CDECL sgx_unseal(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_unseal_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_unseal_t* ms = SGX_CAST(ms_unseal_t*, pms);
	ms_unseal_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_unseal_t), ms, sizeof(ms_unseal_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	sgx_sealed_data_t* _tmp_sealed_data = __in_ms.ms_sealed_data;
	size_t _tmp_sealed_size = __in_ms.ms_sealed_size;
	size_t _len_sealed_data = _tmp_sealed_size;
	sgx_sealed_data_t* _in_sealed_data = NULL;
	uint8_t* _tmp_plaintext = __in_ms.ms_plaintext;
	uint32_t _tmp_plaintext_len = __in_ms.ms_plaintext_len;
	size_t _len_plaintext = _tmp_plaintext_len;
	uint8_t* _in_plaintext = NULL;
	sgx_status_t _in_retval;

	CHECK_UNIQUE_POINTER(_tmp_sealed_data, _len_sealed_data);
	CHECK_UNIQUE_POINTER(_tmp_plaintext, _len_plaintext);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_sealed_data != NULL && _len_sealed_data != 0) {
		_in_sealed_data = (sgx_sealed_data_t*)malloc(_len_sealed_data);
		if (_in_sealed_data == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_sealed_data, _len_sealed_data, _tmp_sealed_data, _len_sealed_data)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_plaintext != NULL && _len_plaintext != 0) {
		if ( _len_plaintext % sizeof(*_tmp_plaintext) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		if ((_in_plaintext = (uint8_t*)malloc(_len_plaintext)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_plaintext, 0, _len_plaintext);
	}
	_in_retval = unseal(_in_sealed_data, _tmp_sealed_size, _in_plaintext, _tmp_plaintext_len);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}
	if (_in_plaintext) {
		if (memcpy_verw_s(_tmp_plaintext, _len_plaintext, _in_plaintext, _len_plaintext)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_sealed_data) free(_in_sealed_data);
	if (_in_plaintext) free(_in_plaintext);
	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv; uint8_t is_switchless;} ecall_table[9];
} g_ecall_table = {
	9,
	{
		{(void*)(uintptr_t)sgx_scanf_override, 0, 0},
		{(void*)(uintptr_t)sgx_find_delta, 0, 0},
		{(void*)(uintptr_t)sgx_find_centroids, 0, 0},
		{(void*)(uintptr_t)sgx_find_distance, 0, 0},
		{(void*)(uintptr_t)sgx_sealed_centroids_out, 0, 0},
		{(void*)(uintptr_t)sgx_sealed_clusters_out, 0, 0},
		{(void*)(uintptr_t)sgx_get_init_centroids, 0, 0},
		{(void*)(uintptr_t)sgx_seal, 0, 0},
		{(void*)(uintptr_t)sgx_unseal, 0, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[5][9];
} g_dyn_entry_table = {
	5,
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_scan_input(char output[100])
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_output = 100 * sizeof(char);

	ms_ocall_scan_input_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_scan_input_t);
	void *__tmp = NULL;

	void *__tmp_output = NULL;

	CHECK_ENCLAVE_POINTER(output, _len_output);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (output != NULL) ? _len_output : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_scan_input_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_scan_input_t));
	ocalloc_size -= sizeof(ms_ocall_scan_input_t);

	if (output != NULL) {
		if (memcpy_verw_s(&ms->ms_output, sizeof(char*), &__tmp, sizeof(char*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_output = __tmp;
		if (_len_output % sizeof(*output) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset_verw(__tmp_output, 0, _len_output);
		__tmp = (void *)((size_t)__tmp + _len_output);
		ocalloc_size -= _len_output;
	} else {
		ms->ms_output = NULL;
	}

	status = sgx_ocall(0, ms);

	if (status == SGX_SUCCESS) {
		if (output) {
			if (memcpy_s((void*)output, _len_output, __tmp_output, _len_output)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_print_string(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_string_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_string_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_string_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_string_t));
	ocalloc_size -= sizeof(ms_ocall_print_string_t);

	if (str != NULL) {
		if (memcpy_verw_s(&ms->ms_str, sizeof(const char*), &__tmp, sizeof(const char*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}

	status = sgx_ocall(1, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_save_dataset(int* retval, const uint8_t* sealed_data, size_t sealed_size)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_sealed_data = sealed_size;

	ms_ocall_save_dataset_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_save_dataset_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(sealed_data, _len_sealed_data);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (sealed_data != NULL) ? _len_sealed_data : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_save_dataset_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_save_dataset_t));
	ocalloc_size -= sizeof(ms_ocall_save_dataset_t);

	if (sealed_data != NULL) {
		if (memcpy_verw_s(&ms->ms_sealed_data, sizeof(const uint8_t*), &__tmp, sizeof(const uint8_t*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_sealed_data % sizeof(*sealed_data) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, sealed_data, _len_sealed_data)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_sealed_data);
		ocalloc_size -= _len_sealed_data;
	} else {
		ms->ms_sealed_data = NULL;
	}

	if (memcpy_verw_s(&ms->ms_sealed_size, sizeof(ms->ms_sealed_size), &sealed_size, sizeof(sealed_size))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(2, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_load_dataset(int* retval, uint8_t* sealed_data, size_t sealed_size)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_sealed_data = sealed_size;

	ms_ocall_load_dataset_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_load_dataset_t);
	void *__tmp = NULL;

	void *__tmp_sealed_data = NULL;

	CHECK_ENCLAVE_POINTER(sealed_data, _len_sealed_data);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (sealed_data != NULL) ? _len_sealed_data : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_load_dataset_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_load_dataset_t));
	ocalloc_size -= sizeof(ms_ocall_load_dataset_t);

	if (sealed_data != NULL) {
		if (memcpy_verw_s(&ms->ms_sealed_data, sizeof(uint8_t*), &__tmp, sizeof(uint8_t*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_sealed_data = __tmp;
		if (_len_sealed_data % sizeof(*sealed_data) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset_verw(__tmp_sealed_data, 0, _len_sealed_data);
		__tmp = (void *)((size_t)__tmp + _len_sealed_data);
		ocalloc_size -= _len_sealed_data;
	} else {
		ms->ms_sealed_data = NULL;
	}

	if (memcpy_verw_s(&ms->ms_sealed_size, sizeof(ms->ms_sealed_size), &sealed_size, sizeof(sealed_size))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(3, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (sealed_data) {
			if (memcpy_s((void*)sealed_data, _len_sealed_data, __tmp_sealed_data, _len_sealed_data)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_save_data(int* retval, char* filename, const uint8_t* sealed_data, size_t sealed_size)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_sealed_data = sealed_size;

	ms_ocall_save_data_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_save_data_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(sealed_data, _len_sealed_data);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (sealed_data != NULL) ? _len_sealed_data : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_save_data_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_save_data_t));
	ocalloc_size -= sizeof(ms_ocall_save_data_t);

	if (memcpy_verw_s(&ms->ms_filename, sizeof(ms->ms_filename), &filename, sizeof(filename))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (sealed_data != NULL) {
		if (memcpy_verw_s(&ms->ms_sealed_data, sizeof(const uint8_t*), &__tmp, sizeof(const uint8_t*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_sealed_data % sizeof(*sealed_data) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, sealed_data, _len_sealed_data)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_sealed_data);
		ocalloc_size -= _len_sealed_data;
	} else {
		ms->ms_sealed_data = NULL;
	}

	if (memcpy_verw_s(&ms->ms_sealed_size, sizeof(ms->ms_sealed_size), &sealed_size, sizeof(sealed_size))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(4, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

