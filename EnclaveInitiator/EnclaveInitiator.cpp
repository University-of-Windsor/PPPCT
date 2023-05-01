#include "sgx_eid.h"
#include "EnclaveInitiator_t.h"
#include "EnclaveMessageExchange.h"
#include "error_codes.h"
#include "Utility_E1.h"
#include "sgx_dh.h"
#include "sgx_utils.h"
#include <map>


#define UNUSED(val) (void)(val)

#define RESPONDER_PRODID 1

std::map<sgx_enclave_id_t, dh_session_t>g_src_session_info_map;

dh_session_t g_session;

// This is hardcoded responder enclave's MRSIGNER for demonstration purpose. The content aligns to responder enclave's signing key
//Please replace with your project responder enclave's MRSIGNER in your project!!!
//The command to get your signed enclave's MRSIGNER: <SGX_SDK Installation Path>/bin/x64/sgx_sign dump -enclave <Signed Enclave> -dumpfile mrsigner.txt
//Find the signed enclave's MRSIGNER in the mrsigner.txt(mrsigner->value:), then replace blow value
sgx_measurement_t g_responder_mrsigner = {
	{
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	}
};

/* Function Description:
 *   This is ECALL routine to create ECDH session.
 *   When it succeeds to create ECDH session, the session context is saved in g_session.
 * */
extern "C" uint32_t test_create_session()
{
        return create_session(&g_session);
}

uint32_t modify_test_message_exchange(worker_input_t * secret_data, worker_output_t * response)
{
    ATTESTATION_STATUS ke_status = SUCCESS;
    uint32_t retstatus;
    uint32_t target_fn_id, msg_type;
    char* marshalled_inp_buff;
    size_t marshalled_inp_buff_len;
    char* out_buff;
    size_t out_buff_len;
    size_t max_out_buff_size;
    worker_output_t *secret_response;// = (worker_output_t*)malloc(sizeof(worker_output_t)+24);
    //uint32_t secret_data;

    target_fn_id = 0;
    msg_type = MESSAGE_EXCHANGE;
    max_out_buff_size = 16000; // it's assumed the maximum payload size in response message is 50 bytes, it's for demonstration purpose
    //secret_data = 1234; //Secret Data here is shown only for purpose of demonstration.
    worker_output_t output;
    //////Marshals the secret data into a buffer
    ke_status = marshal_message_exchange_request(target_fn_id, msg_type, *secret_data, &marshalled_inp_buff, &marshalled_inp_buff_len);

    if(ke_status != SUCCESS)
    {
        return ke_status;
    }

    //Core Reference Code function
    ke_status = send_request_receive_response(&g_session, marshalled_inp_buff,
                                                marshalled_inp_buff_len, max_out_buff_size, &out_buff, &out_buff_len);


   if(ke_status != SUCCESS)
    {
        SAFE_FREE(marshalled_inp_buff);
        SAFE_FREE(out_buff);
        return ke_status;
    }

    //secret_response = (worker_output_t *) malloc (sizeof(worker_output_t)); 
    //Un-marshal the secret response data
    ke_status = umarshal_message_exchange_response(out_buff, response); //secret_response);

    if(ke_status != SUCCESS)
    {
        SAFE_FREE(marshalled_inp_buff);
        SAFE_FREE(out_buff);
        return ke_status;
    }
  //  memcpy(response, secret_response, sizeof(worker_output_t));
  //  ke_status = get_output_ocall(&retstatus,secret_response,response);

    SAFE_FREE(marshalled_inp_buff); 
    SAFE_FREE(out_buff); 
    return SUCCESS;
}


/* Function Description:
 *   This is ECALL interface to close secure session*/
uint32_t test_close_session()
{
    ATTESTATION_STATUS ke_status = SUCCESS;

    ke_status = close_session(&g_session);

    //Erase the session context
    memset(&g_session, 0, sizeof(dh_session_t));
    return ke_status;
}

/* Function Description:
 *   This is to verify peer enclave's identity.
 * For demonstration purpose, we verify below points:
 *   1. peer enclave's MRSIGNER is as expected
 *   2. peer enclave's PROD_ID is as expected
 *   3. peer enclave's attribute is reasonable: it's INITIALIZED'ed enclave; in non-debug build configuration, the enclave isn't loaded with enclave debug mode.
 **/
extern "C" uint32_t verify_peer_enclave_trust(sgx_dh_session_enclave_identity_t* peer_enclave_identity)
{
    if (!peer_enclave_identity)
        return INVALID_PARAMETER_ERROR;

    // check peer enclave's MRSIGNER
    // Please enable blow check in your own project!!!
    /*
    if (memcmp((uint8_t *)&peer_enclave_identity->mr_signer, (uint8_t*)&g_responder_mrsigner, sizeof(sgx_measurement_t)))
        return ENCLAVE_TRUST_ERROR;
    */
    // check peer enclave's product ID and enclave attribute (should be INITIALIZED'ed)
    if (peer_enclave_identity->isv_prod_id != RESPONDER_PRODID || !(peer_enclave_identity->attributes.flags & SGX_FLAGS_INITTED))
        return ENCLAVE_TRUST_ERROR;

    // check the enclave isn't loaded in enclave debug mode, except that the project is built for debug purpose
#if defined(NDEBUG)
    if (peer_enclave_identity->attributes.flags & SGX_FLAGS_DEBUG)
    	return ENCLAVE_TRUST_ERROR;
#endif

    return SUCCESS;
}

/* Function Description: Operates on the input secret and generate the output secret
 * */
worker_input_t get_message_exchange_response(worker_output_t inp_secret_data)
{
   
worker_input_t worker_input;
uint32_t rows2 = inp_secret_data.n_rows;
uint32_t cols2 = inp_secret_data.n_cols;
uint32_t num_workers = 1;
uint32_t num_clusters = inp_secret_data.n_clusters;
//worker_input_t* input = 
initialize_worker_input(&worker_input, rows2, cols2, num_workers, num_clusters);
return worker_input;

}
void initialize_worker_input(worker_input_t *worker_input, uint32_t rows, uint32_t cols, uint32_t num_workers, uint32_t num_clusters) {
    worker_input_t* input = (worker_input_t*)malloc(sizeof(worker_input_t));
    input->worker_id = 0;
    input->n_clusters = num_clusters;
    input->n_rows = rows;
    input->n_cols = cols;
    memcpy(worker_input, input, sizeof(worker_input_t));
}
//Generates the response from the request message
/* Function Description:
 *   process request message and generate response
 * Parameter Description:
 *   [input] decrypted_data: this is pointer to decrypted message
 *   [output] resp_buffer: this is pointer to response message, the buffer is allocated inside this function
 *   [output] resp_length: this points to response length
 * */
extern "C" uint32_t message_exchange_response_generator(char* decrypted_data,
                                              char** resp_buffer,
                                              size_t* resp_length)
{
    ms_in_msg_exchange_t *ms;
    worker_output_t inp_secret_data;
    worker_input_t out_secret_data;
    if(!decrypted_data || !resp_length)
    {
        return INVALID_PARAMETER_ERROR;
    }
    ms = (ms_in_msg_exchange_t *)decrypted_data;

    if(umarshal_message_exchange_request(&inp_secret_data,ms) != SUCCESS)
        return ATTESTATION_ERROR;


    out_secret_data = get_message_exchange_response(inp_secret_data);

    if(marshal_message_exchange_response(resp_buffer, resp_length, out_secret_data) != SUCCESS)
        return MALLOC_ERROR;

    return SUCCESS;
}