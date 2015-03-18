#ifndef __SCALER_H__
#define __SCALER_H__

int  scale1_act(
	void * input_addr,
	unsigned short input_w,
	unsigned short input_h,
	void * output_addr,
	unsigned short output_w,
	unsigned short output_h,
	unsigned int input_format,
	unsigned int out_format
);

int 
scaler_process_zoom(
	unsigned int input_addr,
	unsigned short input_w,
	unsigned short input_h,
	unsigned int output_addr,
	unsigned short output_w,
	unsigned short output_h,
	int srcformat,
	int dstformat,
	int zoom
);

int 
scaler_process(
	unsigned int input_addr,
	unsigned short input_w,
	unsigned short input_h,
	unsigned int output_addr,
	unsigned short output_w,
	unsigned short output_h,
	int srcformat,
	int dstformat
);
#endif
