/* 
 * global_var.h
 *	define global varibles
 *
 *	Be aware:
 *		define 'CAM_GLOBALS' out of this file
 *
 */
#ifndef _MY_GLOBAL_VARS_
#define _MY_GLOBAL_VARS_


#ifdef CAM_GLOBALS
#define EXT 
#else
#define EXT extern
#endif


/* 
 * varibles are decleared here:
 */
EXT void	*camera_buffer;
EXT int		camera_buffer_size;
EXT int		video_fd;

#include <linux/videodev2.h>
EXT struct v4l2_buffer	st_desbuf;


#endif

