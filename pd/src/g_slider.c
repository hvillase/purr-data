// g_slider.c
// written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001 as hdial/vdial.
// thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja.
// Copyright (c) 2014 by Mathieu Bouchard. (rewrite)
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt", in this distribution.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "g_all_guis.h"
#include <math.h>

extern int gfxstub_haveproperties(void *key);
t_widgetbehavior slider_widgetbehavior;
t_class *hslider_class;
t_class *vslider_class;

static void slider_draw_update(t_gobj *client, t_glist *glist)
{
    t_slider *x = (t_slider *)client;
    if (!x->x_gui.x_changed) return;
    x->x_gui.x_changed = 0;
    if (!glist_isvisible(glist)) return;
    t_canvas *canvas=glist_getcanvas(glist);
    int x1=text_xpix(&x->x_gui.x_obj, glist), x2=x1+x->x_gui.x_w;
    int y1=text_ypix(&x->x_gui.x_obj, glist), y2=y1+x->x_gui.x_h;
    if (x->x_orient) y2+=5; else x2+=5;
    int r;
    if (x->x_orient) {
        r=y2-3 - (x->x_val+50)/100;
        sys_vgui(".x%lx.c coords %lxKNOB %d %d %d %d\n",
            canvas, x, x1+2, r, x2-2, r);
        char tagbuf[MAXPDSTRING];
        sprintf(tagbuf, "x%lx", (long unsigned int)x);
        gui_vmess("gui_slider_update", "ssiiiiii",
            canvas_string(canvas), tagbuf, x1+2, r, x2-2, r,
            x1, y1);
    } else {
        r=x1+3 + (x->x_val+50)/100;
        sys_vgui(".x%lx.c coords %lxKNOB %d %d %d %d\n",
            canvas, x, r, y1+2, r, y2-2);
        char tagbuf[MAXPDSTRING];
        sprintf(tagbuf, "x%lx", (long unsigned int)x);
        gui_vmess("gui_slider_update", "ssiiiiii",
            canvas_string(canvas), tagbuf, r, y1+2, r, y2-2,
            x1, y1);
    }
    int t = x->x_thick;
    x->x_thick = x->x_val == x->x_center;
    if (t!=x->x_thick)
        sys_vgui(".x%lx.c itemconfigure %lxKNOB -strokewidth %d\n",
            canvas, x, 4*x->x_thick+3);
}

static void slider_draw_new(t_slider *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int x1=text_xpix(&x->x_gui.x_obj, glist), x2=x1+x->x_gui.x_w;
    int y1=text_ypix(&x->x_gui.x_obj, glist), y2=y1+x->x_gui.x_h;
    if (x->x_orient) y2+=5; else x2+=5;
    int r;
    if (x->x_orient) r = y2-3 - (x->x_val + 50)/100;
    else             r = x1+3 + (x->x_val + 50)/100;
    iemgui_base_draw_new(&x->x_gui);
    if (x->x_orient) {
        sys_vgui(".x%lx.c create polyline %d %d %d %d -strokewidth 3 "
            "-stroke #%6.6x -tags {%lxKNOB x%lx text iemgui}\n",
            canvas, x1+2, r, x2-2, r, x->x_gui.x_fcol, x, x);
        char tagbuf[MAXPDSTRING];
        sprintf(tagbuf, "x%lx", (long unsigned int)x);
        char colorbuf[MAXPDSTRING];
        sprintf(colorbuf, "#%6.6x", x->x_gui.x_fcol);
        gui_vmess("gui_create_slider", "sssiiiiii", canvas_string(canvas), tagbuf,
            colorbuf, x1+2, r, x2-2, r, x1, y1);
    } else {
        sys_vgui(".x%lx.c create polyline %d %d %d %d -strokewidth 3 "
            "-stroke #%6.6x -tags {%lxKNOB x%lx text iemgui}\n",
            canvas, r, y1+2, r, y2-2, x->x_gui.x_fcol, x, x);
        char tagbuf[MAXPDSTRING];
        sprintf(tagbuf, "x%lx", (long unsigned int)x);
        char colorbuf[MAXPDSTRING];
        sprintf(colorbuf, "#%6.6x", x->x_gui.x_fcol);
        gui_vmess("gui_create_slider", "sssiiiiii", canvas_string(canvas), tagbuf,
            colorbuf, r, y1+2, r, y2-2, x1, y1);
    }
}

