/* ---------------------------------------------------------------------------
** This software is furnished "as is", without technical support,
** and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** main.c
** Contains main declarations and definitions.
**
** Author: Sk. Mohammadul Haque
** Copyright (c) 2014 Sk. Mohammadul Haque
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
** Unless required by applicable law or agreed to in writing, software distributed
** under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied. See the License for the
** specific language governing permissions and limitations under the License.
**
** For more details and updates, visit http://mohammadulhaque.alotspace.com
** -------------------------------------------------------------------------*/

#include "../include/strfuncs.h"
#include "../include/main.h"
#include "../include/program.h"


GtkWidget *window = NULL, *cndlg = NULL;
GtkWidget *button_prog_run = NULL;
GtkWidget *vbox_main = NULL, *hbox_iface = NULL;
GtkWidget *menubar = NULL, *file_menu = NULL, *file_menu_item = NULL, *load_menu_item = NULL, *save_menu_item = NULL, *quit_menu_item = NULL, *settings_menu = NULL, *settings_menu_item = NULL, *save_path_menu_item = NULL, *help_menu = NULL, *help_menu_item = NULL, *about_menu_item = NULL;
GtkWidget *check_rc = NULL, *text_rc = NULL, *edit_rc = NULL, *check_prog = NULL;
GtkWidget *check_rc_cmos = NULL, *text_rc_cmos = NULL, *edit_rc_cmos = NULL;
GtkWidget *check_bright = NULL, *text_bright = NULL, *edit_bright = NULL;

GtkWidget *statusbr = NULL;
GdkGLConfig *glconfig = NULL;
GtkWidget *drawing_area = NULL;

typedef void (*CODE_LOOK_UP_TABLE)();
CODE_LOOK_UP_TABLE code_look_up_table[13] =
{
    NULL, /* 0 not used */
    run_comm_switch_mode_1,
    run_comm_switch_mode_2,
    run_comm_switch_mode_3,
    run_comm_switch_mode_4,
    run_comm_switch_mode_5,
    run_comm_switch_mode_6, /* mode 1-6 */
    run_comm_start, /* 7 start code */
    run_comm_stop, /* 8 stop code */
    run_comm_capture, /* 9 capture code */
    run_comm_save, /* 10 save code */
    run_comm_projector, /* 11 projector code */
    run_comm_end /* 12 loop end code */
};

typedef void (*CODE_LOOK_UP_TABLE_WITH_ARG)(int arg);
CODE_LOOK_UP_TABLE_WITH_ARG code_look_up_table_with_arg[13] =
{
    NULL, /* not used */
    run_comm_pause_with_arg, /* 14 pause code */
    run_comm_brightness_with_arg, /* 15 brightness code */
    run_comm_switch_with_arg, /* 16 switch code */
    run_comm_loop_with_arg, /* 17 loop code */
};

typedef struct program_capture_data
{
    char is_ir;
    char is_rgb;
    char is_depth;
    char is_saved;
    int frame_number;
    int sz_ir[3];
    int sz_rgb[3];
    int sz_depth[3];
    uint8_t format_ir;
    uint8_t format_rgb;
    uint8_t resolution_ir;
    uint8_t resolution_rgb;
    uint8_t *data_ir;
    uint8_t *data_rgb;
    uint8_t *data_depth;
} program_capture_data;
program_capture_data prog_cap_data;

/* required for working */
uint8_t frame_captured = 0;
uint8_t command_entered = 0;
uint8_t command_mode_enable = 0;
uint8_t cmos_command_mode_enable = 0;
uint8_t bright_mode_enable = 0;
uint8_t program_mode_enable = 0;
uint8_t program_running = 0;
uint8_t histogram_enable = 0;
uint8_t check_exposure_ir_enable = 0;
uint8_t depth_changed = 0;
uint8_t depth_running = 0;

int capturing_frame_number = -1;
FILEPOINTER capture_file = NULL;
char file_name[256];
char folder_name[256];

const char hfstring[] = "FNK1";
const char dispstring[8][32] =
{
    "",
    "Mode 1: 640x480 RGB",
    "Mode 2: 640x480 YUV",
    "Mode 3: 640x480 IR 8bit",
    "Mode 4: 1280x1024 RGB",
    "Mode 5: 1280x1024 IR 8bit",
    "Mode 6: 1280x1024 IR 10bit",
    "Mode 7: Programmable Mode"
};
char statusbr_text[128];
const int mode_look_up_table[3][6] =
{
    {0,0,0,0,0,0},
    {1,0,3,0,0,2},
    {4,0,5,6,0,0}
};

int capture_mode = 1;
pthread_t freenect_thread;
volatile int die = 0;
int res = 0, initflag = 0;
long prog_th_id = 0;
double hist_rgb[3][HIST_NUM_BARS], hist_ir[HIST_NUM_BARS];

DTYPE_STACK prog_stck = NULL;
dtype prog_tmp;
DTYPE prog_tmp2 = NULL;
int prog_k;

pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
uint8_t *depth_mid = NULL, *depth_front = NULL, *depth_mid_raw = NULL, *depth_front_raw = NULL;
uint8_t *rgb_back = NULL, *rgb_mid = NULL, *rgb_front = NULL;
uint8_t *exposure_pixels = NULL;
GLuint gl_depth_tex;
GLuint gl_rgb_tex;

freenect_context *f_ctx = NULL;
freenect_device *f_dev = NULL;
int freenect_angle = 0;
int freenect_led;
int nr_devices, user_device_number = 0;

freenect_video_format requested_format = FREENECT_VIDEO_RGB;
freenect_video_format current_format = FREENECT_VIDEO_RGB;
freenect_resolution requested_resolution = FREENECT_RESOLUTION_MEDIUM;
freenect_resolution current_resolution = FREENECT_RESOLUTION_MEDIUM;

pthread_cond_t gl_frame_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t prog_save_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_t th;
uint8_t got_rgb = 0;
uint8_t got_depth = 0;
uint8_t depth_on = 1;
uint8_t projector_on = 1;
uint16_t t_gamma[2048];

