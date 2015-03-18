#ifndef __CDSP_PREFERENCE_20_H__
#define __CDSP_PREFERENCE_20_H__
#include "mach/gp_cdsp.h"

gp_cdsp_user_preference_t cdsp_user_preference =
{
	.ae_target = 31,
	.ae_target_night = 31,
	.y_scale_day = 43,
	.y_offset_day = 10,
	.y_scale_night = 32,
	.y_offset_night = 2,
	.u_offset_day = 2,
	.v_offset_day = 5,
	.u_offset_night = 0,
	.v_offset_night = 4,
	.u_scale_day = 30,
	.v_scale_day = 30,
	.u_scale_night = 26,
	.v_scale_night = 26,
	.edge_day = 2,
	.edge_night = 0,
	.wb_offset_day = 1,
	.wb_offset_night = 0,
	.max_lum = 64-15,
};
#endif