// Support library for ipscrub module.
// Copyright Mason Simon 2018

#ifndef _IPSCRUB_SUPPORT_H_INCLUDED_
#define _IPSCRUB_SUPPORT_H_INCLUDED_

#include <ngx_core.h>

ngx_int_t null_terminate(ngx_pool_t *pool, ngx_str_t input, u_char **hashed);
ngx_int_t concat(ngx_pool_t *pool, ngx_str_t prefix, ngx_str_t suffix, u_char **out);

#endif /* _IPSCRUB_SUPPORT_H_INCLUDED_ */