int main(int argc, char *argv[])
{

    gtk_init(&argc, &argv);
    gtk_gl_init(&argc, &argv);

    /* main window design */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), (DRAWING_AREA_WIDTH), (DRAWING_AREA_HEIGHT+50));
    gtk_widget_set_size_request(GTK_WIDGET(window), (DRAWING_AREA_WIDTH), (DRAWING_AREA_HEIGHT+50));
    gtk_window_set_title(GTK_WINDOW(window), "KinectCapture");
    vbox_main = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox_main);

    /* menu bar design */
    menubar = gtk_menu_bar_new();

    file_menu = gtk_menu_new();
    file_menu_item = gtk_menu_item_new_with_label("File");

    load_menu_item = gtk_menu_item_new_with_label("Load Program");
    save_menu_item = gtk_menu_item_new_with_label("Save Depth to RGB Registration Data");
    quit_menu_item = gtk_menu_item_new_with_label("Quit");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), file_menu);

    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), load_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_menu_item);

    settings_menu = gtk_menu_new();
    settings_menu_item = gtk_menu_item_new_with_label("Settings");

    save_path_menu_item = gtk_menu_item_new_with_label("Save Path");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(settings_menu_item), settings_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(settings_menu), save_path_menu_item);

    help_menu = gtk_menu_new();
    help_menu_item = gtk_menu_item_new_with_label("Help");

    about_menu_item = gtk_menu_item_new_with_label("About");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item), help_menu);

    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_menu_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), settings_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_menu_item);
    gtk_box_pack_start(GTK_BOX(vbox_main), menubar, FALSE, TRUE, 0);

    g_signal_connect_swapped(G_OBJECT(load_menu_item), "activate", G_CALLBACK(load_file), (gpointer)window);
    g_signal_connect_swapped(G_OBJECT(save_menu_item), "activate", G_CALLBACK(save_reg), (gpointer)window);
    g_signal_connect(G_OBJECT(quit_menu_item), "activate", G_CALLBACK(delete_event), NULL);
    g_signal_connect_swapped(G_OBJECT(save_path_menu_item), "activate", G_CALLBACK(set_save_path), (gpointer)window);
    g_signal_connect_swapped(G_OBJECT(about_menu_item), "activate", G_CALLBACK(show_about), (gpointer)window);

    /* graphics area design */
    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_can_focus(GTK_WIDGET(drawing_area), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox_main), drawing_area, TRUE, TRUE, 0);
    gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK);

    /* other interface design */
    hbox_iface = gtk_hbox_new(FALSE, 0);
    check_rc = gtk_check_button_new_with_label("Enable Write Mode.");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_rc), FALSE);

    text_rc = gtk_label_new("Write to register:");
    edit_rc = gtk_entry_new_with_max_length(16);

    check_rc_cmos = gtk_check_button_new_with_label("Enable CMOS Write Mode.");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_rc_cmos), FALSE);

    text_rc_cmos = gtk_label_new("Write to CMOS register:");
    edit_rc_cmos = gtk_entry_new_with_max_length(16);

    check_prog = gtk_check_button_new_with_label("Enable Program Mode.");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_prog), FALSE);

    button_prog_run = gtk_button_new_with_label("Run");

    check_bright = gtk_check_button_new_with_label("Enable Brightness Set.");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_bright), FALSE);

    text_bright = gtk_label_new("(0-50):");
    edit_bright = gtk_entry_new_with_max_length(16);


    g_signal_connect(G_OBJECT(check_rc), "toggled", G_CALLBACK(command_mode_change), NULL);
    g_signal_connect(G_OBJECT(check_rc_cmos), "toggled", G_CALLBACK(cmos_command_mode_change), NULL);
    g_signal_connect(G_OBJECT(check_prog), "toggled", G_CALLBACK(program_mode_change), NULL);
    g_signal_connect(G_OBJECT(button_prog_run), "clicked", G_CALLBACK(run_current_program), NULL);
    g_signal_connect(G_OBJECT(check_bright), "toggled", G_CALLBACK(bright_mode_change), NULL);

    gtk_box_pack_start(GTK_BOX(hbox_iface), check_rc, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_iface), text_rc, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_iface), edit_rc, FALSE, TRUE, 0);
    gtk_entry_set_width_chars(GTK_ENTRY(edit_rc), 16);
    gtk_box_pack_start(GTK_BOX(hbox_iface), check_rc_cmos, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_iface), text_rc_cmos, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_iface), edit_rc_cmos, FALSE, TRUE, 0);
    gtk_entry_set_width_chars(GTK_ENTRY(edit_rc_cmos), 16);

    gtk_box_pack_start(GTK_BOX(hbox_iface), check_prog, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_iface), button_prog_run, FALSE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(hbox_iface), check_bright, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_iface), text_bright, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_iface), edit_bright, FALSE, TRUE, 0);
    gtk_entry_set_width_chars(GTK_ENTRY(edit_bright), 16);

    gtk_box_pack_start(GTK_BOX(vbox_main), hbox_iface, FALSE, TRUE, 0);

    statusbr = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vbox_main), statusbr, FALSE, TRUE, 0);

    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL);
    g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(on_key_press), NULL);

    gtk_widget_set_sensitive(GTK_WIDGET(edit_rc), command_mode_enable);
    gtk_widget_set_sensitive(GTK_WIDGET(edit_rc_cmos), cmos_command_mode_enable);
    gtk_widget_set_sensitive(GTK_WIDGET(button_prog_run), program_mode_enable);
    gtk_widget_set_sensitive(GTK_WIDGET(edit_bright), bright_mode_enable);
    gtk_widget_set_sensitive(GTK_WIDGET(load_menu_item), program_mode_enable);

    gtk_widget_set_can_focus(GTK_WIDGET(edit_rc), TRUE);
    gtk_widget_set_can_focus(GTK_WIDGET(edit_rc_cmos), TRUE);
    gtk_widget_set_can_focus(GTK_WIDGET(edit_bright), TRUE);

    gtk_widget_show(window);
    init_gl();
    gtk_widget_show_all(window);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    if(freenect_run()==0) gtk_main();
    return 0;
}

static void clear_all_data()
{
    die = 1;
    pthread_join(freenect_thread, NULL);
    if(program_mode_enable==1)
    {
        free(prog_cap_data.data_rgb);
        free(prog_cap_data.data_ir);
        free(prog_cap_data.data_depth);
    }
    free(depth_mid);
    free(depth_mid_raw);
    free(depth_front);
    free(depth_front_raw);
    free(rgb_back);
    free(rgb_mid);
    free(rgb_front);
    free(exposure_pixels);

    prog_cap_data.data_rgb = NULL;
    prog_cap_data.data_ir = NULL;
    prog_cap_data.data_depth = NULL;
    depth_mid = NULL;
    depth_mid_raw = NULL;
    depth_front = NULL;
    depth_front_raw = NULL;
    rgb_back = NULL;
    rgb_mid = NULL;
    rgb_front = NULL;
    exposure_pixels = NULL;
}

gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    die = 1;
    pthread_join(freenect_thread, NULL);
    if(program_mode_enable==1)
    {
        free(prog_cap_data.data_rgb);
        free(prog_cap_data.data_ir);
        free(prog_cap_data.data_depth);
    }
    gtk_main_quit();
    free(depth_mid);
    free(depth_mid_raw);
    free(depth_front);
    free(depth_front_raw);
    free(rgb_back);
    free(rgb_mid);
    free(rgb_front);
    free(exposure_pixels);
    if(initflag==1) freenect_shutdown(f_ctx);
    return FALSE;
}