static void slider_draw_move(t_slider *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    if (!glist_isvisible(canvas)) return;
    int x1=text_xpix(&x->x_gui.x_obj, glist), x2=x1+x->x_gui.x_w;
    int y1=text_ypix(&x->x_gui.x_obj, glist), y2=y1+x->x_gui.x_h;
    if (x->x_orient) y2+=5; else x2+=5;
    int r;
    if (x->x_orient) r = y2-3 - (x->x_val + 50)/100;
    else             r = x1+3 + (x->x_val + 50)/100;
    iemgui_base_draw_move(&x->x_gui);
    if (x->x_orient)
        sys_vgui(".x%lx.c coords %lxKNOB %d %d %d %d\n",
            canvas, x, x1+2, r, x2-2, r);
    else
        sys_vgui(".x%lx.c coords %lxKNOB %d %d %d %d\n",
            canvas, x, r, y1+2, r, y2-2);
}

static void slider_draw_config(t_slider *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    iemgui_base_draw_config(&x->x_gui);
    sys_vgui(".x%lx.c itemconfigure %lxKNOB -stroke #%6.6x\n",
        canvas, x, x->x_gui.x_fcol);
}

void slider_check_minmax(t_slider *x, double min, double max);
void slider_check_length(t_slider *x, int w);

static void hslider__clickhook2(t_scalehandle *sh, t_slider *x) {
    double w_change_ratio = (double)(x->x_gui.x_w + sh->h_dragx)
        /(double)x->x_gui.x_w;
    x->x_val = x->x_val * w_change_ratio;
    slider_check_length(x, x->x_gui.x_w + sh->h_dragx);
    x->x_gui.x_h += sh->h_dragy;
    slider_check_minmax(x, x->x_min, x->x_max);
}
static void vslider__clickhook2(t_scalehandle *sh, t_slider *x) {
    double h_change_ratio = (double)(x->x_gui.x_h + sh->h_dragy)
        /(double)x->x_gui.x_h;
    x->x_val = x->x_val * h_change_ratio;
    x->x_gui.x_w += sh->h_dragx;
    slider_check_length(x, x->x_gui.x_h + sh->h_dragy);
    slider_check_minmax(x, x->x_min, x->x_max);
}

static void slider__clickhook(t_scalehandle *sh, int newstate)
{
    if (sh->h_dragon && newstate == 0 && sh->h_scale)
    {
        t_slider *x = (t_slider *)(sh->h_master);
        canvas_apply_setundo(x->x_gui.x_glist, (t_gobj *)x);
        if (sh->h_dragx || sh->h_dragy)
        {
            if (x->x_orient) vslider__clickhook2(sh,x);
            else             hslider__clickhook2(sh,x);
            canvas_dirty(x->x_gui.x_glist, 1);
        }
        if (glist_isvisible(x->x_gui.x_glist))
        {
            slider_draw_move(x, x->x_gui.x_glist);
            scalehandle_unclick_scale(sh);
        }
    }
    iemgui__clickhook3(sh,newstate);
}

