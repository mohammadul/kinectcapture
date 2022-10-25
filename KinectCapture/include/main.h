/* ---------------------------------------------------------------------------
** This software is furnished "as is", without technical support,
** and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** main.h
** Contains header declarations and definitions for main.c.
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

#ifndef __MAIN__
#define __MAIN__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libfreenect.h"
#include "cameras.h"
#include "libfreenect-registration.h"

#include <math.h>
#include <assert.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#if(defined(_WIN32) || defined(WIN32)|| defined(_WINDOWS) || defined(WINDOWS))
#include <windows.h>
#define kc_sleep(x) Sleep((x))
#else
#include <unistd.h>
#define kc_sleep(x) usleep((x)*1000)
#endif

static void clear_all_data();
gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

static void draw_histogram_rgb();
static void draw_histogram_ir();
static void calc_histogram_rgb();
static void calc_histogram_ir();
static void check_exposure_ir();

static void command_mode_change(GtkToggleButton *button);
static void cmos_command_mode_change(GtkToggleButton *button);
static void program_mode_change(GtkToggleButton *button);
static void bright_mode_change(GtkToggleButton *button);

static gboolean expose(GtkWidget *drawing_area, GdkEventExpose *event, gpointer user_data);
static gboolean configure(GtkWidget *drawing_area, GdkEventConfigure *event, gpointer user_data);
static gboolean update(gpointer user_data);
static void init_gl();
static void draw_gl_scene();
static void resize_gl_scene(int width, int height);

static void show_about(GtkWidget *widget, gpointer data);
static void load_file(GtkWidget *widget, gpointer data);
static void save_reg(GtkWidget *widget, gpointer data);
static void set_save_path(GtkWidget *widget, gpointer data);

static int freenect_run();

static void update_status_text(const char *str, int cnum);
static void update_frame_number();
static int run_register_command();
static int run_register_cmos_command();
static int run_bright_command();

static void run_current_program(GtkWidget *button);
static void *run_current_program_thread(void *arg);
static void run_comm_switch_mode_1();
static void run_comm_switch_mode_2();
static void run_comm_switch_mode_3();
static void run_comm_switch_mode_4();
static void run_comm_switch_mode_5();
static void run_comm_switch_mode_6();
static void run_comm_start();
static void run_comm_stop();
static void run_comm_capture();
static void run_comm_save();
static void run_comm_projector();
static void run_comm_end();

static void run_comm_pause_with_arg(int arg);
static void run_comm_brightness_with_arg(int arg);
static void run_comm_switch_with_arg(int arg);
static void run_comm_loop_with_arg(int arg);

static void *freenect_threadfunc(void *arg);
static void initialize_all_data();

#define DRAWING_AREA_WIDTH 1280
#define DRAWING_AREA_HEIGHT 480


#define HIST_WIDTH (60.0)
#define HIST_HEIGHT (40.0)
#define HIST_NUM_BARS (16)
#define HIST_GAP (2.0)
#define HIST_RGB (0)
#define HIST_IR (1)

#define HIST_RGB_X (641.0)
#define HIST_RGB_Y (1.0)

#define HIST_IR_X (641.0)
#define HIST_IR_Y (1.0)

#define HIST_BAR_WIDTH (((HIST_WIDTH-HIST_GAP)/HIST_NUM_BARS)-HIST_GAP)

#endif
