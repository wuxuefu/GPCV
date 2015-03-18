#ifndef __CDSP_PREFERENCE_MJ2520B_H__
#define __CDSP_PREFERENCE_MJ2520_H__
#include "mach/gp_cdsp.h"
#if 1
gp_cdsp_user_preference_t cdsp_user_preference =
{
	.ae_target = 57,
	.ae_target_night = 57,
	.y_scale_day = 47,
	.y_offset_day = 36,
	.y_scale_night = 32,
	.y_offset_night = 2,
	.u_offset_day = 2,//1,
	.v_offset_day = 3,//3,
	.u_offset_night = 0,
	.v_offset_night = 4,
	.u_scale_day = 27,//35,
	.v_scale_day = 30,//36,
	.u_scale_night = 26,
	.v_scale_night = 26,
	.edge_day = 4,
	.edge_night = 0,
	.wb_offset_day = 64,//1,
	.wb_offset_night = 0,
	.max_lum = 64-15,
};
#else
gp_cdsp_user_preference_t cdsp_user_preference =
{
	.ae_target = 62,
	.ae_target_night = 62,
	.y_scale_day = 43,
	.y_offset_day = 10,
	.y_scale_night = 32,
	.y_offset_night = 2,
	.u_offset_day = 0,//1,
	.v_offset_day = 2,//3,
	.u_offset_night = 0,
	.v_offset_night = 4,
	.u_scale_day = 35,//35,
	.v_scale_day = 35,//36,
	.u_scale_night = 26,
	.v_scale_night = 26,
	.edge_day = 2,
	.edge_night = 0,
	.wb_offset_day = 1,//1,
	.wb_offset_night = 0,
	.max_lum = 64-15,
};
#endif
#endif
