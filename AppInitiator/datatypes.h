#include "sgx_report.h"
#include "sgx_eid.h"
#include "sgx_ecp_types.h"
#include "sgx_dh.h"
#include "sgx_tseal.h"
#ifndef DATATYPES_H_
#define DATATYPES_H_


#define N_ROWS             10
#define N_COLS             4
#define N_CLUSTERS         3
#define N_ROWS_PER_WORKER  3000     


#define DH_KEY_SIZE        20
#define NONCE_SIZE         16
#define MAC_SIZE           16
#define MAC_KEY_SIZE       16
#define PADDING_SIZE       16

#define TAG_SIZE        16
#define IV_SIZE            12

#define DERIVE_MAC_KEY      0x0
#define DERIVE_SESSION_KEY  0x1
#define DERIVE_VK1_KEY      0x3
#define DERIVE_VK2_KEY      0x4

#define CLOSED 0x0
#define IN_PROGRESS 0x1
#define ACTIVE 0x2

#define MESSAGE_EXCHANGE 0x0
#define ENCLAVE_TO_ENCLAVE_CALL 0x1

#define INVALID_ARGUMENT                   -2   ///< Invalid function argument
#define LOGIC_ERROR                        -3   ///< Functional logic error
#define FILE_NOT_FOUND                     -4   ///< File not found

#define SAFE_FREE(ptr)     {if (NULL != (ptr)) {free(ptr); (ptr)=NULL;}}

#define VMC_ATTRIBUTE_MASK  0xFFFFFFFFFFFFFFCB

typedef uint8_t dh_nonce[NONCE_SIZE];
typedef uint8_t cmac_128[MAC_SIZE];

#pragma pack(push, 1)

//Format of the AES-GCM message being exchanged between the source and the destination enclaves
typedef struct _secure_message_t
{
    uint32_t session_id; //Session ID identifying the session to which the message belongs
    sgx_aes_gcm_data_t message_aes_gcm_data;
}secure_message_t;

//Format of the input function parameter structure
typedef struct _ms_in_msg_exchange_t {
    uint32_t msg_type; //Type of Call E2E or general message exchange
    uint32_t target_fn_id; //Function Id to be called in Destination. Is valid only when msg_type=ENCLAVE_TO_ENCLAVE_CALL
    uint32_t inparam_buff_len; //Length of the serialized input parameters
    char inparam_buff[1]; //Serialized input parameters
} ms_in_msg_exchange_t;

//Format of the return value and output function parameter structure
typedef struct _ms_out_msg_exchange_t {
    uint32_t retval_len; //Length of the return value
    uint32_t ret_outparam_buff_len; //Length of the serialized return value and output parameters
    char ret_outparam_buff[1]; //Serialized return value and output parameters
} ms_out_msg_exchange_t;

//Session Tracker to generate session ids
typedef struct _session_id_tracker_t
{
    uint32_t          session_id;
}session_id_tracker_t;

/* typedef struct _matrix_t
{
    double **data;
    uint32_t rows;
    uint32_t cols;
}matrix_t;
 */
typedef struct _matrix_t {
    float* data;
    int rows;
    int cols;
}matrix_t;


typedef struct _worker_input_t
{
        uint8_t               worker_id; 

    uint8_t               n_clusters; 
    uint32_t              n_rows;
    uint8_t               n_cols;
    uint8_t              labels[N_ROWS_PER_WORKER];
    uint8_t              serialized_dataset[N_ROWS_PER_WORKER*N_COLS*sizeof(float)];
    uint8_t              serialized_centroids[N_CLUSTERS*N_COLS*sizeof(float)];
}worker_input_t;

typedef struct _worker_output_t
{
        uint8_t               worker_id; 

    uint8_t               n_clusters; 
    uint8_t               n_cols;  
    uint32_t              n_rows;
    uint8_t               labels[N_ROWS_PER_WORKER];
    uint8_t              serialized_centroids[N_CLUSTERS*N_COLS*sizeof(float)];
/*     uint32_t                mem_counter[N_CLUSTERS];
    uint8_t               mem_matrix[N_ROWS_PER_WORKER*N_CLUSTERS*sizeof(uint8_t)]; */
}worker_output_t;

matrix_t* create_centroids(uint32_t rows, uint32_t cols);
void destroy_centroids(matrix_t* matrix);
void set_centroid_value(matrix_t* matrix, uint32_t row, uint32_t col, double value);
double get_centroid_value(matrix_t* matrix, uint32_t row, uint32_t col);



#pragma pack(pop)

#endif