static void slider__motionhook(t_scalehandle *sh, t_floatarg f1, t_floatarg f2)
{
    if (sh->h_dragon && sh->h_scale)
    {
        t_slider *x = (t_slider *)(sh->h_master);
        int dx = (int)f1, dy = (int)f2;
        int minx = x->x_orient ? IEM_GUI_MINSIZE : IEM_SL_MINSIZE;
        int miny = x->x_orient ? IEM_SL_MINSIZE : IEM_GUI_MINSIZE;
        
        dx = maxi(dx,minx-x->x_gui.x_w);
        dy = maxi(dy,miny-x->x_gui.x_h);
        sh->h_dragx = dx;
        sh->h_dragy = dy;
        scalehandle_drag_scale(sh);

        int properties = gfxstub_haveproperties((void *)x);
        if (properties)
        {
            int new_w = x->x_gui.x_w + sh->h_dragx;
            int new_h = x->x_gui.x_h + sh->h_dragy;
            properties_set_field_int(properties,"dim.w_ent",new_w);
            properties_set_field_int(properties,"dim.h_ent",new_h);
        }
    }
    scalehandle_dragon_label(sh,f1,f2);
}

void slider_draw(t_slider *x, t_glist *glist, int mode)
{
    if(mode == IEM_GUI_DRAW_MODE_UPDATE)      sys_queuegui(x, glist, slider_draw_update);
    else if(mode == IEM_GUI_DRAW_MODE_MOVE)   slider_draw_move(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_NEW)    slider_draw_new(x, glist);
    else if(mode == IEM_GUI_DRAW_MODE_CONFIG) slider_draw_config(x, glist);
}

static void slider_getrect(t_gobj *z, t_glist *glist,
                            int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_slider *x = (t_slider *)z;

    *xp1 = text_xpix(&x->x_gui.x_obj, glist);
    *yp1 = text_ypix(&x->x_gui.x_obj, glist);
    *xp2 = *xp1 + x->x_gui.x_w;
    *yp2 = *yp1 + x->x_gui.x_h;
    if (x->x_orient) *yp2+=5; else *xp2+=5;
    iemgui_label_getrect(x->x_gui, glist, xp1, yp1, xp2, yp2);
}

static void slider_save(t_gobj *z, t_binbuf *b)
{
    t_slider *x = (t_slider *)z;
    int bflcol[3];
    t_symbol *srl[3];
    iemgui_save(&x->x_gui, srl, bflcol);
    binbuf_addv(b, "ssiisiiffiisssiiiiiiiii", gensym("#X"),gensym("obj"),
        (int)x->x_gui.x_obj.te_xpix, (int)x->x_gui.x_obj.te_ypix,
        gensym(x->x_orient ? "vsl" : "hsl"), x->x_gui.x_w, x->x_gui.x_h,
        (t_float)x->x_min, (t_float)x->x_max,
        x->x_lin0_log1, iem_symargstoint(&x->x_gui),
        srl[0], srl[1], srl[2], x->x_gui.x_ldx, x->x_gui.x_ldy,
        iem_fstyletoint(&x->x_gui), x->x_gui.x_fontsize,
        bflcol[0], bflcol[1], bflcol[2],
        x->x_val, x->x_steady);
    binbuf_addv(b, ";");
}

void slider_check_length(t_slider *x, int w)
{
    if(w < IEM_SL_MINSIZE)
        w = IEM_SL_MINSIZE;
    if (x->x_orient) x->x_gui.x_h = w;
    else             x->x_gui.x_w = w;
    x->x_center = (w-1)*50;
    if(x->x_val > (w-1)*100)
    {
        x->x_pos = (w-1)*100;
        x->x_val = x->x_pos;
    }
    if(x->x_lin0_log1)
        x->x_k = log(x->x_max/x->x_min)/(double)(w-1);
    else
        x->x_k =    (x->x_max-x->x_min)/(double)(w-1);
}

void slider_check_minmax(t_slider *x, double min, double max)
{
    if(x->x_lin0_log1)
    {
        if((min == 0.0)&&(max == 0.0))
            max = 1.0;
        if(max > 0.0)
        {
            if(min <= 0.0) min = 0.01*max;
        }
        else
        {
            if(min >  0.0) max = 0.01*min;
        }
    }
    x->x_min = min;
    x->x_max = max;
    x->x_gui.x_reverse = x->x_min > x->x_max;
    int w = x->x_orient ? x->x_gui.x_h : x->x_gui.x_w;
    if(x->x_lin0_log1)
        x->x_k = log(x->x_max/x->x_min)/(double)(w-1);
    else
        x->x_k =    (x->x_max-x->x_min)/(double)(w-1);
}

