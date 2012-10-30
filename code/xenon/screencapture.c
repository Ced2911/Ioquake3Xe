#include <libpng15/png.h>
#include <sys/time.h>
#include <time/time.h>
#include <byteswap.h>
#include <malloc.h>
#include <debug.h>
#include <xenos/xe.h>

extern struct XenosDevice *xe;

struct file_buffer_t {
	long offset;
	unsigned char * data;
};

static unsigned int * untiled_fb = NULL;

static void initBuffer() {
	untiled_fb = (unsigned int*)malloc(10 * 1024 * 1024);
}
static void png_mem_write(png_structp png_ptr, png_bytep data, png_size_t length) {
	struct file_buffer_t *dst = (struct file_buffer_t *) png_get_io_ptr(png_ptr);
	/* Copy data from image buffer */
	memcpy(dst->data + dst->offset, data, length);
	/* Advance in the file */
	dst->offset += length;
}

void screenCapture(unsigned char *dest, int * pnglen) {
    struct XenosSurface * framebuffer = Xe_GetFramebufferSurface(xe);
    volatile unsigned int *screen = (unsigned int*)(framebuffer->base);  
    int width = framebuffer->width;
    int height = framebuffer->height;
	struct file_buffer_t *file;
	png_structp png_ptr_w;
	png_infop info_ptr_w;
	png_bytepp row_pointers = NULL;
	void * data = malloc(sizeof(int)*width*height);
	
	png_ptr_w = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png_ptr_w) {
		printf("[write_png_file] png_create_read_struct failed\n");
		return;
	}
    info_ptr_w = png_create_info_struct(png_ptr_w);
    if (!info_ptr_w) {
		printf("[write_png_file] png_create_info_struct failed\n");
		return;
	}
	
	if (untiled_fb==NULL)
		initBuffer();
	
	file = (struct file_buffer_t *) malloc(sizeof (struct file_buffer_t));
	file->offset = 0;
	file->data = data;
	

    row_pointers = (png_bytepp)malloc(sizeof(png_bytep) * height);

    int y, x;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            unsigned int base = ((((y & ~31) * width) + (x & ~31)*32) +
                    (((x & 3) + ((y & 1) << 2) + ((x & 28) << 1) + ((y & 30) << 5)) ^ ((y & 8) << 2)));
            untiled_fb[y * width + x] = 0xFF | __builtin_bswap32(screen[base] >> 8);
        }
        row_pointers[y] = (untiled_fb + y * width);
    }    

    png_set_write_fn(png_ptr_w, (png_voidp *) file, png_mem_write, NULL);
    png_set_IHDR(png_ptr_w, info_ptr_w, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr_w, info_ptr_w);

	png_write_image(png_ptr_w, row_pointers);
	/*
    png_set_rows(png_ptr_w, info_ptr_w, row_pointers);
    TR
    png_write_png(png_ptr_w, info_ptr_w, PNG_TRANSFORM_IDENTITY, 0);
    */ 
    png_write_end(png_ptr_w, info_ptr_w);
    png_destroy_write_struct(&png_ptr_w, &info_ptr_w);
      
    *pnglen = file->offset;
    
	memcpy(dest, data, sizeof(int) * width * height);
	free(row_pointers);
    free(file);
    free(data);
}
