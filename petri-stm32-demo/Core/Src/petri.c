#include "petri.h"

#include "cJSON.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int empty_action_false(void *data)
{
    return -1;
}

int empty_action_true(void *data)
{
    return 0;
}

/* PLACES INTERFACE */
pn_place *place_create(void)
{
    pn_place *place = malloc(sizeof(*place));
    place_init(place, "P", 0);
    memalloc += sizeof(*place);
    return place;
}

int place_free(pn_place *place)
{
    if (!place)
        return -1;

    memalloc -= sizeof(*place);
    free(place);
    return 0;
}

void place_set_name(pn_place *place, const char *name)
{
    uint32_t length = strlen(name);
    strncpy(place->name, name, (length < MAX_NAME) ? length + 1 : MAX_NAME - 1);
}

void place_set_tokens(pn_place *place, uint32_t tokens)
{
    place->tokens = tokens;
}

int place_init(pn_place *place, const char *name, uint32_t tokens)
{
    if (!place)
        return -1;

    place_set_name(place, name);
    place_set_tokens(place, tokens);

    return 0;
}

bool place_has_tokens(pn_place *place)
{
    if (!place)
        return false;

    return (place->tokens > 0);
}

uint32_t place_get_tokens(pn_place *place)
{
    if (!place)
        return 0;
    
    return place->tokens;
}

int place_add_tokens(pn_place *place, uint32_t n)
{
    if (!place)
        return -1;
    
    /* check for overflow */
    if (place->tokens + n < place->tokens) {
        place->tokens = UINT32_MAX;
        return -2;
    }

    place->tokens = place->tokens + n;

    return 0;
}

int place_rm_tokens(pn_place *place, uint32_t n)
{
    if (!place)
        return -1;
    
    /* check for overflow */
    if (place->tokens - n > place->tokens) {
        place->tokens = 0;
        return -2;
    }

    place->tokens = place->tokens - n;

    return 0;
}

inline int place_inc_token(pn_place *place)
{
    return place_add_tokens(place, 1);
}

inline int place_dec_token(pn_place *place)
{
    return place_rm_tokens(place, 1);
}

cJSON *place_serialize(pn_place *place)
{
    if (!place)
        return NULL;

    cJSON *jplace = cJSON_CreateObject();
    cJSON_AddStringToObject(jplace, "name", place->name);
    cJSON_AddNumberToObject(jplace, "tokens", place->tokens);

    return jplace;
}

cJSON *place_serialize_name(pn_place *place)
{
    if (!place)
        return NULL;

    cJSON *jplace = cJSON_CreateObject();
    cJSON_AddStringToObject(jplace, "name", place->name);

    return jplace;
}

int place_deserialize(pn_place *place, cJSON *jplace)
{
    if (!place || !jplace)
        return -1;

    /* field is borrowed object, do not free */
    cJSON *field = cJSON_GetObjectItemCaseSensitive(jplace, "name"); 
    /* get fields */
    if (cJSON_IsString(field) && (field->valuestring != NULL)) { 
        place_set_name(place, field->valuestring);
    }
    field = cJSON_GetObjectItemCaseSensitive(jplace, "tokens");
    if (cJSON_IsNumber(field)) { 
        place_set_tokens(place, field->valueint);
    }

    return 0;
}

int place_snprint_name(pn_place *place, char *buf, size_t size)
{
    if (!place)
        return -1;

    snprintf(buf, size, "P-%s", place->name);

    return 0;
}

int place_snprint_tokens(pn_place *place, char *buf, size_t size)
{
    if (!place)
        return -1;

    snprintf(buf, size, "P-%2lu", place->tokens);

    return 0;
}

/* TRANSITIONS INTERFACE */
pn_trans *trans_create(void)
{
    pn_trans *trans = malloc(sizeof(*trans));
    trans_init(trans, "t");
    memalloc += sizeof(*trans);
    return trans;
}

int trans_free(pn_trans *trans)
{
    if (!trans)
        return -1;

    memalloc -= sizeof(*trans);
    free(trans);
    return 0;
}

void trans_set_name(pn_trans *trans, const char *name)
{
    uint32_t length = strlen(name);
    strncpy(trans->name, name, (length < MAX_NAME) ? length + 1 : MAX_NAME - 1);
}

int trans_set_action(pn_trans *trans, int (*action)(void *data), void *data)
{
    if (!trans)
        return -1;

    trans->action = action;
    trans->data = data;

    return 0;
}

