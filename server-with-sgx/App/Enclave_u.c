#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_ecall_init_t {
	const char* ms_indices;
	size_t ms_indices_len;
} ms_ecall_init_t;

typedef struct ms_ocall_print_string_t {
	const char* ms_str;
} ms_ocall_print_string_t;

typedef struct ms_ocall_receive_t {
	char* ms_id;
	size_t ms_idLength;
	char* ms_buf;
	size_t ms_bufLength;
} ms_ocall_receive_t;

typedef struct ms_ocall_send_t {
	const char* ms_id;
	const char* ms_buffer;
} ms_ocall_send_t;

static sgx_status_t SGX_CDECL Enclave_ocall_print_string(void* pms)
{
	ms_ocall_print_string_t* ms = SGX_CAST(ms_ocall_print_string_t*, pms);
	ocall_print_string(ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_receive(void* pms)
{
	ms_ocall_receive_t* ms = SGX_CAST(ms_ocall_receive_t*, pms);
	ocall_receive(ms->ms_id, ms->ms_idLength, ms->ms_buf, ms->ms_bufLength);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_send(void* pms)
{
	ms_ocall_send_t* ms = SGX_CAST(ms_ocall_send_t*, pms);
	ocall_send(ms->ms_id, ms->ms_buffer);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[3];
} ocall_table_Enclave = {
	3,
	{
		(void*)Enclave_ocall_print_string,
		(void*)Enclave_ocall_receive,
		(void*)Enclave_ocall_send,
	}
};
sgx_status_t ecall_init(sgx_enclave_id_t eid, const char* indices)
{
	sgx_status_t status;
	ms_ecall_init_t ms;
	ms.ms_indices = indices;
	ms.ms_indices_len = indices ? strlen(indices) + 1 : 0;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	return status;
}

