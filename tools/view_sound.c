#include <gtk/gtk.h> 
#include <gdk/gdkkeysyms.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include "../src/debug.h"
#include "../src/misc.h"


#define DRAW_SELECTION (1<<0)


typedef struct {
    SF_INFO info;
    float *data;
    float *view;
    int view_size;
    int start;//<end
    int end;//<info.frames
    int zoom_start;
    int zoom_end;
    int draw;
} data_t;


//di/vi

int
data_view_process(data_t *data)
{
    float value;
    int vi, di=0;
    int data_size=data->end-data->start;

    if(!data->info.frames)
	return 0;
    value=data->data[data->start];

    for(vi=0; vi<data->view_size; vi++)
	{
	    int c, size;

	    size=(data_size-di)/(1+data->view_size-vi);
	    if(!size)
		{
		    data->view[vi]=value;
		}
	    else
		{
		    data->view[vi]=0;
		    for(c=0; c<size; c++)
			data->view[vi]+=data->data[data->start+di++];
		    data->view[vi]/=size;
		}
	}
    return 0;
}


int
data_view_size(data_t *data, int view_size)
{
    data->view_size=view_size;

    if(data->view)
	free(data->view);
    ck_err(!(data->view = Xalloc(float, view_size)));

    data_view_process(data);

    return 0;
  error:
    return -1;
}


gboolean
expose_event (GtkWidget *widget, GdkEventExpose *event, data_t *data)
{
    GdkGC *gc;
    GdkColor color;
    gint i, w, h;

    w = widget->allocation.width;
    h = widget->allocation.height;

    gc = gdk_gc_new(widget->window);

    gdk_draw_line(widget->window, gc, 0, h/2, w, h/2);
    gdk_draw_line(widget->window, gc, 0, 0, 0, h);

    color.red = 65535;
    color.green = color.blue = 0;

    gdk_colormap_alloc_color(gtk_widget_get_colormap(widget), &color, TRUE, TRUE);
    gdk_gc_set_foreground(gc, &color);

    if(w != data->view_size)
	ck_err(data_view_size(data, w) < 0);

    for(i=1; i<=data->view_size; i++)
	gdk_draw_line(widget->window, gc, i-1, (data->view[i-1]+1)*(float)(h/2), i, (data->view[i]+1)*(float)(h/2));

  error:
    return FALSE;	
}


gboolean
noexpose_event (GtkWidget *widget, GdkEventNoExpose *event, data_t *data)
{
    printf("noexpose\n");

    return FALSE;	
}


int
zoom_selection(data_t *data)
{
    int size;

    size = data->end - data->start;

    if(data->zoom_start<data->zoom_end)
	{
	    data->end   = data->start + (data->zoom_end   * size) / data->view_size;
	    data->start = data->start + (data->zoom_start * size) / data->view_size;
	}
    else
	{
	    data->end   = data->start + (data->zoom_start * size) / data->view_size;
	    data->start = data->start + (data->zoom_end   * size) / data->view_size;
	}

    printf("zoom: %6d-%6d\n", data->zoom_start, data->zoom_end);
    printf("data: %6d-%6d\n", data->start, data->end);

    if(data->end>=data->info.frames)
	data->end=data->info.frames-1;
    if(data->start>data->end)
	data->start=data->end;
    if(data->start<0)
	data->start=0;

    return 0;
}


data_t*
data_create(char *file_name)
{
    SNDFILE *file;
    data_t *data;

    ck_err(!(data=Talloc(data_t)));

    ck_err(!(file=sf_open(file_name, SFM_READ, &data->info)));

    ck_err(!(data->data=Xalloc(float, data->info.frames * data->info.channels)));
    ck_err(sf_readf_float(file, data->data, data->info.frames) != data->info.frames);

    sf_close(file);

    data->end=data->info.frames;

    return data;
  error:
    return 0;
}


static gboolean
key_event(GtkWidget *widget, GdkEventKey *event, data_t *data)
{
    if(event->keyval == GDK_q)
	gtk_main_quit();

    if(event->keyval == GDK_Escape)
	{
	    data->start=0;
	    data->end=data->info.frames-1;
	    ck_err(data_view_process(data) < 0);
	    gtk_widget_queue_draw(widget);
	}

  error:
    return FALSE;
}


static gboolean
button_event(GtkWidget *widget, GdkEventButton *event, data_t *data)
{
    if(event->type==GDK_BUTTON_PRESS && event->button==1)
	{
	    data->zoom_start=event->x;
	    printf("button event: data->zoom_start: %d\n", data->zoom_start);
	}
    if(event->type==GDK_BUTTON_RELEASE && event->button==1)
	{
	    printf("button event: data->zoom_end: %d\n", data->zoom_end);
	    zoom_selection(data);
	    ck_err(data_view_process(data) < 0);
	    gtk_widget_queue_draw(widget);
	}

  error:
    return FALSE;
}


static gboolean
motion_event(GtkWidget *widget, GdkEventMotion *event, data_t *data)
{
    data->zoom_end=event->x;
    gtk_widget_queue_draw(widget);

    return FALSE;
}


int main(int argc,char **argv)
{
    GtkWidget *window, *drawing_area, *vbox;//, *label;
    data_t *data;
    ck_err(argc!=2);

    gtk_init(&argc,&argv);

    data = data_create(argv[1]);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL); 
    gtk_window_set_title(GTK_WINDOW(window), "Sound Viewer"); 
    g_signal_connect(G_OBJECT(window), "destroy_event",
		     G_CALLBACK(gtk_main_quit), NULL); 
    g_signal_connect(G_OBJECT(window), "key_release_event",
		     G_CALLBACK(key_event), data); 
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox); 
    //	label = gtk_label_new(argv[1]);
    //	gtk_box_pack_start_defaults(GTK_BOX(vbox), label);
    drawing_area = gtk_drawing_area_new();
    gtk_box_pack_start_defaults(GTK_BOX(vbox), GTK_WIDGET(drawing_area));
    gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_BUTTON1_MOTION_MASK);
    g_signal_connect(G_OBJECT(drawing_area), "expose_event",
		     G_CALLBACK (expose_event), data);
    g_signal_connect(G_OBJECT(drawing_area), "no_expose_event",
		     G_CALLBACK (noexpose_event), data);
    g_signal_connect(G_OBJECT(drawing_area), "button_press_event",
		     G_CALLBACK(button_event), data); 
    g_signal_connect(G_OBJECT(drawing_area), "button_release_event",
		     G_CALLBACK(button_event), data); 
    g_signal_connect(G_OBJECT(drawing_area), "motion_notify_event",
		     G_CALLBACK(motion_event), data); 
    gtk_widget_show_all(window); 
    gtk_main();

    return 0;
  error:
    return -1;
}