int trans_init(pn_trans *trans, const char *name)
{
    if (!trans)
        return -1;

    trans_set_name(trans, name);
    trans_set_action(trans, empty_action_true, NULL);

    trans->in_count = 0;
    trans->out_count = 0;
    for (uint32_t i = 0; i < MAX_PLACES_IN_TRANS; i++) {
        trans->in_places[i] = NULL;
        trans->out_places[i] = NULL;
    }

    return 0;
}

int trans_add_input(pn_trans *trans, pn_place *place)
{
    if (!trans || !place)
        return -1;

    if (trans->in_count >= MAX_PLACES_IN_TRANS)
        return -2;

    trans->in_places[trans->in_count] = place;
    trans->in_count++;

    return 0;
}

int trans_add_output(pn_trans *trans, pn_place *place)
{
    if (!trans || !place)
        return -1;

    if (trans->out_count >= MAX_PLACES_IN_TRANS)
        return -2;

    trans->out_places[trans->out_count] = place;
    trans->out_count++;

    return 0;
}

int trans_rm_input(pn_trans *trans, pn_place *place)
{
    if (!trans || !place)
        return -1;

    if (trans->in_count < 1)
        return -2;

    for (uint32_t i = 0; i < trans->in_count; i++) {
        if (trans->in_places[i] != place) {
            /* loop until find */
            continue;
        } else if (i == trans->in_count - 1) {
            /* remove last */
            trans_rm_last_input(trans);
            return 0;
        }
        /* remove in middle */
        memcpy(&trans->in_places[i], &trans->in_places[i + 1], (trans->in_count - i) * sizeof(pn_place *));
        trans->in_count--;
    }

    return 0;
}

int trans_rm_output(pn_trans *trans, pn_place *place)
{
    if (!trans || !place)
        return -1;

    if (trans->out_count < 1)
        return -2;

    for (uint32_t i = 0; i < trans->out_count; i++) {
        if (trans->out_places[i] != place) {
            /* loop until find */
            continue;
        } else if (i == trans->out_count - 1) {
            /* remove last */
            trans_rm_last_output(trans);
            return 0;
        }
        /* remove in middle */
        memcpy(&trans->out_places[i], &trans->out_places[i + 1], (trans->out_count - i) * sizeof(pn_place *));
        trans->out_count--;
    }

    return 0;
}

int trans_rm_last_input(pn_trans *trans)
{
    if (!trans)
        return -1;

    if (trans->in_count < 1)
        return -2;

    trans->in_places[trans->in_count] = NULL;
    trans->in_count--;

    return 0;
}

int trans_rm_last_output(pn_trans *trans)
{
    if (!trans)
        return -1;

    if (trans->out_count < 1)
        return -2;

    trans->out_places[trans->out_count] = NULL;
    trans->out_count--;

    return 0;
}

int trans_add_place(pn_trans *trans, pn_place *place, pn_arc_t type)
{
    if (!trans || !place)
        return -1;

    switch (type) {
    case ARC_IN:
        trans_add_input(trans, place);
        break;
    case ARC_OUT:
        trans_add_output(trans, place);
        break;
    case ARC_IO:
        trans_add_input(trans, place);
        trans_add_output(trans, place);
        break;

    default:
        return -2;
    }

    return 0;
}

int trans_rm_place(pn_trans *trans, pn_place *place, pn_arc_t type)
{
    if (!trans || !place)
        return -1;

    switch (type) {
    case ARC_IN:
        trans_rm_input(trans, place);
        break;
    case ARC_OUT:
        trans_rm_output(trans, place);
        break;
    case ARC_IO:
        trans_rm_input(trans, place);
        trans_rm_output(trans, place);
        break;

    default:
        return -2;
    }

    return 0;
}

int trans_rm_last_place(pn_trans *trans, pn_arc_t type)
{
    if (!trans)
        return -1;

    switch (type) {
    case ARC_IN:
        trans_rm_last_input(trans);
        break;
    case ARC_OUT:
        trans_rm_last_output(trans);
        break;
    case ARC_IO:
        trans_rm_last_input(trans);
        trans_rm_last_output(trans);
        break;

    default:
        return -2;
    }

    return 0;
}

bool trans_can_fire(pn_trans *trans)
{
    if (!trans)
        return false;

    for (uint32_t i = 0; i < trans->in_count; i++)
        if (!place_has_tokens(trans->in_places[i]))
            return false;

    return true;
}