static void draw_histogram_rgb()
{
    int i;
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f,1.0f,1.0f, 0.35f);
    glRectf(HIST_RGB_X, HIST_RGB_Y,HIST_RGB_X+HIST_WIDTH, HIST_RGB_Y+HIST_HEIGHT);

    glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(HIST_RGB_X+HIST_GAP, HIST_RGB_Y+HIST_HEIGHT-HIST_GAP);
    for(i=0; i<HIST_NUM_BARS; ++i)
    {
        glVertex2f(HIST_RGB_X+HIST_GAP+i*(HIST_BAR_WIDTH+HIST_GAP), HIST_RGB_Y+(1-hist_rgb[0][i])*(HIST_HEIGHT-2*HIST_GAP)+HIST_GAP);
    }
    glVertex2f(HIST_RGB_X+HIST_GAP+i*(HIST_BAR_WIDTH+HIST_GAP), HIST_RGB_Y+HIST_HEIGHT-HIST_GAP);
    glEnd();

    glColor3f(0.0f,1.0f,0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(HIST_RGB_X+HIST_GAP, HIST_RGB_Y+HIST_HEIGHT-HIST_GAP);
    for(i=0; i<HIST_NUM_BARS; ++i)
    {
        glVertex2f(HIST_RGB_X+HIST_GAP+i*(HIST_BAR_WIDTH+HIST_GAP), HIST_RGB_Y+(1-hist_rgb[1][i])*(HIST_HEIGHT-2*HIST_GAP)+HIST_GAP);
    }
    glVertex2f(HIST_RGB_X+HIST_GAP+i*(HIST_BAR_WIDTH+HIST_GAP), HIST_RGB_Y+HIST_HEIGHT-HIST_GAP);
    glEnd();

    glColor3f(0.0f,0.0f,1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(HIST_RGB_X+HIST_GAP, HIST_RGB_Y+HIST_HEIGHT-HIST_GAP);
    for(i=0; i<HIST_NUM_BARS; ++i)
    {
        glVertex2f(HIST_RGB_X+HIST_GAP+i*(HIST_BAR_WIDTH+HIST_GAP), HIST_RGB_Y+(1-hist_rgb[2][i])*(HIST_HEIGHT-2*HIST_GAP)+HIST_GAP);
    }
    glVertex2f(HIST_RGB_X+HIST_GAP+i*(HIST_BAR_WIDTH+HIST_GAP), HIST_RGB_Y+HIST_HEIGHT-HIST_GAP);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

static void draw_histogram_ir()
{
    int i;
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f,1.0f,1.0f, 0.35f);
    glRectf(HIST_IR_X, HIST_IR_Y,HIST_IR_X+HIST_WIDTH,HIST_IR_Y+HIST_HEIGHT);

    glColor3f(0.1f,0.1f,0.1f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(HIST_IR_X+HIST_GAP, HIST_IR_Y+HIST_HEIGHT-HIST_GAP);
    for(i=0; i<HIST_NUM_BARS; ++i)
    {
        glVertex2f(HIST_IR_X+HIST_GAP+i*(HIST_BAR_WIDTH+HIST_GAP), HIST_IR_Y+(1-hist_ir[i])*(HIST_HEIGHT-2*HIST_GAP)+HIST_GAP);
    }
    glVertex2f(HIST_IR_X+HIST_GAP+i*(HIST_BAR_WIDTH+HIST_GAP), HIST_IR_Y+HIST_HEIGHT-HIST_GAP);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

static void calc_histogram_rgb()
{
    int i, k, total_pixels = freenect_find_video_mode(current_resolution, current_format).bytes, total_pixels_divided_by_3;
    total_pixels_divided_by_3 = total_pixels/3;
    for(i=0; i<3; ++i)
    {
        for(k=0; k<HIST_NUM_BARS; ++k)
        {
            hist_rgb[i][k] = 0;
        }
        for(k=0; k<total_pixels; k=k+3)
        {
            ++hist_rgb[i][(int)(HIST_NUM_BARS*(((double)rgb_front[k+i])/256.0))];
        }
        for(k=0; k<HIST_NUM_BARS; ++k)
        {
            hist_rgb[i][k]/=total_pixels_divided_by_3;
        }
    }
}

static void calc_histogram_ir()
{
    int k, total_pixels;
    for(k=0; k<HIST_NUM_BARS; ++k)
    {
        hist_ir[k] = 0;
    }
    if(capture_mode==3 || capture_mode==5)
    {
        total_pixels = freenect_find_video_mode(current_resolution, current_format).bytes;
        for(k=0; k<total_pixels; ++k)
        {
            ++hist_ir[(int)(HIST_NUM_BARS*(((double)rgb_front[k])/256.0))];
        }
    }
    else
    {
        total_pixels = freenect_find_video_mode(current_resolution, current_format).bytes;
        for(k=0; k<total_pixels; k= k+2)
        {
            ++hist_ir[(int)(HIST_NUM_BARS*((double)(*((uint16_t*)(rgb_front+k))/1024.0)))];
        }
        total_pixels/=2;
    }

    for(k=0; k<HIST_NUM_BARS; ++k)
    {
        hist_ir[k]/=total_pixels;
    }
}

static void check_exposure_ir()
{
    int k, total_pixels;
    uint16_t curr_val;
    freenect_frame_mode frame_mode;
    frame_mode = freenect_get_current_video_mode(f_dev);
    glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
    total_pixels = freenect_find_video_mode(current_resolution, current_format).bytes;
    if(capture_mode==3 || capture_mode==5)
    {
        memset(exposure_pixels, 0, total_pixels*3);
        for(k=0; k<total_pixels; ++k)
        {
            if(rgb_front[k]>251) exposure_pixels[3*k] = 255;
            else if(rgb_front[k]<5) exposure_pixels[3*k+2] = 255;
            else memset(exposure_pixels+3*k, rgb_front[k], 3);
        }
    }
    else if(capture_mode==6)
    {
        total_pixels /= 2;
        memset(exposure_pixels, 0, total_pixels*3);
        for(k=0; k<total_pixels; ++k)
        {
            curr_val = (*((uint16_t*)(rgb_front+2*k)));
            if(curr_val>1007) exposure_pixels[3*k] = 255;
            else if(curr_val<20) exposure_pixels[3*k+2] = 255;
            else memset(exposure_pixels+3*k, curr_val/4, 3);
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, 3, frame_mode.width, frame_mode.height, 0, GL_RGB, GL_UNSIGNED_BYTE, exposure_pixels);
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glTexCoord2f(0, 0);
    glVertex3f((DRAWING_AREA_WIDTH/2), 0, 0);
    glTexCoord2f(1, 0);
    glVertex3f(DRAWING_AREA_WIDTH, 0, 0);
    glTexCoord2f(1, 1);
    glVertex3f(DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT, 0);
    glTexCoord2f(0, 1);
    glVertex3f((DRAWING_AREA_WIDTH/2), DRAWING_AREA_HEIGHT, 0);
    glEnd();
}


static void command_mode_change(GtkToggleButton *button)
{
    command_mode_enable = gtk_toggle_button_get_active(button);
    gtk_widget_set_sensitive(GTK_WIDGET(edit_rc), command_mode_enable);
    gtk_widget_grab_focus(GTK_WIDGET(edit_rc));
}

static void cmos_command_mode_change(GtkToggleButton *button)
{
    cmos_command_mode_enable = gtk_toggle_button_get_active(button);
    gtk_widget_set_sensitive(GTK_WIDGET(edit_rc_cmos), cmos_command_mode_enable);
    gtk_widget_grab_focus(GTK_WIDGET(edit_rc_cmos));
}

static void bright_mode_change(GtkToggleButton *button)
{
    bright_mode_enable = gtk_toggle_button_get_active(button);
    gtk_widget_set_sensitive(GTK_WIDGET(edit_bright), bright_mode_enable);
    gtk_widget_grab_focus(GTK_WIDGET(edit_bright));
}

static void program_mode_change(GtkToggleButton *button)
{
    program_mode_enable = gtk_toggle_button_get_active(button);
    gtk_widget_set_sensitive(GTK_WIDGET(load_menu_item), program_mode_enable);
    gtk_widget_set_sensitive(GTK_WIDGET(button_prog_run), program_mode_enable && (codes.nlines>1));
    if(program_mode_enable==0)
    {
        program_running = 0;
        free(prog_cap_data.data_rgb);
        free(prog_cap_data.data_ir);
        free(prog_cap_data.data_depth);
        prog_cap_data.is_depth = 0;
        prog_cap_data.is_ir = 0;
        prog_cap_data.is_rgb = 0;
        prog_cap_data.frame_number = capturing_frame_number;
        prog_cap_data.is_saved = 0;
        prog_cap_data.format_ir = 255;
        prog_cap_data.format_rgb = 255;
        prog_cap_data.resolution_ir = 255;
        prog_cap_data.resolution_rgb = 255;
    }
    else
    {
        prog_cap_data.data_ir = (uint8_t*)malloc(freenect_find_video_mode(requested_resolution, requested_format).bytes);
        prog_cap_data.data_rgb = (uint8_t*)malloc(freenect_find_video_mode(requested_resolution, requested_format).bytes);
        prog_cap_data.data_depth = (uint8_t*)malloc(640*480*2);
        switch(capture_mode)
        {
        case 1:
        case 2:
        case 4:
            prog_cap_data.resolution_rgb = current_resolution;
            prog_cap_data.format_rgb = current_format;
            break;
        case 3:
        case 5:
        case 6:
            prog_cap_data.resolution_ir = current_resolution;
            prog_cap_data.format_ir = current_format;
        }
        prog_cap_data.frame_number = capturing_frame_number;
    }
}

static gboolean expose(GtkWidget *drawing_area, GdkEventExpose *event, gpointer user_data)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context(drawing_area);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(drawing_area);
    if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) g_assert_not_reached();
    draw_gl_scene();
    if(gdk_gl_drawable_is_double_buffered(gldrawable)) gdk_gl_drawable_swap_buffers(gldrawable);
    else glFlush();
    gdk_gl_drawable_gl_end(gldrawable);
    return TRUE;
}

static gboolean configure(GtkWidget *drawing_area, GdkEventConfigure *event, gpointer user_data)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context(drawing_area);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(drawing_area);

    if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) g_assert_not_reached();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);

    glGenTextures(1, &gl_depth_tex);
    glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &gl_rgb_tex);
    glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    resize_gl_scene(DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT);
    gdk_gl_drawable_gl_end(gldrawable);
    return TRUE;
}

