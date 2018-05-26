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

// randlong fills out with secure random bytes and returns NGX_OK iff successful.
ngx_int_t randlong(long *out) {
  #if !(NGX_DARWIN || NGX_SOLARIS || NGX_FREEBSD || NGX_LINUX)
    // Windows not supported a.t.m.
    // TODO: support Windows (https://msdn.microsoft.com/en-us/library/sxtz2fa8.aspx).
    return -1;
  #endif

  int rand = open("/dev/urandom", O_RDONLY);
  if (rand < 0) {
      return -1;
  }

  ssize_t ret = read(rand, out, sizeof(long));
  if (ret != sizeof(long)) {
      return -1;
  }

  return NGX_OK;
}
