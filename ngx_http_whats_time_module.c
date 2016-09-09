#include <stdio.h>
#include <time.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char* ngx_http_whats_time(ngx_conf_t*,ngx_command_t*,void*);
static void* whats_time_create_loc_conf(ngx_conf_t*);
static ngx_int_t ngx_http_whats_time_handler(ngx_http_request_t*);

static ngx_command_t ngx_http_whats_time_commands[] = {
    { ngx_string("whats_time"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_whats_time,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};

static ngx_http_module_t ngx_http_whats_time_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,                          /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    whats_time_create_loc_conf,   /* create location configuration */
    NULL                           /* merge location configuration */
};

ngx_module_t ngx_http_whats_time_module = {
    NGX_MODULE_V1,
    &ngx_http_whats_time_module_ctx, /* module context */
    ngx_http_whats_time_commands,   /* module directives */
    NGX_HTTP_MODULE,                /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};

typedef struct {
    ngx_flag_t turn_on;
} ngx_http_whats_time_loc_conf_t;

static void *
whats_time_create_loc_conf(ngx_conf_t *cf) {
    ngx_http_whats_time_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_whats_time_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    return conf;
}

static char *
ngx_http_whats_time(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    // Will serve this location
    ngx_http_whats_time_loc_conf_t *my_conf = conf;
    my_conf->turn_on=1;

    //                           with that handler
    ngx_http_core_loc_conf_t *his_conf=ngx_http_conf_get_module_loc_conf(cf,
        ngx_http_core_module);
    his_conf->handler = ngx_http_whats_time_handler;

    return NGX_CONF_OK;
}

ngx_int_t c_cxx_bridge(ngx_buf_t*,ngx_pool_t*,const char*,int);

static ngx_int_t ngx_http_whats_time_handler(ngx_http_request_t *r) {
    // Only support http GET
    if(!(r->method & NGX_HTTP_GET)) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    // Don't need body
    ngx_int_t rc = ngx_http_discard_request_body(r);
    if(rc != NGX_OK && rc != NGX_AGAIN) {
        return rc;
    }

    // Could this question be asked?
    if(r->headers_in.if_modified_since) {
        return NGX_HTTP_NOT_MODIFIED;
    }

    // Send http header and maybe quit
    rc=ngx_http_send_header(r);
    if(rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    ngx_buf_t *answer = ngx_calloc_buf(r->pool);
    /*
     ngx_http_send_special_response() calls ngx_calloc_buf() but not 
      ngx_pfree(). Guess I do not need to call ngx_pfree(,answer), too?
    */
    for(;;) {
        // Out of memory?
        if(!answer) {
            rc=NGX_ERROR;
            break;
        }

        // What does she want? Maybe time?
        if( (5!=r->uri.len) || memcmp(r->uri.data,"/time",5) ) {
            // Compiler insists that line below has no effect. Thus comment-out
            //rc==NGX_OK;
            break;
        }
        
        // Cook chain suitable for ngx_http_output_filter()
        ngx_chain_t out;
        out.buf = answer;
        out.next = NULL;
        char response[100];
#if STACK_TIME
        time_t current_time = time(NULL);
#else
        time_t* current_time=ngx_palloc(r->pool,sizeof(time_t));
        *current_time = time(NULL);
#endif
        sprintf(response,"%s",ctime(
#if STACK_TIME
         &current_time
#else
         current_time
#endif
        ));
        int length = strlen(response);            // around 24 bytes
        answer->pos = ngx_palloc(r->pool,length); 
        if( !answer->pos ) {
            rc=NGX_ERROR;
            ngx_pfree(r->pool,answer);
            break;
        }
        answer->last = answer->pos + length;
        memcpy(answer->pos,response,length);
        answer->memory = answer->last_buf = answer->last_in_chain = 1;
        /*
         answer is marked read-only, ngx_http_output_filter() below should
          not free it
        */
        
        /*
         Send answer, free memory taken at line 143 above.

         Looks like ngx_pfree is not required, disabling call
        */
        #if 0
         void* free_me = answer->pos;
         rc=ngx_http_output_filter(r, &out);
         (void)ngx_pfree(r->pool,free_me);
        #else
         return ngx_http_output_filter(r, &out);
        #endif

        // not freeing current_time

        break;
    }
    return rc;
}
