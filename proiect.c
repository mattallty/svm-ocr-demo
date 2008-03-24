/* Copyright (c) 2007, Cristi Măgherușan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the names of its contributors may
 *       be used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include<gtk/gtk.h>
#include<glib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
GtkWidget *w=NULL;
int verbose=0;
static GtkWidget *window = NULL;
GtkWidget *draw_area=NULL;
/* Pixmap for scribble area, to store current scribbles */
static GdkPixmap *pixmap = NULL;
static GdkPixbuf *pixbuf = NULL;

/* Create a new pixmap of the appropriate size to store our scribbles */

GtkWidget *centreaza_but,*init_learn_but, *entry;
GtkWidget *adauga_but,*recunoaste_but,*iesire_but;



int min(int a,int b){
        if(a<b) return a;
        return b;
}
int max(int a,int b){
        if(a>b) return a;
        return b;
}
int abs(int x){
        if(x<0) return -x;
        return x;
}


int last_x,last_y,desenez=0;
int left=1000,right=0,top=1000,bottom=0;

        static gboolean
scribble_configure_event (GtkWidget *widget,
                GdkEventConfigure *event,
                gpointer	     draw_area)
{
        if (pixmap)
                g_object_unref (pixmap);

        pixmap = gdk_pixmap_new (widget->window,
                        widget->allocation.width,
                        widget->allocation.height,
                        -1);
        /* Initialize the pixmap to white */
        gdk_draw_rectangle (pixmap,
                        widget->style->white_gc,
                        TRUE,
                        0, 0,
                        widget->allocation.width,
                        widget->allocation.height);
        /* We've handled the configure event, no need for further processing. */
        return TRUE;
}

/* Redraw the screen from the pixmap */
        static gboolean
scribble_expose_event (GtkWidget *widget,
                GdkEventExpose *event,
                gpointer draw_area)
{
        /* We use the "foreground GC" for the widget since it already exists,
         * but honestly any GC would work. The only thing to worry about
         * is whether the GC has an inappropriate clip region set.
         */

        gdk_draw_drawable (widget->window,
                        widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                        pixmap,
                        /* Only copy the area that was exposed. */
                        event->area.x, event->area.y,
                        event->area.x, event->area.y,
                        event->area.width, event->area.height);

        return FALSE;
}

/* Draw on the screen */
        static void
draw (GtkWidget *widget,
                gint    x,
                gint    y)
{
        GdkGC *gc= widget->style->black_gc;
        gdk_gc_set_line_attributes (gc,2,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);


        gdk_draw_line (pixmap, gc,
                        last_x,
                        last_y,
                        x,
                        y);

        GdkRectangle update_rect;
        update_rect.x = min(last_x,x)-3 ;
        update_rect.y = min(last_y,y)-3 ;
        update_rect.width =abs(last_x-x)+6 ;
        update_rect.height=abs(last_y-y)+6;



        left=min(last_x,left);
        right=max(last_x,right);

        top=min(last_y,top);
        bottom=max(last_y,bottom);



        if(left<0) left=0;
        if(top<0)top=0;
        if(bottom>200)bottom=200;
        if(right>120)right=120;

        /*g_print("x=%d, y=%d\n",last_x,last_y);*/

        gdk_window_invalidate_rect (widget->window,
                        &update_rect,
                        FALSE);
}

/*
   GdkPixbuf * get_pixbuf_from_pixmap(GtkWidget *widget)
   {
   return gdk_pixbuf_get_from_drawable  (NULL,pixmap,
   NULL,0,0,0,0, widget->allocation.width,
   widget->allocation.height);
   }
   void set_pixmap_from_pixbuf(GtkWidget *widget,GdkPixbuf * pixbuf )
   {
   gdk_draw_pixbuf(pixmap,NULL, pixbuf,0,0,0,0,-1,-1,GDK_RGB_DITHER_NONE,0,0);
   }
   */


