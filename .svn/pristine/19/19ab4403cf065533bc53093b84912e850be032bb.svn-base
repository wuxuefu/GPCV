#include <mach/gp_chunkmem.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#include "jpeglib.h"
#include <sys/time.h>
 
#define RGB565(r,g,b) ((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3))

char * soft_decode_jpg(const char *input_filename,int *width,int *height)
{
		struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr jerr;
		FILE *input_file;
		JSAMPARRAY buffer;
		int row_width;

		unsigned char *output_buffer;
		unsigned char *output_buf_end;
		unsigned char *tmp = NULL;
		struct timeval t1,t2;
		gettimeofday(&t1,NULL);
		
		cinfo.err = jpeg_std_error(&jerr);

		if ((input_file = fopen(input_filename, "rb")) == NULL) {
				fprintf(stderr, "can't open %s\n", input_filename);
				return NULL;
		}
		jpeg_create_decompress(&cinfo);

		/* Specify data source for decompression */
		jpeg_stdio_src(&cinfo, input_file);

		/* Read file header, set default decompression parameters */
		(void) jpeg_read_header(&cinfo, TRUE);

		/* Start decompressor */
		(void) jpeg_start_decompress(&cinfo);

		row_width = cinfo.output_width * cinfo.output_components;

		buffer = (*cinfo.mem->alloc_sarray)
				((j_common_ptr) &cinfo, JPOOL_IMAGE, row_width, 1);

	 //   write_bmp_header(&cinfo, output_file);
		printf("cinfo.output_width =%d,cinfo.output_height=%d\n",cinfo.output_width,cinfo.output_height);
	 	*width =cinfo.output_width&~0x07; //alin. 
	 	*height = cinfo.output_height;
		output_buffer = (unsigned char *)malloc(row_width * cinfo.output_height);
		memset(output_buffer, 0, row_width * cinfo.output_height);
		tmp = output_buffer;

		/* Process data */
		while (cinfo.output_scanline < cinfo.output_height) {
				(void) jpeg_read_scanlines(&cinfo, buffer, 1);
				memcpy(tmp, *buffer, row_width);
				tmp += row_width;
		}
		
		output_buf_end = (unsigned char *)gpChunkMemAlloc((*width) * cinfo.output_height*2);
		short * pixe=(short *)output_buf_end;
		//change RGB565
		int i,j;
		for(i=0;i< cinfo.output_height;i++)
			for(j=0;j<*width;j++)
			{
				char r,g,b;
	
				int position = cinfo.output_width*i+j;
				int cut_postion = (*width)*i+j;
				r=output_buffer[3*position];
				g =output_buffer[3*position+1];
				b=output_buffer[3*position+2];
				
				pixe[cut_postion]= RGB565(r,g,b);
				
			}	
	
		free(output_buffer);
		(void) jpeg_finish_decompress(&cinfo);

		jpeg_destroy_decompress(&cinfo);
		
		/* Close files, if we opened them */
		fclose(input_file);
		gettimeofday(&t2,NULL);
		printf("software decode cost time %d\n",(t2.tv_sec-t1.tv_sec)*1000+(t2.tv_usec-t1.tv_usec)/1000);
		return output_buf_end;
}

