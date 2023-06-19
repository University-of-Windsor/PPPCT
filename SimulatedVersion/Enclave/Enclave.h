
#ifndef _ENCLAVE_H_
#define _ENCLAVE_H_

#include <stdlib.h>
#include <assert.h>
#include "sgx_trts.h"
#include "sgx_tseal.h"

#define MAX_ITER 300
#define THRESHOLD 1e-6
#if defined(__cplusplus)
extern "C" {
#endif
#define DATASET_FILE "dataset.seal"
#define CENTROIDS	"centroids.seal"
#define COMPUTATION_TIME "computation_time"
#define LABELS "labels"

void printf(const char *fmt, ...);
void printf_helloworld(char * input);

#if defined(__cplusplus)
}
#endif

#endif /* !_ENCLAVE_H_ */
