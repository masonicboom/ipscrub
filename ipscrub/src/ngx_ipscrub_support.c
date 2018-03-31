// Support library for ipscrub module.
// Copyright Mason Simon 2018

#include "ngx_ipscrub_support.h"

// null_terminate allocates a new, null-terminated string based on input.
ngx_int_t null_terminate(ngx_pool_t *pool, ngx_str_t input, u_char **out)
{
  *out = ngx_pnalloc(pool, input.len + 1);
  if (*out == NULL) {
      return NGX_ERROR;
  }
  (void) ngx_cpymem(*out, input.data, input.len);
  (*out)[input.len] = '\0';

  return NGX_OK;
}

// concat concatenates prefix and suffix then null-terminates the result.
ngx_int_t concat(ngx_pool_t *pool, ngx_str_t prefix, ngx_str_t suffix, u_char **out)
{
  // Allocate.
  *out = ngx_pnalloc(pool, prefix.len + suffix.len + 1);
  if (*out == NULL) {
      return NGX_ERROR;
  }

  // Write prefix.
  (void) ngx_cpymem(*out, prefix.data, prefix.len);

  // Write suffix.
  (void) ngx_cpymem(*out + prefix.len, suffix.data, suffix.len);

  // Terminate.
  (*out)[prefix.len + suffix.len] = '\0';

  return NGX_OK;
}
