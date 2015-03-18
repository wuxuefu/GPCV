#ifndef __CDSP_PREFERENCE_PW650_H__
#define __CDSP_PREFERENCE_PW650_H__

#include "mach/gp_cdsp.h"
gp_cdsp_user_preference_t cdsp_user_preference = 
{
	.ae_target = 36,//36,
	.ae_target_night = 36,
	.y_scale_day = 33,
	.y_offset_day = -4,
	.y_scale_night = 32,
	.y_offset_night = 4,
	.u_offset_day = -4,
	.v_offset_day = 4,
	.u_offset_night = -4, // -128 ~ +127,   +: more blue,  -: more yellow/green	
	.v_offset_night = 4, //  -128 ~ +127,   +: more red,  -: more blue/green
	.u_scale_day = 24,
	.v_scale_day = 24,
	.u_scale_night = 26, //
	.v_scale_night = 26,
	.edge_day = 2,
	.edge_night = 2,
	.wb_offset_day = 0,
	.wb_offset_night = 0,
	.max_lum = (64 - 15)
};
#endif