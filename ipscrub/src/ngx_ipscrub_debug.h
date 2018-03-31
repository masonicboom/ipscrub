// Debug support code for ipscrub module.
// Copyright Mason Simon 2018

#ifndef _IPSCRUB_DEBUG_H_INCLUDED_
#define _IPSCRUB_DEBUG_H_INCLUDED_

#include <ngx_core.h>

ngx_int_t ngx_http_variable_remote_addr_ipscrub_debug(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
ngx_int_t ngx_http_variable_ipscrub_salted_hash_debug(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);

#endif /* _IPSCRUB_DEBUG_H_INCLUDED_ */
