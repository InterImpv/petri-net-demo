///*
// * netjson.c
// *
// *  Created on: Dec 4, 2023
// *      Author: krypton
// */
//
//#include "action.h"
//#include "petri.h"
//#include "netjson.h"
//
//#define LT_PLACES 5
//#define LT_TRANS 5
//
//static void create_lights(void)
//{
//	pn_place places[LT_PLACES];
//
//
//	pn_place trans[LT_TRANS];
//}
//
//void pnet_sys_init(pnet_t *sys, uint32_t size)
//{
//	uint32_t offset = 0;
//	uint32_t offsets[MAX_SYSTEMS] = { 0, 5, 11 };
//
//	const char *sys_names[MAX_SYSTEMS] = {
//		"LightSys",
//		"ClimateSys",
//		"FireSys"
//	};
//	const char *lt_places[LT_NAMES] = {
//			"lt.EMPTY",
//			"lt.FULL",
//			"lt.OFF",
//			"lt.ON",
//			"lt.ENABLE"
//	};
//	const char *lt_trans[LT_TRANS] = {
//			"lt.people>0",
//			"lt.people<1",
//			"lt.auto_off",
//			"lt.turn_off",
//			"lt.turn_on"
//	};
//	/* set names */
//	for (uint32_t i = 0; i < MAX_SYSTEMS; i++) {
//		pnet_init(&sys[i], sys_names[i]);
//	}
//	/* temp storage */
//	pn_place *place = NULL;
//	pn_trans *trans = NULL;
//	/* init lights */
//		/* places */
//	for (uint32_t i = 0; i < LT_PLACES; i++) {
//		place = place_create();
//		place_init(place, lt_places[i], 0);
//		pnet_add_place(&sys[0], place);
//	}
//	place_inc_token(pnet_find_place(&sys[0], lt_places[0]));
//	place_inc_token(pnet_find_place(&sys[0], lt_places[2]));
//	place_inc_token(pnet_find_place(&sys[0], lt_places[4]));
//		/* transitions */
//	for (uint32_t i = 0; i < LT_TRANS; i++) {
//		trans = trans_create();
//		trans_init(trans, lt_trans[i]);
//		pnet_add_trans(&sys[0], trans);
//	}
//	trans = pnet_find_trans(&sys[0], lt_trans[0]);
//	place = pnet_find_place(&sys[0], lt_places[0]);
//	trans_add_place()
//	return;
//}
