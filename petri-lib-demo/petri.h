#ifndef __PETRI_H
#define __PETRI_H

#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_PLACES_IN_TRANS 8u
#define MAX_PLACES 16u
#define MAX_TRANSITIONS 16u
#define MAX_NAME 16u

typedef struct petri_place_int {
    char name[MAX_NAME];
    uint32_t tokens;
} pn_place;

typedef struct petri_transition {
    char name[MAX_NAME];

    int (*action)(void *data);
    void *data;

    pn_place *in_places[MAX_PLACES_IN_TRANS];
    uint32_t in_count;
    pn_place *out_places[MAX_PLACES_IN_TRANS];
    uint32_t out_count;
} pn_trans;

typedef enum ARC_TYPE {
    ARC_IN = 0,
    ARC_OUT = 1,
    ARC_IO = 2
} pn_arc_t;

typedef struct petri_net {
    char name[MAX_NAME];

    pn_place *places[MAX_PLACES];
    uint32_t place_count;
    pn_trans *trans[MAX_TRANSITIONS];
    uint32_t trans_count;
} pnet_t;

pn_place *place_create(void);
int place_free(pn_place *place);
void place_set_name(pn_place *place, const char *name);
void place_set_tokens(pn_place *place, uint32_t tokens);
int place_init(pn_place *place, const char *name, uint32_t tokens);
bool place_has_tokens(pn_place *place);
uint32_t place_get_tokens(pn_place *place);
int place_add_tokens(pn_place *place, uint32_t n);
int place_rm_tokens(pn_place *place, uint32_t n);
int place_inc_token(pn_place *place);
int place_dec_token(pn_place *place);
cJSON *place_serialize(pn_place *place);
cJSON *place_serialize_name(pn_place *place);
int place_deserialize(pn_place *place, cJSON *jplace);
int place_print(pn_place *place);

/* TRANSITIONS INTERFACE */
int empty_action_false(void *data);
int empty_action_true(void *data);

pn_trans *trans_create(void);
int trans_free(pn_trans *trans);
void trans_set_name(pn_trans *trans, const char *name);
int trans_set_action(pn_trans *trans, int (*action)(void *data), void *data);
int trans_init(pn_trans *trans, const char *name, int (*action)(void *data), void *data);
int trans_add_input(pn_trans *trans, pn_place *place);
int trans_add_output(pn_trans *trans, pn_place *place);
int trans_rm_input(pn_trans *trans, pn_place *place);
int trans_rm_output(pn_trans *trans, pn_place *place);
int trans_rm_last_input(pn_trans *trans);
int trans_rm_last_output(pn_trans *trans);
int trans_add_place(pn_trans *trans, pn_place *place, pn_arc_t type);
int trans_rm_place(pn_trans *trans, pn_place *place, pn_arc_t type);
int trans_rm_last_place(pn_trans *trans, pn_arc_t type);
int trans_fire(pn_trans *trans);
cJSON *trans_serialize(pn_trans *trans);
int trans_deserialize_with_net(pn_trans *trans, cJSON *jtrans, pnet_t *net);
int trans_print(pn_trans *trans);

/* NET INTERFACE */
pnet_t *pnet_create(void);
int pnet_free(pnet_t *net);
void pnet_set_name(pnet_t *net, const char *name);
int pnet_init(pnet_t *net, const char *name);
int pnet_add_place(pnet_t *net, pn_place *place);
int pnet_add_trans(pnet_t *net, pn_trans *trans);
int pnet_rm_place(pnet_t *net, pn_place *place);
int pnet_rm_trans(pnet_t *net, pn_trans *trans);
int pnet_rm_last_place(pnet_t *net);
int pnet_rm_last_trans(pnet_t *net);
pn_place *pnet_find_place(pnet_t *net, const char *name);
pn_trans *pnet_find_trans(pnet_t *net, const char *name);
int pnet_fire_sequence(pnet_t *net, FILE *fp);
int pnet_fire_random(pnet_t *net, FILE *fp);
int pnet_fire_available(pnet_t *net, FILE *fp);
int pnet_dump_header_to_file(pnet_t *net, FILE *fp);
int pnet_dump_state_to_file(pnet_t *net, FILE *fp);
int pnet_dump_to_hex(pnet_t *net, FILE *fp);
int pnet_dump_state_as_str(pnet_t *net, char *str);
cJSON *pnet_serialize(pnet_t *net);
int pnet_deserialize_str(pnet_t *net, char *jstr);
int pnet_deserialize(pnet_t *net, const char *filename);
int pnet_print(pnet_t *net);

#endif