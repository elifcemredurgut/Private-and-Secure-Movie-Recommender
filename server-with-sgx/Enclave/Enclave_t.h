#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

void ecall_init(const char* indices);

sgx_status_t SGX_CDECL ocall_print_string(const char* str);
sgx_status_t SGX_CDECL ocall_receive(char* id, size_t idLength, char* buf, size_t bufLength);
sgx_status_t SGX_CDECL ocall_send(const char* id, const char* buffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
