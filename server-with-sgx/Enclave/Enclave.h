#ifndef _ENCLAVE_H_
#define _ENCLAVE_H_

#include <stdlib.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

void printf(const char *fmt, ...);
void ecall_init(const char *indices);

#if defined(__cplusplus)
}
#endif

#endif /* !_ENCLAVE_H_ */
