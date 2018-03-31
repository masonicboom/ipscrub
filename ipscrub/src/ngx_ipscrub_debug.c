// Debug support code for ipscrub module.
// Copyright Mason Simon 2018

#include <ngx_http.h>
#include <ngx_crypt.h>
#include "ngx_ipscrub_support.h"
#include "ngx_ipscrub_debug.h"

// This function hashes the request URI, for testing.
ngx_int_t
ngx_http_variable_remote_addr_ipscrub_debug(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data)
{
  u_char     *uri;
  ngx_int_t   rc;
  u_char     *hashed;

  u_char *salt = (u_char *) "{SHA}";

  // ngx_crypt computes the length of its second param using ngx_strlen, which requires a null-terminated string.
  rc = null_terminate(r->pool, r->uri, &uri);
  if (rc != NGX_OK) {
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  rc = ngx_crypt(r->pool, uri, salt, &hashed);
  if (rc != NGX_OK) {
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  // Strip prefix.
  u_char *obscured = hashed + (sizeof("{SHA}") - 1);

  v->len = 28; // SHA-1 is 160 bits. Base64 is 6 bits per char. ceil(160/6) = 27, but Base64 always groups into chunks of 4, so 28 chars total.
  v->valid = 1;
  v->no_cacheable = 0;
  v->not_found = 0;
  v->data = obscured;

  return NGX_OK;
}

// This function hashes the request URI, for testing, using the first 4 chars, excluding the initial slash, as a salt.
ngx_int_t
ngx_http_variable_ipscrub_salted_hash_debug(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data)
{
  ngx_str_t   salt;
  ngx_str_t   plaintext;
  ngx_int_t   rc;
  u_char     *combined;
  u_char     *hashed;

  u_int saltlen = 4;

  // First 4 chars of URL, excluding initial "/" are interpreted as the salt--they must be present.
  if (r->uri.len < saltlen + 1) {
    return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }
  salt.data = r->uri.data + 1;
  salt.len = saltlen;

  plaintext.data = r->uri.data + saltlen + 1;
  plaintext.len = r->uri.len - (saltlen + 1);

  // ngx_crypt computes the length of its second param using ngx_strlen, which requires a null-terminated string.
  rc = concat(r->pool, plaintext, salt, &combined);
  if (rc != NGX_OK) {
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  rc = ngx_crypt(r->pool, combined, (u_char *) "{SHA}", &hashed);
  if (rc != NGX_OK) {
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  // Strip prefix.
  u_char *obscured = hashed + (sizeof("{SHA}") - 1);

  v->len = 28; // SHA-1 is 160 bits. Base64 is 6 bits per char. ceil(160/6) = 27, but Base64 always groups into chunks of 4, so 28 chars total.
  v->valid = 1;
  v->no_cacheable = 0;
  v->not_found = 0;
  v->data = obscured;

  return NGX_OK;
}