static gboolean update(gpointer user_data)
{
    GtkWidget *drawing_area = GTK_WIDGET(user_data);
    gdk_window_invalidate_rect(drawing_area->window, &drawing_area->allocation, FALSE);
    gdk_window_process_updates(drawing_area->window, FALSE);
    return TRUE;
}

static void init_gl()
{
    glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB|GDK_GL_MODE_DOUBLE);
    if(!glconfig) g_assert_not_reached();
    if(!gtk_widget_set_gl_capability(drawing_area, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE)) g_assert_not_reached();
    g_signal_connect(drawing_area, "configure-event", G_CALLBACK(configure), NULL);
    g_signal_connect(drawing_area, "expose-event", G_CALLBACK(expose), NULL);
    g_timeout_add(1000.0/30, update, drawing_area);
}

static int run_register_command()
{
    if(command_mode_enable)
    {
        char command[32], *next_word = NULL;
        int num_of_words = 0;
        uint16_t n1, n2;
        sprintf(command, "%s", gtk_entry_get_text(GTK_ENTRY(edit_rc)));
        kc_count_words_in_line(command, &num_of_words);
        if(num_of_words==2)
        {
            n1 = (uint16_t)strtol(command, NULL, 0);
            next_word = kc_go_next_word(command);
            if(next_word!=NULL)
            {
                n2 = (uint16_t)strtol(next_word, NULL, 0);
                write_register_mine(f_dev, n1, n2);
                return 0;
            }
        }
    }
    return 1;
}

static int run_register_cmos_command()
{
    if(cmos_command_mode_enable)
    {
        char command[32], *next_word = NULL;
        int num_of_words = 0;
        uint16_t n1, n2;

        sprintf(command, "%s", gtk_entry_get_text(GTK_ENTRY(edit_rc_cmos)));
        kc_count_words_in_line(command, &num_of_words);
        if(num_of_words==2)
        {
            n1 = (uint16_t)strtol(command, NULL, 0);
            next_word = kc_go_next_word(command);
            if(next_word!=NULL)
            {
                n2 = (uint16_t)strtol(next_word, NULL, 0);
                write_cmos_register_mine(f_dev, n1, n2);
                return 0;
            }
        }
    }
    return 1;
}

static int run_bright_command()
{
    if(bright_mode_enable)
    {
        char command[32];
        int num_of_words = 0;
        uint16_t n;

        sprintf(command, "%s", gtk_entry_get_text(GTK_ENTRY(edit_bright)));
        kc_count_words_in_line(command, &num_of_words);
        if(num_of_words==1)
        {
            n = (uint16_t)strtol(command, NULL, 0);
            if(n<=50)
            {
                write_register_mine(f_dev, 0x15, n);
                return 0;
            }
        }
    }
    return 1;
}

static void run_current_program(GtkWidget *button)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&th, &attr, run_current_program_thread, (void *)&prog_th_id);
    pthread_attr_destroy(&attr);
}

static void *run_current_program_thread(void *x)
{
    if(program_mode_enable==1 && codes.allocated==1)
    {
        prog_stck = dtype_stack_creat();
        for(prog_k=0; prog_k<codes.nlines; ++prog_k)
        {
            switch(codes.cwrds[prog_k].codenum)
            {
            case 7: /* start */
                run_comm_start();
                break;

            case 8: /* stop */
                run_comm_stop();
                break;

            case 17: /* loop */
                run_comm_loop_with_arg(codes.cwrds[prog_k].codeval);
                break;

            case 12: /* end */
                run_comm_end();
                break;

            default:
                prog_tmp2 = dtype_stack_top(prog_stck);
                if(prog_tmp2->second>=0)
                {
                    if(codes.cwrds[prog_k].hasval)
                    {
                        code_look_up_table_with_arg[codes.cwrds[prog_k].codenum-(NUM_CODEWORDS+1)](codes.cwrds[prog_k].codeval);
                    }
                    else
                    {
                        code_look_up_table[codes.cwrds[prog_k].codenum]();
                    }
                    kc_sleep(50);
                }
            }
        }
    }
    pthread_exit(NULL);
    return 0;
}

static void run_comm_switch_mode_1()
{
    /* switch to mode 1 640x480 RGB */
    if(depth_on==0) depth_on = 1;
    requested_format = FREENECT_VIDEO_RGB;
    requested_resolution = FREENECT_RESOLUTION_MEDIUM;
}

static void run_comm_switch_mode_2()
{
    if(depth_on==0) depth_on = 1;
    /* switch to mode 2 640x480 YUV */
    requested_format = FREENECT_VIDEO_YUV_RGB;
    requested_resolution = FREENECT_RESOLUTION_MEDIUM;
}

static void run_comm_switch_mode_3()
{
    /* switch to mode 3 640x480 IR */
    if(depth_on==0) depth_on = 1;
    requested_format = FREENECT_VIDEO_IR_8BIT;
    requested_resolution = FREENECT_RESOLUTION_MEDIUM;
}

static void run_comm_switch_mode_4()
{
    /* switch to mode 4 1280x1024 RGB */
    if(depth_on==0) depth_on = 1;
    requested_format = FREENECT_VIDEO_RGB;
    requested_resolution = FREENECT_RESOLUTION_HIGH;
}

static void run_comm_switch_mode_5()
{
    /* switch to mode 5 1280x1024 IR 8bit */
    requested_format = FREENECT_VIDEO_IR_8BIT;
    requested_resolution = FREENECT_RESOLUTION_HIGH;
    if(current_format==FREENECT_VIDEO_RGB ||current_format==FREENECT_VIDEO_YUV_RGB||current_resolution==FREENECT_RESOLUTION_MEDIUM)
    {
        freenect_stop_depth(f_dev);
        memset(depth_mid, 0, 640*480*3); /* black out the depth camera */
        got_depth = 1;
        depth_on = 0;
    }
}

static void run_comm_switch_mode_6()
{
    /* switch to mode 6 1280x1024 IR 10bit */
    requested_format = FREENECT_VIDEO_IR_10BIT;
    requested_resolution = FREENECT_RESOLUTION_HIGH;
    if(current_format==FREENECT_VIDEO_RGB ||current_format==FREENECT_VIDEO_YUV_RGB||current_resolution==FREENECT_RESOLUTION_MEDIUM)
    {
        freenect_stop_depth(f_dev);
        memset(depth_mid, 0, 640*480*3); /* black out the depth camera */
        got_depth = 1;
        depth_on = 0;
    }
}

static void run_comm_start()
{
    program_running = 1;
    gtk_widget_set_sensitive(GTK_WIDGET(button_prog_run), FALSE);
    prog_tmp.first = prog_k;
    prog_tmp.second = codes.cwrds[prog_k].codeval-1;
    dtype_stack_push(prog_stck, prog_tmp);
}