static void slider_properties(t_gobj *z, t_glist *owner)
{
    t_slider *x = (t_slider *)z;
    char buf[800];
    t_symbol *srl[3];
    int minx = x->x_orient ? IEM_GUI_MINSIZE : IEM_SL_MINSIZE;
    int miny = x->x_orient ? IEM_SL_MINSIZE : IEM_GUI_MINSIZE;
    iemgui_properties(&x->x_gui, srl);
    sprintf(buf, "pdtk_iemgui_dialog %%s |%csl| \
        --------dimensions(pix)(pix):-------- %d %d width: %d %d height: \
        -----------output-range:----------- %g left: %g right: %g \
        %d lin log %d %d empty %d {%s} {%s} {%s} %d %d %d %d %d %d %d\n",
        x->x_orient ? 'v' : 'h',
        x->x_gui.x_w, minx, x->x_gui.x_h, miny,
        x->x_min, x->x_max, 0.0,/*no_schedule*/
        x->x_lin0_log1, x->x_gui.x_loadinit, x->x_steady, -1,/*no multi, but iem-characteristic*/
        srl[0]->s_name, srl[1]->s_name,
        srl[2]->s_name, x->x_gui.x_ldx, x->x_gui.x_ldy,
        x->x_gui.x_font_style, x->x_gui.x_fontsize,
        0xffffff & x->x_gui.x_bcol, 0xffffff & x->x_gui.x_fcol,
        0xffffff & x->x_gui.x_lcol);
    gfxstub_new(&x->x_gui.x_obj.ob_pd, x, buf);
}

static void slider_bang(t_slider *x)
{
    double out;
    if(x->x_lin0_log1)
        out = x->x_min*exp(x->x_k*(double)(x->x_val)*0.01);
    else {
        if (x->x_is_last_float && x->x_last <= x->x_max &&
            x->x_last >= x->x_min) {
            out = x->x_last;
        }
        else
            out = (double)(x->x_val)*0.01*x->x_k + x->x_min;
    }
    if((out < 1.0e-10)&&(out > -1.0e-10))
        out = 0.0;
    iemgui_out_float(&x->x_gui, 0, 0, out);
}

static void slider_dialog(t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    canvas_apply_setundo(x->x_gui.x_glist, (t_gobj *)x);
    int w = atom_getintarg(0, argc, argv);
    int h = atom_getintarg(1, argc, argv);
    double min = atom_getfloatarg(2, argc, argv);
    double max = atom_getfloatarg(3, argc, argv);
    x->x_lin0_log1 = !!atom_getintarg(4, argc, argv);
    x->x_steady = !!atom_getintarg(17, argc, argv);
    int sr_flags = iemgui_dialog(&x->x_gui, argc, argv);
    if (x->x_orient) {
        x->x_gui.x_w = iemgui_clip_size(w);
        int oldl = x->x_gui.x_h;
        slider_check_length(x, h);
        if (x->x_gui.x_h != oldl)
            x->x_val *= ((double)x->x_gui.x_h/(double)oldl);
    } else {
        x->x_gui.x_h = iemgui_clip_size(h);
        int oldl = x->x_gui.x_w;
        slider_check_length(x, w);
        if (x->x_gui.x_w != oldl)
            x->x_val *= ((double)x->x_gui.x_w/(double)oldl);
    }
    slider_check_minmax(x, min, max);
    iemgui_draw_config(&x->x_gui);
    iemgui_draw_io(&x->x_gui, sr_flags);
    iemgui_shouldvis(&x->x_gui, IEM_GUI_DRAW_MODE_MOVE);
    scalehandle_draw(&x->x_gui);
    scrollbar_update(x->x_gui.x_glist);
}

