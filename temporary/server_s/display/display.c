/*
 * display.c
 *	gtk+图形界面
 *	初始化、画图等等
 */

#include <gtk/gtk.h>
#include <fcntl.h>
#include <errno.h>
#include <memory.h>


#define WIDTH	500
#define HEIGHT	300


/*
 * 私有全局变量
 */
static GtkWidget *window = NULL;
static GtkWidget *image = NULL;

/* 图像缓冲 */
#define IMAGE_SIZE 220000
static char image_buffer[IMAGE_SIZE];
char *image_buffer_addr = image_buffer;
int image_size = 0;


/*
 * DisplayLowLevelInit()
 *
 *	初始化一个500x300的窗口，但并未显示
 *
 *	返回：小于0为失败
 *
 */
int DisplayLowLevelInit(void)
{
	if(window == NULL)
	{
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
		gtk_window_set_title(GTK_WINDOW(window), "camera");
		gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);
		g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

		gtk_widget_show_all(window);
		printf("Show window\n");
	}
}


/*
 * DisplayWriteData
 *
 *	填入图像数据，并未开始画图
 *
 *	输入：
 *		buffer：指向像数据的指针
 *		size：数据大小
 */
void DisplayWriteData(void *buffer, int size)
{
	memcpy(image_buffer_addr, buffer, size);

	/* 累加计数 */
	image_buffer_addr += size;
	image_size += size;

	printf("\tTotally %d\n", image_size);
}


/*
 * DisplayDraw
 *
 *	把一个图像调整大小后，画到window中
 *
 */
void DisplayDraw(void)
{
	static int		sign_show = 0;

	GdkPixbuf		*pixbuf;
	GdkPixbufLoader	*loader;

	
		/*
	if(!sign_show){
		printf("\t\t\tCreate window");
		gtk_widget_show_all(window);
		sign_show = 1;
	}
		*/


	/* load图片，如果压缩过，则解压 */
	loader = gdk_pixbuf_loader_new();
	gdk_pixbuf_loader_write(loader, image_buffer, image_size, 0);
	gdk_pixbuf_loader_close(loader, 0);

	pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

	/* 调整大小 */
	pixbuf = gdk_pixbuf_scale_simple(pixbuf, WIDTH, HEIGHT, GDK_INTERP_HYPER);

	/* 贴图 */
	if(image != NULL)
	{
		printf("\t\t\trefresh..............\n");
		gtk_container_remove(GTK_CONTAINER(window), image);
	}
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add(GTK_CONTAINER(window), image);

	gtk_widget_show(image);

	/* 显示完一张图片后，指针与计数器要复原 */
	image_buffer_addr = image_buffer;
	image_size = 0;
}