void image_convert(int flag,char* file)
{ FILE* f ;
        int i,j;
        guchar *pixels;
        float val;
        pixels=gdk_pixbuf_get_pixels (pixbuf);
        int wt=gdk_pixbuf_get_width (pixbuf);

        int ht=gdk_pixbuf_get_height (pixbuf);
        int rowstride=gdk_pixbuf_get_rowstride (pixbuf);
        /*
           p1 p2 
           p3 p4
           */

        const gchar * entry_text = gtk_entry_get_text (GTK_ENTRY (entry));
        if(flag)
        {    if(strlen(entry_text)==0 || (entry_text[0]!='0'&&entry_text[0]!='1') )
                {
                        GtkWidget*dialog =gtk_message_dialog_new_with_markup (GTK_WINDOW(window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_OK,
                                        "Introduce The drawn number (0 or 1)\ninside the text entry!!! ");
                        gtk_dialog_run (GTK_DIALOG (dialog));
                        gtk_widget_destroy (dialog);

                        return;
                }

                else
                {

                        f= fopen(file,"a");
                        if(entry_text[0]=='0')
                                fprintf(f,"%d ",-1);
                        else 
                                fprintf(f,"%d ",1);

                }
        }
        else
        {
                f = fopen(file,"w");
                fprintf(f,"%d ",0);
        }

        for(i=0;i<ht;i+=2)
                for(j=0;j<rowstride;j+=6)
                {
                        val=0.0;
                        if(pixels[ i * rowstride +j +0]==0)     val+=0.25;  /* p1 is black */
                        if(pixels[ i * rowstride +j +3]==0)     val+=0.25;  /* p2 is black */
                        if(pixels[ (i+1) * rowstride +j +0]==0) val+=0.25;  /* p3 is black */
                        if(pixels[ (i+1) * rowstride +j +3]==0) val+=0.25;  /* p4 is black */

                        if(val>0.1)
                        {
                                fprintf(f,"%d:%0.2f ",(i/2)*wt+(j/6),val);
                                /*printf("%d:%0.2f ",(i/2)*wt+(j/6),val);*/
                        }

                }

        /* printf("i=%d ,j=%d\n",i,j);*/
        /* printf("# %s\n", entry_text);*/

        if(flag)
        {
                fprintf(f,"# %s\n", entry_text);
                if(verbose){
                        GtkWidget*dialog =gtk_message_dialog_new_with_markup (GTK_WINDOW(window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_INFO,
                                        GTK_BUTTONS_OK,
                                        "The number was learned successfully \nin the database");
                        gtk_dialog_run (GTK_DIALOG (dialog));
                        gtk_widget_destroy (dialog);
                        verbose=0;
                }
        }
        else 
                fprintf(f,"\n");
        fclose(f);
}

void prepare_recognition(void)
{
        system("svm/svm_learn data/train.dat data/model");
}


void crop_image()
{g_print("left:%d right:%d top:%d bottom:%d\n",left,right,top, bottom);
        left-=3;
        right+=3;
        top-=3;
        bottom+=3;
        if(left<0) left=0;
        if(top<0)top=0;
        if(bottom>200)bottom=200;
        if(right>120)right=120;

        GdkGC *gc =  gdk_gc_new(pixmap);
        gdk_gc_set_function(gc, GDK_COPY);

        gdk_draw_drawable(pixmap,gc, pixmap,left,top,0,0,right-left,bottom-top);

        GdkPixbuf * pb= gdk_pixbuf_get_from_drawable  (NULL,pixmap, NULL,0,0,0,0,right-left,bottom-top);

        pb= gdk_pixbuf_scale_simple (pb,120,200,GDK_INTERP_HYPER);

        //gdk_pixbuf_save (pb, "outpb.jpg","jpeg", NULL,"quality", "100", NULL );


        //gdk_pixbuf_save (gdk_pixbuf_get_from_drawable  (NULL,pixmap,NULL,left,top,0,0,right-left,bottom-top), "out.jpg","jpeg", NULL,"quality", "100", NULL );

        gdk_draw_pixbuf(pixmap,NULL, pb,0,0,0,0,-1,-1,GDK_RGB_DITHER_NONE,0,0);



        pixbuf=pb;
        GdkRectangle update_rect;
        update_rect.x = 0;
        update_rect.y = 0 ;
        update_rect.width =120;
        update_rect.height=200;

        gdk_window_invalidate_rect (draw_area->window,&update_rect, FALSE);
        //    gdk_window_invalidate_rect (widget->window,&update_rect, FALSE);


}




void recognize_image(void)
{

        crop_image();
        image_convert(0,"data/test.dat");
        system("svm/svm_classify data/test.dat data/model data/predictions");
        float x;
        int rez;
        FILE* f= fopen("data/predictions","r");
        fscanf(f,"%f",&x);
        fclose(f);

        printf("The read value is %f\n",x);
        if(x>0) 
                rez=1;
        else 
                rez=0;

        GtkWidget*dialog =gtk_message_dialog_new_with_markup (GTK_WINDOW(window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_INFO,
                        GTK_BUTTONS_OK,
                        "The number was %d",rez);
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);

} 



void learn_image(void){
        crop_image();
        image_convert(1,"data/train.dat");
}





void restart_draw (GtkWidget *widget)
{
        GdkRectangle update_rect;
        pixmap = gdk_pixmap_new (widget->window,
                        widget->allocation.width,
                        widget->allocation.height,
                        -1);
        /* Initialize the pixmap to white */
        gdk_draw_rectangle (pixmap,
                        widget->style->white_gc,
                        TRUE,
                        0, 0,
                        widget->allocation.width,
                        widget->allocation.height);
        update_rect.x =0 ;
        update_rect.y = 0 ;
        update_rect.width = w->allocation.width;
        update_rect.height= w->allocation.height;



        left=1000,right=0,top=1000,bottom=0;

        gdk_window_invalidate_rect (w->window,
                        &update_rect,
                        TRUE);
}





        static gboolean
scribble_button_press_event (GtkWidget	    *widget,
                GdkEventButton *event,
                gpointer	     draw_area)
{
        if (pixmap == NULL)
                return FALSE; /* paranoia check, in case we haven't gotten a configure event */

        if (event->button == 1)
        { if (event->type ==GDK_BUTTON_PRESS )
                {
                        desenez=1;
                        last_x=event->x;
                        last_y=event->y; 
                }
                if (event->type ==GDK_BUTTON_RELEASE)
                        desenez=0;
        }

        if ((event->button == 3)&&(event->type ==GDK_BUTTON_PRESS ))
        {
                last_x=-1;
                last_y=-1;
                restart_draw(widget);

        }
        /* We've handled the event, stop processing */
        return TRUE;
}


        static gboolean
scribble_motion_notify_event (GtkWidget	     *widget,
                GdkEventMotion *event,
                gpointer	      draw_area)
{
        int x, y;
        GdkModifierType state;

        if (pixmap == NULL)
                return FALSE; 
        gdk_window_get_pointer (event->window, &x, &y, &state);

        if ((state & GDK_BUTTON1_MASK) && desenez)
        {
                if((last_x!=-1)&&(last_y!=-1))
                        draw (widget, x, y);
                last_x=x;
                last_y=y;
        }
        /* We've handled it, stop processing */
        return TRUE;
}


        static void
close_window (void)
{
        window = NULL;

        if (pixmap)
                g_object_unref (pixmap);
        pixmap = NULL;
}

        GtkWidget *
do_drawingarea (GtkWidget *do_widget)
{
        GtkWidget *frame,*frame2;
        GtkWidget *vbox,*vbox2,*vbox3,*hbox,*hbox2;
        GtkWidget *label,*label2;

        if (!window)
        {
                window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
                gtk_window_set_screen (GTK_WINDOW (window),
                                gtk_widget_get_screen (do_widget));
                gtk_window_set_title (GTK_WINDOW (window), "LibSVM-OCR-Demo");

                g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);



                gtk_container_set_border_width (GTK_CONTAINER (window),4);
                hbox = gtk_hbox_new (FALSE, 4);
                gtk_container_add (GTK_CONTAINER (window), hbox);
                vbox = gtk_vbox_new (FALSE, 4);
                gtk_container_add (GTK_CONTAINER (hbox), vbox);
                gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);


                label = gtk_label_new (NULL);
                gtk_label_set_markup (GTK_LABEL (label),
                                "<b>Draw '0' or '1'</b>\n(RMB erases)");
                gtk_label_set_justify(GTK_LABEL (label),GTK_JUSTIFY_CENTER);

                gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

                frame = gtk_frame_new (NULL); 
                gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
                gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

                draw_area= gtk_drawing_area_new ();
                /* set a minimum size */
                gtk_widget_set_size_request (draw_area, 120, 200);
                gtk_container_add (GTK_CONTAINER (frame), draw_area);

                vbox2 = gtk_vbox_new (FALSE, 4);
                gtk_container_add (GTK_CONTAINER (hbox), vbox2);
                gtk_container_set_border_width (GTK_CONTAINER (vbox2), 4);


                frame2 = gtk_frame_new (NULL);
                gtk_frame_set_shadow_type (GTK_FRAME (vbox2), GTK_SHADOW_IN);
                gtk_box_pack_start (GTK_BOX (vbox2), frame2, TRUE, TRUE, 0);
                gtk_frame_set_label (GTK_FRAME (frame2),"Add");

                vbox3 = gtk_vbox_new (FALSE, 4);
                gtk_container_add (GTK_CONTAINER (frame2), vbox3);
                gtk_container_set_border_width (GTK_CONTAINER (vbox3), 4);



                hbox2 = gtk_hbox_new (FALSE, 4);
                gtk_container_add (GTK_CONTAINER (vbox3), hbox2);
                gtk_container_set_border_width (GTK_CONTAINER (hbox2), 4);

                label2= gtk_label_new (NULL);
                gtk_label_set_markup (GTK_LABEL (label2),"Character");
                gtk_box_pack_start (GTK_BOX (hbox2), label2, FALSE, FALSE, 0);

                entry= gtk_entry_new ();
                gtk_entry_set_max_length(GTK_ENTRY(entry),1);
                gtk_widget_set_size_request (entry, 20, -1);
                gtk_box_pack_start (GTK_BOX (hbox2), entry, FALSE, FALSE, 0);


                adauga_but=gtk_button_new_with_label("Learn");
                gtk_box_pack_start ( GTK_BOX(vbox3),adauga_but,FALSE,FALSE,0);

                init_learn_but=gtk_button_new_with_label("Process");
                gtk_box_pack_start ( GTK_BOX(vbox2),init_learn_but,FALSE,FALSE,0);


                recunoaste_but=gtk_button_new_with_label("Recognise");
                gtk_box_pack_start ( GTK_BOX(vbox2),recunoaste_but,FALSE,FALSE,0);

                iesire_but=gtk_button_new_with_label("Exit");
                gtk_box_pack_start ( GTK_BOX(vbox2),iesire_but,FALSE,FALSE,0);



                /* Signals used to handle backing pixmap */

                g_signal_connect (draw_area, "expose_event",
                                G_CALLBACK (scribble_expose_event), NULL);
                g_signal_connect (draw_area,"configure_event",
                                G_CALLBACK (scribble_configure_event), NULL);

                /* Event signals */

                g_signal_connect (draw_area, "motion_notify_event",
                                G_CALLBACK (scribble_motion_notify_event), NULL);
                g_signal_connect (draw_area, "button_press_event",
                                G_CALLBACK (scribble_button_press_event), NULL);


                g_signal_connect (iesire_but, "button_press_event",
                                GTK_SIGNAL_FUNC	 (gtk_main_quit), NULL);
                /*
                   g_signal_connect (centreaza_but, "button_press_event",
                   G_CALLBACK (crop_image),NULL);
                   */
                g_signal_connect (adauga_but, "button_press_event",
                                G_CALLBACK (learn_image),NULL);
                g_signal_connect (init_learn_but, "button_press_event",
                                G_CALLBACK (prepare_recognition),NULL);

                g_signal_connect (recunoaste_but, "button_press_event",
                                G_CALLBACK (recognize_image),NULL);



                g_signal_connect(GTK_OBJECT(window),"destroy",
                                GTK_SIGNAL_FUNC(gtk_main_quit),NULL);

                /* Ask to receive events the drawing area doesn't normally
                 * subscribe to, make unresizeable
                 */
                gtk_window_set_resizable (GTK_WINDOW (window),FALSE);


                gtk_widget_set_events (draw_area, gtk_widget_get_events (draw_area)
                                | GDK_LEAVE_NOTIFY_MASK
                                | GDK_BUTTON_PRESS_MASK
                                | GDK_POINTER_MOTION_MASK
                                | GDK_POINTER_MOTION_HINT_MASK);

        }

        if (!GTK_WIDGET_VISIBLE (window))
                gtk_widget_show_all (window);
        else
                gtk_widget_destroy (window);

        return window;
}

int main(int argc,char *argv[]){

        gtk_init(&argc,&argv);
        w=do_drawingarea(NULL);


        gtk_widget_show_all(w);
        gtk_main();
        return 0;

}