static void run_comm_stop()
{
    dtype_stack_pop(prog_stck);
    dtype_stack_free(prog_stck);
    program_running = 0;
    gtk_widget_set_sensitive(GTK_WIDGET(button_prog_run), TRUE);
}

static void run_comm_capture()
{
    if(capture_mode==1 || capture_mode==2 || capture_mode==4) /* RGB mode */
    {
        prog_cap_data.is_saved = 0;
        pthread_mutex_lock(&prog_save_mtx);
        prog_cap_data.resolution_rgb = current_resolution;
        prog_cap_data.format_rgb = current_format;

        memcpy(prog_cap_data.data_depth, depth_front_raw, 640*480*2);
        memcpy(prog_cap_data.data_rgb, rgb_front, freenect_find_video_mode(current_resolution, current_format).bytes);
        pthread_mutex_unlock(&prog_save_mtx);
        prog_cap_data.is_depth = 1;
        prog_cap_data.is_rgb = 1;
    }
    else  /* IR mode */
    {
        prog_cap_data.is_saved = 0;
        prog_cap_data.resolution_ir = current_resolution;
        prog_cap_data.format_ir = current_format;

        pthread_mutex_lock(&prog_save_mtx);
        memcpy(prog_cap_data.data_ir, rgb_front, freenect_find_video_mode(current_resolution, current_format).bytes);
        pthread_mutex_unlock(&prog_save_mtx);
        prog_cap_data.is_ir = 1;
    }
}

static void run_comm_save()
{
    if(prog_cap_data.is_saved==0)
    {
#if defined(__WIN32) || defined(__WIN32__) ||defined(WIN32) || defined(WINNT)
        sprintf(file_name, "%s\\Frame%d.fnk", folder_name, capturing_frame_number);
#else
        sprintf(file_name, "%s/Frame%d.fnk", folder_name, capturing_frame_number);
#endif
        if((capture_file = fopen(file_name,"wb"))!=NULL)
        {
            double dx, dy, dz;
            freenect_raw_tilt_state* state;
            freenect_update_tilt_state(f_dev);
            /* HEADER STARTS HERE */
            fwrite(hfstring, sizeof(char), 4, capture_file); /* file format */
            fputc((uint8_t)prog_cap_data.resolution_rgb, capture_file); /* current_rgb_resolution */
            fputc((uint8_t)prog_cap_data.resolution_ir, capture_file); /* current_ir_resolution */
            fputc((uint8_t)prog_cap_data.format_rgb, capture_file); /* current_rgb_format */
            fputc((uint8_t)prog_cap_data.format_ir, capture_file); /* current_ir_format */
            fputc((uint8_t)FREENECT_DEPTH_11BIT, capture_file); /* current_depth_format */
            /* content format  | x | RGB | IR | Depth | */
            fputc((uint8_t)(prog_cap_data.is_rgb*4+prog_cap_data.is_ir*2+prog_cap_data.is_depth), capture_file); /* current_content */
            /* HEADER ENDS HERE */
            if(prog_cap_data.is_rgb) fwrite(prog_cap_data.data_rgb, sizeof(uint8_t), prog_cap_data.sz_rgb[0]*prog_cap_data.sz_rgb[1]*prog_cap_data.sz_rgb[2], capture_file);
            if(prog_cap_data.is_ir)
            {
                if(prog_cap_data.format_ir == FREENECT_VIDEO_IR_8BIT)
                {
                    fwrite(prog_cap_data.data_ir, sizeof(uint8_t), prog_cap_data.sz_ir[0]*prog_cap_data.sz_ir[1]*prog_cap_data.sz_ir[2], capture_file); /* 8bit */
                }
                else
                {
                    fwrite(prog_cap_data.data_ir, sizeof(uint8_t), prog_cap_data.sz_ir[0]*prog_cap_data.sz_ir[1]*prog_cap_data.sz_ir[2]*2, capture_file); /* 10 bit */
                }
            }
            if(prog_cap_data.is_depth) fwrite(prog_cap_data.data_depth, sizeof(uint8_t), 640*480*2, capture_file);

            /* store accelerometer */
            state = freenect_get_tilt_state(f_dev);
            freenect_get_mks_accel(state, &dx, &dy, &dz);
            fwrite(&dx, sizeof(double), 1, capture_file);
            fwrite(&dy, sizeof(double), 1, capture_file);
            fwrite(&dz, sizeof(double), 1, capture_file);
            fclose(capture_file);
            update_status_text(dispstring[capture_mode], capturing_frame_number);
            capturing_frame_number++;
        }
        prog_cap_data.frame_number = capturing_frame_number;
    }
}

static void run_comm_projector()
{
    if(projector_on==1)
    {
        write_register_mine(f_dev, 0x105, 0xffff);
        write_register_mine(f_dev, 0x106, 0x0);
        memset(depth_mid, 0, 640*480*3); /* black out the depth camera */
        got_depth = 1;
        projector_on = 0;
    }
    else
    {
        write_register_mine(f_dev, 0x105, 0x0);
        got_depth = 1;
        projector_on = 1;
    }
}

static void run_comm_end()
{
    prog_tmp2 = dtype_stack_top(prog_stck);
    if(prog_tmp2->second<=0)
    {
        dtype_stack_pop(prog_stck);
    }
    else
    {
        --(prog_tmp2->second);
        prog_k = prog_tmp2->first;
    }
}

static void run_comm_pause_with_arg(int arg)
{
    kc_sleep(arg);
}

static void run_comm_brightness_with_arg(int arg)
{
    write_register_mine(f_dev, 0x15, (uint16_t)arg);
}

static void run_comm_switch_with_arg(int arg)
{
    if(arg<0) ++user_device_number;
    else user_device_number = arg;
    die = 1;
    pthread_join(freenect_thread, NULL);
    nr_devices = freenect_num_devices(f_ctx);
    if(user_device_number>=nr_devices) user_device_number = 0;
    die = 0;
    freenect_run();
}

static void run_comm_loop_with_arg(int arg)
{
    prog_tmp.first = prog_k;
    prog_tmp.second = codes.cwrds[prog_k].codeval-1;
    dtype_stack_push(prog_stck, prog_tmp);
}

static void show_about(GtkWidget *widget, gpointer data)
{

    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "KinectCapture");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "0.3.3");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "Copyright (c) 2014 Sk. Mohammadul Haque");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "KinectCapture is a software to view, capture raw Kinect data in different modes.");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "http://mohammadulhaque.alotspace.com");
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void load_file(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(widget), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_widget_destroy(dialog);
        if(codes.allocated==1) unload_program_code();
        load_program_code(filename);
        if(codes.nlines>1) gtk_widget_set_sensitive(GTK_WIDGET(button_prog_run), program_mode_enable);
        g_free(filename);
    }
    else gtk_widget_destroy(dialog);
}

static void save_reg(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(widget), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    freenect_registration reg = freenect_copy_registration(f_dev);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
    if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_ACCEPT)
    {
        FILE *fp = NULL;
        char *filename = NULL;
        int i;
        GtkFileFilter *fnamefilter = gtk_file_filter_new();
        gtk_file_filter_set_name(fnamefilter, "Kinect Registration File");
        gtk_file_filter_add_pattern(fnamefilter, ".reg");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), fnamefilter);
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_widget_destroy(dialog);
        if((fp = fopen(filename, "wb"))!=NULL)
        {
            fwrite(reg.raw_to_mm_shift, sizeof(uint16_t),1024, fp);
            fwrite(reg.depth_to_rgb_shift, sizeof(int32_t),8192*2, fp);
            for(i=0; i<640*480; ++i) fwrite(reg.registration_table[i], sizeof(int32_t),2, fp);
            fclose(fp);
        }
        g_free(filename);
    }
    else gtk_widget_destroy(dialog);
}