int trans_fire(pn_trans *trans)
{
    if (!trans)
        return -1;
    
    /* check if inputs are full */
    for (uint32_t i = 0; i < trans->in_count; i++) {
        if (!place_has_tokens(trans->in_places[i]))
            return -2;
    }
    /* do an action */
    if (!trans->action || trans->action(trans->data))
        return -3;

    /* if full then decrement inputs and increment outputs*/
    for (uint32_t i = 0; i < trans->in_count; i++)
        place_dec_token(trans->in_places[i]);

    for (uint32_t i = 0; i < trans->out_count; i++)
        place_inc_token(trans->out_places[i]);

    return 0;
}

int trans_deserialize_with_net(pn_trans *trans, cJSON *jtrans, pnet_t *net)
{
    if (!trans || !jtrans || !net)
        return -1;

    cJSON *jplace = NULL;

    /* field is borrowed object, do not free */
    cJSON *field = cJSON_GetObjectItemCaseSensitive(jtrans, "name"); 
    /* get fields */
    if (cJSON_IsString(field) && (field->valuestring != NULL)) { 
        trans_set_name(trans, field->valuestring);
    }

    /* get all inputs */
    field = cJSON_GetObjectItemCaseSensitive(jtrans, "inputs");
    cJSON_ArrayForEach(jplace, field) {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(jplace, "name");
        pn_place *place = pnet_find_place(net, name->valuestring);

        if (place) {
            trans_add_input(trans, place);
        }
    }
    /* get all outputs */
    field = cJSON_GetObjectItemCaseSensitive(jtrans, "outputs");
    cJSON_ArrayForEach(jplace, field) {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(jplace, "name");
        pn_place *place = pnet_find_place(net, name->valuestring);

        if (place) {
            trans_add_output(trans, place);
        }
    }

    return 0;
}

int trans_snprint_name(pn_trans *trans, char *buf, size_t size)
{
    if (!trans)
        return -1;

    snprintf(buf, size, "T-%s", trans->name);

    return 0;
}

int trans_snprint_arccount(pn_trans *trans, char *buf, size_t size)
{
    if (!trans)
        return -1;

    snprintf(buf, size, "T-%2lu->%2lu", trans->in_count, trans->out_count);

    return 0;
}

/* NET INTERFACE */
pnet_t *pnet_create(void)
{
    pnet_t *net = malloc(sizeof(*net));
    pnet_init(net, "");
    memalloc += sizeof(*net);
    return net;
}

int pnet_free(pnet_t *net)
{
    if (!net)
        return -1;
    
    for (uint32_t i = 0; i < net->place_count; i++)
        place_free(net->places[i]);
    
    for (uint32_t i = 0; i < net->trans_count; i++)
        trans_free(net->trans[i]);

    memalloc -= sizeof(*net);
    free(net);

    return 0;
}

void pnet_set_name(pnet_t *net, const char *name)
{
    uint32_t length = strlen(name);
    strncpy(net->name, name, (length < MAX_NAME) ? length + 1 : MAX_NAME - 1);
}

int pnet_init(pnet_t *net, const char *name)
{
    if (!net)
        return -1;

    // strncpy(net->name, name, strlen(name) + 1);
    pnet_set_name(net, name);

    net->place_count = 0;
    net->trans_count = 0;
    for (uint32_t i = 0; i < MAX_PLACES; i++)
        net->places[i] = NULL;

    for (uint32_t i = 0; i < MAX_TRANSITIONS; i++)
        net->trans[i] = NULL;

    return 0;
}

int pnet_add_place(pnet_t *net, pn_place *place)
{
    if (!net || !place)
        return -1;

    if (net->place_count >= MAX_PLACES)
        return -2;

    net->places[net->place_count] = place;
    net->place_count++;

    return 0;
}

int pnet_add_trans(pnet_t *net, pn_trans *trans)
{
    if (!net || !trans)
        return -1;

    if (net->trans_count >= MAX_TRANSITIONS)
        return -2;

    net->trans[net->trans_count] = trans;
    net->trans_count++;

    return 0;
}

int pnet_rm_place(pnet_t *net, pn_place *place)
{
    if (!net || !place)
        return -1;

    if (net->place_count < 1)
        return -2;

    for (uint32_t i = 0; i < net->place_count; i++) {
        if (net->places[i] != place) {
            /* loop until find */
            continue;
        } else if (i == net->place_count - 1) {
            /* remove last */
            pnet_rm_last_place(net);
            return 0;
        }
        /* remove in middle */
        memcpy(&net->places[i], &net->places[i + 1], (net->place_count - i) * sizeof(pn_place *));
        net->place_count--;
    }

    return 0;
}

