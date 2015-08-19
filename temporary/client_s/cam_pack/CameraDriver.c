/* 
 * camera 'dirver'
 *
 */

#include <linux/videodev2.h>
#include <sys/mman.h>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define CAM_GLOBALS
#include "cam_global_var.h"


/* 
 * constant varibles
 */
const int var_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

const char VideoDeviceName[] = "/dev/video0";


/* 
 * private functions
 */
void CameraLowLevelInit(void);


/* 
 * function definition
 */

/* 
 * private function
 * LowLevelInit
 *	print out camera info
 */
void CameraLowLevelInit(void)
{
	struct v4l2_capability		st_camera_cap;
	struct v4l2_fmtdesc			st_support_fmt;
	struct v4l2_format			st_camera_fmt;


	/* initialize global varibles */
	camera_buffer = NULL;
	camera_buffer_size = 0;


	/* Open the camera */
	video_fd = open(VideoDeviceName, O_RDWR);
	if(video_fd < 0){
		fprintf(stderr, "\nFailed to open %s...\n\n", VideoDeviceName);
		exit(-1);
	} else {
		printf("\nOpened %s.\n\n",VideoDeviceName);
	}


	/* list its information */
	ioctl(video_fd, VIDIOC_QUERYCAP, &st_camera_cap);
	printf("Driver Name:\n\t%s\nCard Name:\n\t%s\nBus Info:\n\t%s\nDriver Version:\n\t%u.%u.%u\n\n",
				st_camera_cap.driver,
				st_camera_cap.card,
				st_camera_cap.bus_info,
				0xff & (st_camera_cap.version >> 16),
				0xff & (st_camera_cap.version >> 8 ),
				0xff &  st_camera_cap.version
		  );

	/* list its supported format */
	st_support_fmt.index = 0;
	st_support_fmt.type = var_type;
	printf("Supported format:\n");
	while(-1 != ioctl(video_fd, VIDIOC_ENUM_FMT, &st_support_fmt))
	{
		printf("\t%d: %s\n", 
					++st_support_fmt.index, 
					st_support_fmt.description
			  );
	}


	/* set to MJPEG */
	st_camera_fmt.type = var_type;
	st_camera_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	st_camera_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if(-1 == ioctl(video_fd, VIDIOC_S_FMT, &st_camera_fmt)){
		fprintf(stderr, "\nFailed to set MJPEG...\n\n");
		exit(-2);
	} else {
		printf("\nSet to MJPEG!\n\n");
	}


	/* list current range */
	ioctl(video_fd, VIDIOC_G_FMT, &st_camera_fmt);
	printf("%dX%d\n\n",
				st_camera_fmt.fmt.pix.width,
				st_camera_fmt.fmt.pix.height
		  );
}


/* 
 * interfaces
 * CameraInit
 *	initialize the camera:
 *	- call LowLevelInit
 *	- memory mapping
 */
int CameraInit(void)
{
	struct v4l2_requestbuffers	st_reqbuf;


	CameraLowLevelInit();

	/*
	 * memory mapping!!!
	 */
	/* set buffered capture */
	st_reqbuf.count = 1;
	st_reqbuf.type = var_type;
	st_reqbuf.memory = V4L2_MEMORY_MMAP;
	ioctl(video_fd, VIDIOC_REQBUFS, &st_reqbuf);

	memset(&st_desbuf, 0, sizeof(st_desbuf));

	/* get buffer information, e.g. size, start, etc */
	st_desbuf.type = var_type;
	st_desbuf.memory = V4L2_MEMORY_MMAP;
	st_desbuf.index = 0;
	if(-1 == ioctl(video_fd, VIDIOC_QUERYBUF, &st_desbuf)){
		fprintf(stderr, "Failed to locate image buffer...\n\n");
		return(-3);
	}

	camera_buffer_size = st_desbuf.length;
	camera_buffer = mmap(
				NULL,
				st_desbuf.length,
				PROT_WRITE | PROT_READ,
				MAP_SHARED,
				video_fd,
				st_desbuf.m.offset
				);
	if(MAP_FAILED == camera_buffer){
		fprintf(stderr, "mmap() failed...\n\n");
		return(-4);
	} else {
		printf("mmap() OK.\n\tstart:%X       length:%d\n\n",
					(unsigned int)camera_buffer,
					camera_buffer_size
			  );
	}

	/* query buffers */
	ioctl(video_fd, VIDIOC_QBUF, &st_desbuf);
	/* open data stream... */
	ioctl(video_fd, VIDIOC_STREAMON, &var_type);

	return 0;
}


/* 
 * CameraShut
 *
 *	从摄像头获取一张JPEG图片
 *	数据放到 *camera_buffer 所指处
 *
 */
void CameraShut(void)
{
	ioctl(video_fd, VIDIOC_DQBUF, &st_desbuf);
#ifdef DEBUGGING
	printf("++++++++++++\n");
#endif
	ioctl(video_fd, VIDIOC_QBUF,  &st_desbuf);
}