static void set_save_path(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Folder", GTK_WINDOW(widget), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_ACCEPT)
    {
        char *filename = NULL;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_widget_destroy(dialog);
        strcpy(folder_name, filename);
        update_frame_number();
        g_free(filename);
    }
    else gtk_widget_destroy(dialog);
}

static void update_status_text(const char *str, int cnum)
{
    guint i = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbr), "");
    sprintf(statusbr_text, "%s, Current Frame to Save: %d", str, cnum);
    gtk_statusbar_pop(GTK_STATUSBAR(statusbr), i);
    gtk_statusbar_push(GTK_STATUSBAR(statusbr), i, statusbr_text);
}

static void update_frame_number()
{
    FILEPOINTER search_file = NULL;
    char search_filename[256];
    capturing_frame_number = -1;
    do
    {
        if(search_file)
        {
            fclose(search_file);
        }
        ++capturing_frame_number;
#if defined(__WIN32) || defined(__WIN32__) ||defined(WIN32) || defined(WINNT)
        sprintf(search_filename, "%s\\Frame%d.fnk", folder_name, capturing_frame_number);
#else
        sprintf(search_filename, "%s/Frame%d.fnk", folder_name, capturing_frame_number);
#endif
        search_file = fopen(search_filename, "r");
    }
    while(search_file);
    update_status_text(dispstring[capture_mode], capturing_frame_number);

}


static void draw_gl_scene()
{
    int k, total_pixels;
    uint8_t *tmp;
    uint16_t curr_val;
    freenect_frame_mode frame_mode;
    if(initflag==1)
    {
        pthread_mutex_lock(&gl_backbuf_mutex);
        if(current_format==FREENECT_VIDEO_YUV_RGB)
        {
            while(!got_depth && !got_rgb) pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
        }
        else
        {
            while((!got_depth||!got_rgb) && requested_format!=current_format) pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
        }
        if(requested_format!=current_format || requested_resolution != current_resolution)
        {
            pthread_mutex_unlock(&gl_backbuf_mutex);
            return;
        }
        if(got_depth)
        {
            pthread_mutex_lock(&prog_save_mtx);
            tmp = depth_front;
            depth_front = depth_mid;
            depth_mid = tmp;
            tmp = depth_front_raw;
            depth_front_raw = depth_mid_raw;
            pthread_mutex_unlock(&prog_save_mtx);
            depth_mid_raw = tmp;
            got_depth = 0;
        }
        if(got_rgb)
        {
            pthread_mutex_lock(&prog_save_mtx);
            tmp = rgb_front;
            rgb_front = rgb_mid;
            pthread_mutex_unlock(&prog_save_mtx);
            rgb_mid = tmp;
            got_rgb = 0;
        }
        pthread_mutex_unlock(&gl_backbuf_mutex);

        glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, depth_front);

        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glTexCoord2f(0, 0);
        glVertex3f(0, 0, 0);
        glTexCoord2f(1, 0);
        glVertex3f((DRAWING_AREA_WIDTH/2), 0, 0);
        glTexCoord2f(1, 1);
        glVertex3f((DRAWING_AREA_WIDTH/2), DRAWING_AREA_HEIGHT, 0);
        glTexCoord2f(0, 1);
        glVertex3f(0, DRAWING_AREA_HEIGHT, 0);
        glEnd();
        if(check_exposure_ir_enable==1 && (capture_mode==3 || capture_mode==5 ||capture_mode==6)) check_exposure_ir();
        else
        {
            frame_mode = freenect_get_current_video_mode(f_dev);
            glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
            if(current_format==FREENECT_VIDEO_IR_10BIT) /* use exposure_pixels to display 10bit IR image correctly*/
            {
                total_pixels = freenect_find_video_mode(current_resolution, current_format).bytes;
                total_pixels /= 2;
                memset(exposure_pixels, 0, total_pixels*3);
                for(k=0; k<total_pixels; ++k)
                {
                    curr_val = (*((uint16_t*)(rgb_front+2*k)));
                    memset(exposure_pixels+3*k, curr_val/4, 3);
                }
                glTexImage2D(GL_TEXTURE_2D, 0, 3, frame_mode.width, frame_mode.height, 0, GL_RGB, GL_UNSIGNED_BYTE, exposure_pixels);
                glBegin(GL_TRIANGLE_FAN);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glTexCoord2f(0, 0);
                glVertex3f((DRAWING_AREA_WIDTH/2), 0, 0);
                glTexCoord2f(1, 0);
                glVertex3f(DRAWING_AREA_WIDTH, 0, 0);
                glTexCoord2f(1, 1);
                glVertex3f(DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT, 0);
                glTexCoord2f(0, 1);
                glVertex3f((DRAWING_AREA_WIDTH/2), DRAWING_AREA_HEIGHT, 0);
                glEnd();
            }
            else
            {
                if(current_format==FREENECT_VIDEO_RGB || current_format==FREENECT_VIDEO_YUV_RGB)
                    glTexImage2D(GL_TEXTURE_2D, 0, 3, frame_mode.width, frame_mode.height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_front);
                else
                    glTexImage2D(GL_TEXTURE_2D, 0, 1, frame_mode.width, frame_mode.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, rgb_front);
                glBegin(GL_TRIANGLE_FAN);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glTexCoord2f(0, 0);
                glVertex3f((DRAWING_AREA_WIDTH/2), 0, 0);
                glTexCoord2f(1, 0);
                glVertex3f(DRAWING_AREA_WIDTH, 0, 0);
                glTexCoord2f(1, 1);
                glVertex3f(DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT, 0);
                glTexCoord2f(0, 1);
                glVertex3f((DRAWING_AREA_WIDTH/2), DRAWING_AREA_HEIGHT, 0);
                glEnd();
            }
        }
        if(histogram_enable==1)
        {
            if(capture_mode==1||capture_mode==2||capture_mode==4)
            {
                calc_histogram_rgb();
                draw_histogram_rgb();
            }
            else
            {
                calc_histogram_ir();
                draw_histogram_ir();
            }
        }
        if(frame_captured)
        {
#if defined(__WIN32) || defined(__WIN32__) ||defined(WIN32) || defined(WINNT)
            sprintf(file_name, "%s\\Frame%d.fnk", folder_name, capturing_frame_number);
#else
            sprintf(file_name, "%s/Frame%d.fnk", folder_name, capturing_frame_number);
#endif
            if((capture_file = fopen(file_name,"wb"))!=NULL)
            {
                double dx, dy, dz;
                freenect_raw_tilt_state* state;
                freenect_update_tilt_state(f_dev);

                /* HEADER STARTS HERE */
                fwrite(hfstring, sizeof(char), 4, capture_file); /* file format */
                if(capture_mode==1 ||capture_mode==2 ||capture_mode==4) /* RGB mode */
                {
                    fputc((uint8_t)current_resolution, capture_file); /* current_rgb_resolution */
                    fputc((uint8_t)255, capture_file); /* current_ir_resolution */
                    fputc((uint8_t)current_format, capture_file); /* current_rgb_format */
                    fputc((uint8_t)255, capture_file); /* current_ir_format */
                    fputc((uint8_t)FREENECT_DEPTH_11BIT, capture_file); /* current_depth_format */
                    /* content format  | x | RGB | IR | Depth | */
                    fputc((char)(5), capture_file); /* current_content */
                }
                else /* IR mode */
                {
                    fputc((uint8_t)255, capture_file); /* current_rgb_resolution */
                    fputc((uint8_t)current_resolution, capture_file); /* current_ir_resolution */
                    fputc((uint8_t)255, capture_file); /* current_rgb_format */
                    fputc((uint8_t)current_format, capture_file); /* current_ir_format */
                    if(capture_mode==5 || capture_mode==6)
                    {
                        fputc((uint8_t)255, capture_file); /* current_depth_format not required */
                        /* content format  | x | RGB | IR | Depth | */
                        fputc((uint8_t)(2), capture_file); /* current_content */
                    }
                    else
                    {
                        fputc((uint8_t)FREENECT_DEPTH_11BIT, capture_file); /* current_depth_format  is valid and required*/
                        /* content format  | x | RGB | IR | Depth | */
                        fputc((uint8_t)(3), capture_file); /* current_content */
                    }
                }
                /* HEADER ENDS HERE */

                if(capture_mode==3)
                {
                    /* to skip last 8 rows of 488X640 of IR image */
                    fwrite(rgb_front, sizeof(uint8_t), 640*480, capture_file);
                }
                else fwrite(rgb_front, sizeof(uint8_t), freenect_find_video_mode(current_resolution, current_format).bytes, capture_file);
                if(!(capture_mode==5 || capture_mode==6))
                {
                    fwrite(depth_front_raw, sizeof(uint8_t), 640*480*2, capture_file); /* write only when required */
                }

                /* store accelerometer */
                state = freenect_get_tilt_state(f_dev);
                freenect_get_mks_accel(state, &dx, &dy, &dz);
                fwrite(&dx, sizeof(double), 1, capture_file);
                fwrite(&dy, sizeof(double), 1, capture_file);
                fwrite(&dz, sizeof(double), 1, capture_file);
                fclose(capture_file);
                capturing_frame_number++;
                update_status_text(dispstring[capture_mode], capturing_frame_number);
            }
            frame_captured = 0;
        }
    }
}

