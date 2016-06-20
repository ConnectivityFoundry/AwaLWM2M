/*
 * er-resource.h
 *
 *  Created on: 27/03/2016
 *      Author: sean
 */

#ifndef CORE_SRC_ERBIUM_ER_RESOURCE_H_
#define CORE_SRC_ERBIUM_ER_RESOURCE_H_

typedef enum {
  NO_FLAGS = 0,

  /* methods to handle */
  METHOD_GET = (1 << 0),
  METHOD_POST = (1 << 1),
  METHOD_PUT = (1 << 2),
  METHOD_DELETE = (1 << 3),

  /* special flags */
  HAS_SUB_RESOURCES = (1 << 4),
  IS_SEPARATE = (1 << 5),
  IS_OBSERVABLE = (1 << 6),
  IS_PERIODIC = (1 << 7)
} rest_resource_flags_t;

struct resource_s;

/* signatures of handler functions */
typedef void (*restful_handler)(void *request, void *response,
                                uint8_t *buffer, uint16_t preferred_size,
                                int32_t *offset);
typedef void (*restful_final_handler)(struct resource_s *resource,
                                      void *request, void *response);
typedef void (*restful_periodic_handler)(void);
typedef void (*restful_response_handler)(void *data, void *response);
typedef void (*restful_trigger_handler)(void);

/* data structure representing a resource in REST */
struct resource_s {
  struct resource_s *next;        /* for LIST, points to next resource defined */
  const char *url;                /*handled URL */
  rest_resource_flags_t flags;    /* handled RESTful methods */
  const char *attributes;         /* link-format attributes */
  restful_handler get_handler;    /* handler function */
  restful_handler post_handler;   /* handler function */
  restful_handler put_handler;    /* handler function */
  restful_handler delete_handler; /* handler function */
  union {
    struct periodic_resource_s *periodic; /* special data depending on flags */
    restful_trigger_handler trigger;
    restful_trigger_handler resume;
  };
};
typedef struct resource_s resource_t;

#endif /* CORE_SRC_ERBIUM_ER_RESOURCE_H_ */
