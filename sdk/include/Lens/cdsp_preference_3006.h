#ifndef __CDSP_PREFERENCE_3006_H__
#define __CDSP_PREFERENCE_3006_H__
#include "mach/gp_cdsp.h"

gp_cdsp_user_preference_t cdsp_user_preference =
{
	.ae_target = 36,
	.ae_target_night = 18,
	.y_scale_day = 32,
	.y_offset_day = -2,	
	.y_scale_night = 34,
	.y_offset_night = -8,
	.u_offset_day = 2,
	.v_offset_day = 0,
	.u_offset_night = 0,
	.v_offset_night = 0,
	.u_scale_day = 40,//28,
	.v_scale_day = 40,//28,
	.u_scale_night = 26,
	.v_scale_night = 26,
	.edge_day = 2,
	.edge_night = 1,
	.wb_offset_day = 0,
	.wb_offset_night = 0,
	.max_lum = 64-16,
};
#endif