static void slider_motion(t_slider *x, t_floatarg dx, t_floatarg dy)
{
    x->x_is_last_float = 0;
    int old = x->x_val;
    int d = x->x_orient ? -dy : dx;
    if(!x->x_gui.x_finemoved) d *= 100;
    x->x_pos += d;
    x->x_val = x->x_pos;
    int w = x->x_orient ? x->x_gui.x_h : x->x_gui.x_w;
    if(x->x_val > 100*(w-1))
    {
        x->x_val = 100*(w-1);
        x->x_pos += 50 - (x->x_pos%100);
    }
    if(x->x_val < 0)
    {
        x->x_val = 0;
        x->x_pos -= 50 + (x->x_pos%100);
    }
    if(old != x->x_val)
    {
        x->x_gui.x_changed = 1;
        x->x_gui.x_draw(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
        slider_bang(x);
    }
}

static void slider_click(t_slider *x, t_floatarg xpos, t_floatarg ypos,
    t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    if(!x->x_steady)
        x->x_val = x->x_orient ?
         (int)(100.0 * (x->x_gui.x_h + text_ypix(&x->x_gui.x_obj, x->x_gui.x_glist) - ypos)) :
         (int)(100.0 * (        xpos - text_xpix(&x->x_gui.x_obj, x->x_gui.x_glist)));

    int w = x->x_orient ? x->x_gui.x_h : x->x_gui.x_w;
    x->x_val = mini(maxi(x->x_val,0),100*(w-1));
    if (x->x_pos != x->x_val)
    {
        x->x_pos = x->x_val;
        x->x_gui.x_changed = 1;
        x->x_gui.x_draw(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
    }
    x->x_is_last_float=0; // does anyone know how this works with !steady && rcv==snd ?
    slider_bang(x);
    glist_grab(x->x_gui.x_glist, &x->x_gui.x_obj.te_g,
        (t_glistmotionfn)slider_motion, 0, xpos, ypos);
}

static int slider_newclick(t_gobj *z, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_slider *x = (t_slider *)z;
    if(doit)
    {
        slider_click(x, (t_floatarg)xpix, (t_floatarg)ypix, (t_floatarg)shift,
            0, (t_floatarg)alt);
        x->x_gui.x_finemoved = !!shift;
    }
    return (1);
}

static void slider_set(t_slider *x, t_floatarg f)
{
    double g;
    t_floatarg of=f;
    if(x->x_gui.x_reverse)
        f = maxf(minf(f,x->x_min),x->x_max);
    else
        f = maxf(minf(f,x->x_max),x->x_min);
    if(x->x_lin0_log1)
        g = log(f/x->x_min)/x->x_k;
    else
        g =    (f-x->x_min)/x->x_k;
    x->x_val = (int)(100.0*g + 0.49999);
    if (x->x_pos != x->x_val)
    {
        x->x_pos = x->x_val;
        x->x_gui.x_changed = 1;
        x->x_gui.x_draw(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
    }
}

static void slider_float(t_slider *x, t_floatarg f)
{
    x->x_is_last_float = 1;
    x->x_last = f;
    slider_set(x, f);
    if(x->x_gui.x_put_in2out)
        slider_bang(x);
}

static void slider_size(t_slider *x, t_symbol *s, int ac, t_atom *av)
{
    if (x->x_orient) {
        x->x_gui.x_w = iemgui_clip_size((int)atom_getintarg(0, ac, av));
        if(ac > 1)
            slider_check_length(x, (int)atom_getintarg(1, ac, av));
    } else {
        slider_check_length(x, (int)atom_getintarg(0, ac, av));
        if(ac > 1)
            x->x_gui.x_h = iemgui_clip_size((int)atom_getintarg(1, ac, av));
    }
    iemgui_size(&x->x_gui);
}

static void slider_range(t_slider *x, t_symbol *s, int ac, t_atom *av)
{
    slider_check_minmax(x, (double)atom_getfloatarg(0, ac, av),
                         (double)atom_getfloatarg(1, ac, av));
}

static void slider_log(t_slider *x)
{
    x->x_lin0_log1 = 1;
    slider_check_minmax(x, x->x_min, x->x_max);
}

static void slider_lin(t_slider *x)
{
    x->x_lin0_log1 = 0;
    int w = x->x_orient ? x->x_gui.x_h : x->x_gui.x_w;
    x->x_k = (x->x_max - x->x_min)/(double)(w-1);
}

static void slider_steady(t_slider *x, t_floatarg f)
{
    x->x_steady = (f==0.0)?0:1;
}

static void slider_loadbang(t_slider *x)
{
    if(!sys_noloadbang && x->x_gui.x_loadinit)
    {
        x->x_gui.x_draw(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
        slider_bang(x);
    }
}

static void *slider_new(t_symbol *s, int argc, t_atom *argv)
{
    int orient = s==gensym("vsl") || s==gensym("vslider");
    t_slider *x = (t_slider *)pd_new(orient ? vslider_class : hslider_class);
    x->x_orient = orient;
    int bflcol[]={-262144, -1, -1};
    int lilo=0;
    int w,h,ldx,ldy,fs=10, v=0, steady=1;
    if (orient) {
        w=IEM_GUI_DEFAULTSIZE; h=IEM_SL_DEFAULTSIZE; ldx=0, ldy=-9;
    } else {
        w=IEM_SL_DEFAULTSIZE; h=IEM_GUI_DEFAULTSIZE; ldx=-2; ldy=-8;
    }
    double min=0.0, max=(double)(IEM_SL_DEFAULTSIZE-1);

    iem_inttosymargs(&x->x_gui, 0);
    iem_inttofstyle(&x->x_gui, 0);

    if(((argc == 17)||(argc == 18))&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
       &&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,3)
       &&IS_A_FLOAT(argv,4)&&IS_A_FLOAT(argv,5)
       &&(IS_A_SYMBOL(argv,6)||IS_A_FLOAT(argv,6))
       &&(IS_A_SYMBOL(argv,7)||IS_A_FLOAT(argv,7))
       &&(IS_A_SYMBOL(argv,8)||IS_A_FLOAT(argv,8))
       &&IS_A_FLOAT(argv,9)&&IS_A_FLOAT(argv,10)
       &&IS_A_FLOAT(argv,11)&&IS_A_FLOAT(argv,12)&&IS_A_FLOAT(argv,13)
       &&IS_A_FLOAT(argv,14)&&IS_A_FLOAT(argv,15)&&IS_A_FLOAT(argv,16))
    {
        w = atom_getintarg(0, argc, argv);
        h = atom_getintarg(1, argc, argv);
        min = atom_getfloatarg(2, argc, argv);
        max = atom_getfloatarg(3, argc, argv);
        lilo = !!atom_getintarg(4, argc, argv);
        iem_inttosymargs(&x->x_gui, atom_getintarg(5, argc, argv));
        iemgui_new_getnames(&x->x_gui, 6, argv);
        ldx = atom_getintarg(9, argc, argv);
        ldy = atom_getintarg(10, argc, argv);
        iem_inttofstyle(&x->x_gui, atom_getintarg(11, argc, argv));
        fs = maxi(atom_getintarg(12, argc, argv),4);
        bflcol[0] = (int)atom_getintarg(13, argc, argv);
        bflcol[1] = (int)atom_getintarg(14, argc, argv);
        bflcol[2] = (int)atom_getintarg(15, argc, argv);
        v = atom_getintarg(16, argc, argv);
    }
    else iemgui_new_getnames(&x->x_gui, 6, 0);
    if((argc == 18)&&IS_A_FLOAT(argv,17))
        steady = (int)atom_getintarg(17, argc, argv);
    x->x_gui.x_draw = (t_iemfunptr)slider_draw;
    x->x_is_last_float = 0;
    x->x_last = 0.0;
    x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
    x->x_val = x->x_gui.x_loadinit ? v : 0;
    x->x_pos = x->x_val;
    x->x_lin0_log1 = lilo;
    if(steady != 0) steady = 1;
    x->x_steady = steady;
    if (x->x_gui.x_font_style<0 || x->x_gui.x_font_style>2) x->x_gui.x_font_style=0;
    if(iemgui_has_rcv(&x->x_gui))
        pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
    x->x_gui.x_ldx = ldx;
    x->x_gui.x_ldy = ldy;
    x->x_gui.x_fontsize = fs;
    if (orient) {
        x->x_gui.x_w = iemgui_clip_size(w);
        slider_check_length(x, h);
    } else {
        x->x_gui.x_h = iemgui_clip_size(h);
        slider_check_length(x, w);
    }
    slider_check_minmax(x, min, max);
    iemgui_all_colfromload(&x->x_gui, bflcol);
    x->x_thick = 0;
    iemgui_verify_snd_ne_rcv(&x->x_gui);
    outlet_new(&x->x_gui.x_obj, &s_float);

    x->x_gui. x_handle = scalehandle_new((t_object *)x,x->x_gui.x_glist,1,slider__clickhook,slider__motionhook);
    x->x_gui.x_lhandle = scalehandle_new((t_object *)x,x->x_gui.x_glist,0,slider__clickhook,slider__motionhook);
    x->x_gui.x_obj.te_iemgui = 1;
    x->x_gui.x_changed = 0;

    return (x);
}

static void slider_free(t_slider *x)
{
    if(iemgui_has_rcv(&x->x_gui))
        pd_unbind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
    gfxstub_deleteforkey(x);

    if (x->x_gui. x_handle) scalehandle_free(x->x_gui. x_handle);
    if (x->x_gui.x_lhandle) scalehandle_free(x->x_gui.x_lhandle);
}

void slider_addmethods(t_class *c) {
    class_addmethod(c, (t_method)slider_loadbang,
        gensym("loadbang"), 0);
    class_addmethod(c, (t_method)slider_set,
        gensym("set"), A_FLOAT, 0);
    class_addmethod(c, (t_method)slider_log, gensym("log"), 0);
    class_addmethod(c, (t_method)slider_lin, gensym("lin"), 0);
    class_addmethod(c, (t_method)iemgui_init, gensym("init"), A_FLOAT, 0);
    class_addmethod(c, (t_method)slider_steady, gensym("steady"), A_FLOAT, 0);
    class_addmethod(c, (t_method)slider_range, gensym("range"), A_GIMME, 0);
    class_addbang(c,slider_bang);
    class_addfloat(c,slider_float);
    class_addmethod(c, (t_method)slider_click, gensym("click"),
        A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(c, (t_method)slider_motion, gensym("motion"),
        A_FLOAT, A_FLOAT, 0);
    class_setsavefn(c, slider_save);
    class_addmethod(c, (t_method)slider_size, gensym("size"), A_GIMME, 0);
    class_setwidget(c, &slider_widgetbehavior);
    class_addmethod(c, (t_method)slider_dialog, gensym("dialog"),
        A_GIMME, 0);
    class_setpropertiesfn(c, slider_properties);
}

void g_slider_setup(void)
{
    wb_init(&slider_widgetbehavior,slider_getrect,slider_newclick);

    hslider_class = class_new(gensym("hsl"), (t_newmethod)slider_new,
        (t_method)slider_free, sizeof(t_slider), 0, A_GIMME, 0);
    vslider_class = class_new(gensym("vsl"), (t_newmethod)slider_new,
        (t_method)slider_free, sizeof(t_slider), 0, A_GIMME, 0);
    class_addcreator((t_newmethod)slider_new, gensym("hslider"), A_GIMME, 0);
    class_addcreator((t_newmethod)slider_new, gensym("vslider"), A_GIMME, 0);
 
    slider_addmethods(hslider_class);
    slider_addmethods(vslider_class);
    iemgui_class_addmethods(hslider_class);
    iemgui_class_addmethods(vslider_class);
 
    class_sethelpsymbol(hslider_class, gensym("hslider"));
    class_sethelpsymbol(vslider_class, gensym("vslider"));
}
