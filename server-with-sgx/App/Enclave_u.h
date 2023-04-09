#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OCALL_PRINT_STRING_DEFINED__
#define OCALL_PRINT_STRING_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_string, (const char* str));
#endif
#ifndef OCALL_RECEIVE_DEFINED__
#define OCALL_RECEIVE_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_receive, (char* id, size_t idLength, char* buf, size_t bufLength));
#endif
#ifndef OCALL_SEND_DEFINED__
#define OCALL_SEND_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_send, (const char* id, const char* buffer));
#endif

sgx_status_t ecall_init(sgx_enclave_id_t eid, const char* indices);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