static void resize_gl_scene(int width, int height)
{
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGHT, 0, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    switch(event->keyval)
    {
    case GDK_w:
    case GDK_W:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            freenect_angle++;
            if(freenect_angle>30) freenect_angle = 30;
            freenect_set_tilt_degs(f_dev,freenect_angle);
        }
        break;

    case GDK_x:
    case GDK_X:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            freenect_angle--;
            if(freenect_angle<-30) freenect_angle = -30;
            freenect_set_tilt_degs(f_dev,freenect_angle);
        }
        break;

    case GDK_s:
    case GDK_S:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            freenect_angle = 0;
            freenect_set_tilt_degs(f_dev,freenect_angle);
        }
        break;

    case GDK_m:
    case GDK_M:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running)) write_register_mine(f_dev, 0x16, 0x0);
        break;

    case GDK_n:
    case GDK_N:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running)) write_register_mine(f_dev, 0x16, 0x1);
        break;

    case GDK_h:
    case GDK_H:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running)) histogram_enable = 1-histogram_enable;
        break;

    case GDK_e:
    case GDK_E:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running)) check_exposure_ir_enable = 1-check_exposure_ir_enable;
        break;

    case GDK_Return:
        if(program_running==0)
        {
            if(GTK_WIDGET_HAS_FOCUS(edit_rc)) run_register_command();
            else if(GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)) run_register_cmos_command();
            else if(GTK_WIDGET_HAS_FOCUS(edit_bright)) run_bright_command();
        }
        break;

    case GDK_1:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            code_look_up_table[1]();
        }
        break;
    case GDK_2:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            code_look_up_table[2]();
        }
        break;
    case GDK_3:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            code_look_up_table[3]();
        }
        break;
    case GDK_4:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            code_look_up_table[4]();
        }
        break;
    case GDK_5:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            code_look_up_table[5]();
        }
        break;
    case GDK_6:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            code_look_up_table[6]();
        }
        break;

    case GDK_f:
    case GDK_F:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            if(capture_mode==6) code_look_up_table[1]();
            else code_look_up_table[capture_mode+1]();
        }
        break;

    case GDK_d:
    case GDK_D:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            if(depth_on)
            {
                depth_changed = 1;
                got_depth = 0;
                depth_on = 0;
            }
            else
            {
                if(capture_mode != mode_look_up_table[FREENECT_RESOLUTION_HIGH][FREENECT_VIDEO_IR_8BIT] && capture_mode != mode_look_up_table[FREENECT_RESOLUTION_HIGH][FREENECT_VIDEO_IR_10BIT])
                {
                    depth_changed = 1;
                    depth_on = 1;
                }
            }
        }
        break;

    case GDK_c:
    case GDK_C:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running)) frame_captured = 1;
        break;

    case GDK_p:
    case GDK_P:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running)) run_comm_projector();
        break;

    case GDK_r:
    case GDK_R:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            if(gtk_widget_get_sensitive(GTK_WIDGET(button_prog_run)))run_current_program(NULL);
        }
        break;
    case GDK_q:
    case GDK_Q:
        if(!(GTK_WIDGET_HAS_FOCUS(edit_rc)||GTK_WIDGET_HAS_FOCUS(edit_rc_cmos)||GTK_WIDGET_HAS_FOCUS(edit_bright)||program_running))
        {
            run_comm_switch_with_arg(-1);
        }
        break;
    }
    return FALSE;
}

static void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
{
    int i, pval, lb;
    uint16_t *depth = (uint16_t*)v_depth;

    pthread_mutex_lock(&gl_backbuf_mutex);
    for(i=0; i<640*480; i++)
    {
        pval = t_gamma[depth[i]];
        lb = pval & 0xff;
        switch(pval>>8)
        {
        case 0:
            depth_mid[3*i+0] = 255;
            depth_mid[3*i+1] = 255-lb;
            depth_mid[3*i+2] = 255-lb;
            break;
        case 1:
            depth_mid[3*i+0] = 255;
            depth_mid[3*i+1] = lb;
            depth_mid[3*i+2] = 0;
            break;
        case 2:
            depth_mid[3*i+0] = 255-lb;
            depth_mid[3*i+1] = 255;
            depth_mid[3*i+2] = 0;
            break;
        case 3:
            depth_mid[3*i+0] = 0;
            depth_mid[3*i+1] = 255;
            depth_mid[3*i+2] = lb;
            break;
        case 4:
            depth_mid[3*i+0] = 0;
            depth_mid[3*i+1] = 255-lb;
            depth_mid[3*i+2] = 255;
            break;
        case 5:
            depth_mid[3*i+0] = 0;
            depth_mid[3*i+1] = 0;
            depth_mid[3*i+2] = 255-lb;
            break;
        default:
            depth_mid[3*i+0] = 0;
            depth_mid[3*i+1] = 0;
            depth_mid[3*i+2] = 0;
            break;
        }
        depth_mid_raw[2*i+0] = depth[i] & 0xff;
        depth_mid_raw[2*i+1] = (depth[i]>>8) & 7;
    }
    got_depth = 1;
    pthread_cond_signal(&gl_frame_cond);
    pthread_mutex_unlock(&gl_backbuf_mutex);
}

static void rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp)
{
    pthread_mutex_lock(&gl_backbuf_mutex);
    assert (rgb_back == rgb);
    rgb_back = rgb_mid;
    freenect_set_video_buffer(dev, rgb_back);
    rgb_mid = (uint8_t*)rgb;

    got_rgb++;
    pthread_cond_signal(&gl_frame_cond);
    pthread_mutex_unlock(&gl_backbuf_mutex);
}