int pnet_rm_trans(pnet_t *net, pn_trans *trans)
{
    if (!net || !trans)
        return -1;

    if (net->trans_count < 1)
        return -2;

    for (uint32_t i = 0; i < net->trans_count; i++) {
        if (net->trans[i] != trans) {
            /* loop until find */
            continue;
        } else if (i == net->trans_count - 1) {
            /* remove last */
            pnet_rm_last_trans(net);
            return 0;
        }
        /* remove in middle */
        memcpy(&net->trans[i], &net->trans[i + 1], (net->trans_count - i) * sizeof(pn_trans *));
        net->trans_count--;
    }

    return 0;
}

int pnet_rm_last_place(pnet_t *net)
{
    if (!net)
        return -1;

    if (net->place_count < 1)
        return -2;

    net->places[net->place_count] = NULL;
    net->place_count--;

    return 0;
}

int pnet_rm_last_trans(pnet_t *net)
{
    if (!net)
        return -1;

    if (net->trans_count < 1)
        return -2;

    net->trans[net->trans_count] = NULL;
    net->trans_count--;

    return 0;
}

pn_place *pnet_find_place(pnet_t *net, const char *name)
{
    pn_place *ret = NULL;
    if (!net)
        return ret;

    for (uint32_t i = 0; i < net->place_count; i++) {
        if (!strncmp(net->places[i]->name, name, MAX_NAME)) {
            ret = net->places[i];
            break;
        }
    }

    return ret;
}

pn_trans *pnet_find_trans(pnet_t *net, const char *name)
{
    pn_trans *ret = NULL;
    if (!net)
        return ret;

    for (uint32_t i = 0; i < net->trans_count; i++) {
        if (!strncmp(net->trans[i]->name, name, MAX_NAME)) {
            ret = net->trans[i];
            break;
        }
    }

    return ret;
}

int pnet_fire_sequence(pnet_t *net)
{
    if (!net)
        return -1;

    for (uint32_t i = 0; i < net->trans_count; i++)
        trans_fire(net->trans[i]);

    return 0;
}

int pnet_fire_random(pnet_t *net)
{
    if (!net)
        return -1;

    uint32_t idx = rand() % net->trans_count;
    trans_fire(net->trans[idx]);

    return 0;
}

int pnet_fire_available(pnet_t *net)
{
    if (!net)
        return -1;

    pn_trans *atrans[MAX_TRANSITIONS];
    uint32_t acount = 0;

    for (uint32_t i = 0; i < net->trans_count; i++) {
        if (trans_can_fire(net->trans[i])) {
            atrans[acount] = net->trans[i];
            acount++;
        }
    }

    for (uint32_t i = 0; i < acount; i++)
        trans_fire(atrans[i]);

    return 0;
}

int pnet_deserialize(pnet_t *net, char *jstr)
{
    int ret = 0;
    if (!net) {
        ret = -1;
        return ret;
    }

    /* try to parse json data */
    cJSON *jnet = cJSON_Parse(jstr);
    if (!jnet) { 
        ret = -2;
        goto end_full;
    }

    /* deserialize all places */
    cJSON *jfields = cJSON_GetObjectItemCaseSensitive(jnet, "places"); 
    if (!cJSON_IsArray(jfields)) {
    	ret = -3;
        goto end_full;
    }

        /* loop through places */
    cJSON *field = NULL;
    cJSON_ArrayForEach(field, jfields) {
        pn_place *place = place_create();
        place_deserialize(place, field);
        pnet_add_place(net, place);
    }

    /* deserialize all transitions */
    jfields = cJSON_GetObjectItemCaseSensitive(jnet, "transitions"); 
    if (!cJSON_IsArray(jfields))
        goto end_full;
    
        /* loop through transitions */
    field = NULL;
    cJSON_ArrayForEach(field, jfields) {
        pn_trans *trans = trans_create();
        trans_deserialize_with_net(trans, field, net);
        pnet_add_trans(net, trans);
    }

end_full:
    cJSON_Delete(jnet);

    return ret;
}

int pnet_dump_state_as_str(pnet_t *net, char *str)
{
    if (!net || !str)
        return -1;

    memset(str, 0, MAX_PLACES);
    for (uint32_t i = 0; i < net->place_count; i++) {
        str[i] = "0123456789"[net->places[i]->tokens % 10];
    }
    return 0;
}

