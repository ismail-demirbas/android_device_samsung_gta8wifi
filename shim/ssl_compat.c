#include <stddef.h>
#include <stdint.h>
void *OPENSSL_sk_dup(const void *);
void *OPENSSL_sk_delete(void *, size_t);
void *sk_dup(const void *sk) { return OPENSSL_sk_dup(sk); }
void *sk_delete(void *sk, size_t where) { return OPENSSL_sk_delete(sk, where); }
struct cbs_st { const uint8_t *data; size_t len; };
void CBS_init(struct cbs_st *cbs, const uint8_t *data, size_t len) { cbs->data = data; cbs->len = len; }
