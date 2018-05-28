// nginx module that anonymizes IP addresses (e.g. for logging).
// Copyright Mason Simon 2018

#include <ngx_http.h>
#include <ngx_crypt.h>
#include <time.h>
#include "ngx_ipscrub_support.h"
#include "ngx_ipscrub_debug.h"

typedef struct {
  ngx_int_t  period_seconds;
} ngx_ipscrub_conf_t;

static void * ngx_ipscrub_create_conf(ngx_conf_t *cf);
static char * ngx_ipscrub_conf(ngx_conf_t *cf, void *conf);

static ngx_int_t ngx_http_variable_remote_addr_ipscrub(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);

static ngx_int_t ngx_ipscrub_init(ngx_conf_t *cf);
static ngx_int_t ngx_ipscrub_add_variables(ngx_conf_t *cf);

static ngx_http_variable_t ngx_http_ipscrub_vars[] = {
    {ngx_string("remote_addr_ipscrub"), NULL, ngx_http_variable_remote_addr_ipscrub, 0, 0, 0},
    {ngx_string("ipscrub_hash_debug"), NULL, ngx_http_variable_remote_addr_ipscrub_debug, 0, 0, 0},
    {ngx_string("ipscrub_salted_hash_debug"), NULL, ngx_http_variable_ipscrub_salted_hash_debug, 0, 0, 0},
    {ngx_null_string, NULL, NULL, 0, 0, 0 }};


static ngx_command_t  ngx_ipscrub_commands[] = {

    { ngx_string("ipscrub_period_seconds"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_HTTP_MAIN_CONF_OFFSET,
      offsetof(ngx_ipscrub_conf_t, period_seconds),
      NULL },

    ngx_null_command
};

// Globals.
const int default_period_seconds = 10 * 60; // Period between salt changes.
time_t period_start = -1;
#define num_nonce_bytes 16
u_char nonce[num_nonce_bytes]; // Input to salt generation.


static ngx_http_module_t  ngx_ipscrub_module_ctx = {
    ngx_ipscrub_init,                      /* preconfiguration */
    NULL,                                  /* postconfiguration */

    ngx_ipscrub_create_conf,               /* create main configuration */
    ngx_ipscrub_conf,                      /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


ngx_module_t  ngx_ipscrub_module = {
    NGX_MODULE_V1,
    &ngx_ipscrub_module_ctx,               /* module context */
    ngx_ipscrub_commands,                  /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


/* Configuration support. */
static void *
ngx_ipscrub_create_conf(ngx_conf_t *cf)
{
    ngx_ipscrub_conf_t  *icf;

    icf = ngx_pcalloc(cf->pool, sizeof(ngx_ipscrub_conf_t));
    if (icf == NULL) {
        return NULL;
    }

    icf->period_seconds = NGX_CONF_UNSET;

    return icf;
}

static char *
ngx_ipscrub_conf(ngx_conf_t *cf, void *conf)
{
    ngx_ipscrub_conf_t *icf = conf;

    ngx_conf_init_value(icf->period_seconds, default_period_seconds);

    return NGX_CONF_OK;
}


/* Variable support. */
static ngx_int_t
ngx_ipscrub_init(ngx_conf_t *cf)
{
  return ngx_ipscrub_add_variables(cf);
}

static ngx_int_t
ngx_ipscrub_add_variables(ngx_conf_t *cf)
{
  ngx_http_variable_t *var, *v;

  for (v = ngx_http_ipscrub_vars; v->name.len; v++)
  {
    var = ngx_http_add_variable(cf, &v->name, v->flags);
    if (var == NULL)
    {
      return NGX_ERROR;
    }

    var->get_handler = v->get_handler;
    var->data = v->data;
  }

  return NGX_OK;
}

static ngx_int_t
ngx_http_variable_remote_addr_ipscrub(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data)
{
  ngx_int_t   rc;
  u_char     *hashed;
  u_char     *combined;
  ngx_str_t   salt;
  u_int       len;

  // Get period_seconds from current configuration.
  ngx_ipscrub_conf_t  *icf;
  icf = ngx_http_get_module_main_conf(r, ngx_ipscrub_module);

  // Regenerate salt if past end of period.
  time_t now = time(NULL);
  if (period_start == -1 || now - period_start > icf->period_seconds) {
    rc = randbytes((u_char *) &nonce, num_nonce_bytes);
    if (rc != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    // TODO: actually calculate when period_start should have been.
    period_start = now;
  }

  salt.data = (u_char *) &nonce;
  salt.len = num_nonce_bytes;

  // Although ngx_crypt provides a salted SHA function, specified by a salt beginning with {SSHA}, that function exposes the salt in its result. For our security model, this is inappropriate. Instead, we use the regular nginx SHA function specified by {SHA}, and manually combine the nonce and plaintext.
  rc = concat(r->pool, r->connection->addr_text, salt, &combined);
  if (rc != NGX_OK) {
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  rc = ngx_crypt(r->pool, combined, (u_char *) "{SHA}", &hashed);
  if (rc != NGX_OK) {
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  // Strip prefix.
  u_char *obscured = hashed + (sizeof("{SHA}") - 1);

  // Compute number of bytes to represent hashed address.
  if (r->connection->addr_text.data[3] == '.' ||
      r->connection->addr_text.data[2] == '.' ||
      r->connection->addr_text.data[1] == '.') {
    // This is an IPv4 address.
    // IPv4 has a 32-bit address space. Base64 is 6 bits-per-char. ceil(32/6) = 6.
    len = 6;
  } else {
    // This is an IPv6 address.
    // IPv6 has a 128-bit address space. Base64 is 6 bits-per-char. ceil(128/6) = 22.
    len = 22;
  }

  v->len = len; 
  v->valid = 1;
  v->no_cacheable = 0;
  v->not_found = 0;
  v->data = obscured;

  return NGX_OK;
}