static void *freenect_threadfunc(void *arg)
{
    freenect_set_led(f_dev, LED_RED);
    freenect_set_depth_callback(f_dev, depth_cb);
    freenect_set_video_callback(f_dev, rgb_cb);

    freenect_set_video_mode(f_dev, freenect_find_video_mode(current_resolution, current_format));
    freenect_set_depth_mode(f_dev, freenect_find_depth_mode(current_resolution, FREENECT_DEPTH_11BIT));
    freenect_set_video_buffer(f_dev, rgb_back);

    if(depth_on==1 && depth_running==0)
    {
        freenect_start_depth(f_dev);
        depth_running = 1;
    }
    freenect_start_video(f_dev);
    update_status_text(dispstring[capture_mode], capturing_frame_number);
    while(!die && freenect_process_events(f_ctx)>=0)
    {
        if(depth_changed==1)
        {
            if(depth_on==0 && depth_running==1)
            {
                freenect_stop_depth(f_dev);
                depth_running = 0;
                memset(depth_mid, 0, 640*480*3); /* black out the depth camera */
                got_depth = 1;
            }
            else if(depth_running==0 && depth_on==1)
            {
                freenect_start_depth(f_dev);
                depth_running = 1;
            }
            depth_changed = 0;
        }
        if(requested_format!=current_format||requested_resolution!=current_resolution)
        {
            freenect_frame_mode frame_mode;
            freenect_stop_video(f_dev);
            freenect_set_video_mode(f_dev, freenect_find_video_mode(requested_resolution, requested_format));

            pthread_mutex_lock(&gl_backbuf_mutex);
            free(rgb_back);
            free(rgb_mid);
            free(rgb_front);

            rgb_back = (uint8_t*)malloc(freenect_find_video_mode(requested_resolution, requested_format).bytes);
            rgb_mid = (uint8_t*)malloc(freenect_find_video_mode(requested_resolution, requested_format).bytes);
            rgb_front = (uint8_t*)malloc(freenect_find_video_mode(requested_resolution, requested_format).bytes);

            frame_mode = freenect_get_current_video_mode(f_dev);
            current_resolution = frame_mode.resolution;
            current_format = frame_mode.video_format;

            capture_mode = mode_look_up_table[current_resolution][current_format];
            pthread_mutex_unlock(&gl_backbuf_mutex);

            freenect_set_video_buffer(f_dev, rgb_back);
            if((capture_mode==1 ||capture_mode==2 ||capture_mode==4) && program_mode_enable) /* RGB mode */
            {
                free(prog_cap_data.data_rgb);
                prog_cap_data.data_rgb = (uint8_t*)malloc(freenect_find_video_mode(requested_resolution, requested_format).bytes);
                prog_cap_data.resolution_rgb = current_resolution;
                prog_cap_data.format_rgb = current_format;
                if(capture_mode==4)
                {
                    prog_cap_data.sz_rgb[0] = 1280;
                    prog_cap_data.sz_rgb[1] = 1024;
                    prog_cap_data.sz_rgb[2] = 3;
                }
                else
                {
                    prog_cap_data.sz_rgb[0] = 640;
                    prog_cap_data.sz_rgb[1] = 480;
                    prog_cap_data.sz_rgb[2] = 3;
                }
            }
            else
            {
                switch(capture_mode)
                {
                case 6:
                case 5:
                    free(exposure_pixels);
                    exposure_pixels = (uint8_t*)malloc(1280*1024*3);
                    break;
                case 3:
                    free(exposure_pixels);
                    exposure_pixels = (uint8_t*)malloc(640*488*3);
                    break;
                }
                if(program_mode_enable)
                {
                    free(prog_cap_data.data_ir);
                    prog_cap_data.data_ir = (uint8_t*)malloc(freenect_find_video_mode(requested_resolution, requested_format).bytes);
                    prog_cap_data.resolution_ir = current_resolution;
                    prog_cap_data.format_ir = current_format;
                    switch(capture_mode)
                    {
                    case 6:
                    case 5:
                        prog_cap_data.sz_ir[0] = 1280;
                        prog_cap_data.sz_ir[1] = 1024;
                        prog_cap_data.sz_ir[2] = 1;
                        break;

                    case 3:
                        prog_cap_data.sz_ir[0] = 640;
                        prog_cap_data.sz_ir[1] = 480;
                        prog_cap_data.sz_ir[2] = 1;
                    }
                }
            }
            freenect_start_video(f_dev);
            if(capture_mode<5 && depth_on==1 && depth_running==0)
            {
                freenect_start_depth(f_dev);
                depth_running = 1;
            }
            projector_on = 1;
            update_status_text(dispstring[capture_mode], capturing_frame_number);
        }

    }
    freenect_set_led(f_dev, LED_OFF);
    if(depth_on==1 && depth_running==1)
    {
        freenect_stop_depth(f_dev);
        depth_running = 0;
    }
    freenect_stop_video(f_dev);
    freenect_close_device(f_dev);
    die = 0;
    return NULL;
}

static int freenect_run()
{
    initflag = 0;
    if(freenect_init(&f_ctx, NULL)>=0)
        {
            freenect_set_log_level(f_ctx, FREENECT_LOG_FATAL);
            freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

    do
    {
           nr_devices = freenect_num_devices(f_ctx);
            if(nr_devices>0)
            {
                printf("May be here!\n");
                if(freenect_open_device(f_ctx, &f_dev, user_device_number)>=0) initflag = 1;
            }

        if(initflag==0)
        {
            cndlg = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO, "Could not connect to the Kinect Device. Retry?");
            gtk_window_set_title(GTK_WINDOW(cndlg), "Device Connection Failure");
            if(gtk_dialog_run(GTK_DIALOG(cndlg))==GTK_RESPONSE_NO)
            {
                gtk_widget_destroy(cndlg);
                return 1;
            };
            gtk_widget_destroy(cndlg);
        }

    }
    while(initflag==0);
    initialize_all_data();
    capture_mode = 1;
    res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL);
    if(res)
    {
        clear_all_data();
        freenect_shutdown(f_ctx);
        return 1;
    }
        }
    return 0;
}

static void initialize_all_data()
{
    int i;
    float v;
    depth_mid = (uint8_t*)malloc(640*480*3);
    depth_mid_raw = (uint8_t*)malloc(640*480*2);
    depth_front = (uint8_t*)malloc(640*480*3);
    depth_front_raw = (uint8_t*)malloc(640*480*2);
    rgb_back = (uint8_t*)malloc(640*480*3);
    rgb_mid = (uint8_t*)malloc(640*480*3);
    rgb_front = (uint8_t*)malloc(640*480*3);
    exposure_pixels = (uint8_t*)malloc(640*488*3);

    codes.allocated = 0;
    codes.nlines = 0;
    prog_cap_data.data_depth = NULL;
    prog_cap_data.data_ir = NULL;
    prog_cap_data.data_rgb = NULL;
    prog_cap_data.frame_number = -1;
    prog_cap_data.is_depth = 0;
    prog_cap_data.is_ir = 0;
    prog_cap_data.is_rgb = 0;
    prog_cap_data.is_saved = 0;
    prog_cap_data.sz_rgb[0] = 640;
    prog_cap_data.sz_rgb[1] = 480;
    prog_cap_data.sz_rgb[2] = 3;
    prog_cap_data.sz_ir[0] = 640;
    prog_cap_data.sz_ir[1] = 480;
    prog_cap_data.sz_ir[2] = 1;
    prog_cap_data.format_rgb = FREENECT_VIDEO_RGB;
    prog_cap_data.resolution_rgb = FREENECT_RESOLUTION_MEDIUM;
    prog_cap_data.format_ir = 255;
    prog_cap_data.resolution_ir = 255;

#if defined(__WIN32) || defined(__WIN32__) ||defined(WIN32) || defined(WINNT)
    sprintf(folder_name, "%s\\", getenv("HOMEPATH"));
#else
    sprintf(folder_name, "%s/", getenv("HOME"));
#endif
    update_frame_number();
    for(i=0; i<2048; ++i)
    {
        v = i/2048.0;
        v = powf(v, 3)*6;
        t_gamma[i] = (uint16_t)(v*1536.0);
    }
}
