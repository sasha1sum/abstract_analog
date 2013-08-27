#include "vector_analog.h"

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "string.h"
#include "stdlib.h"

#define UUID {0x15, 0x38, 0xb7, 0xc9, 0x92, 0xac, 0x43, 0x7a, 0xba, 0xcd, 0x64, 0xdb, 0x41, 0xea, 0x22, 0x84}

#define DAY_FIRST 0

PBL_APP_INFO(UUID,
             "Vector Analog",
             "Alec Goebel",
             0, 1, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

static struct SimpleAnalogData {
  Layer simple_bg_layer;

  GPoint minute_offset;
  GPath minute_arrow, hour_arrow;
  Layer hands_layer;

  GPoint dow_offset;
  int32_t dow_theta;
  GPath dow_arrow;
  Layer dow_layer;

  Window window;
} s_data;

static void bg_update_proc(Layer* me, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, me->bounds, 0, GCornerNone);
}

static void hands_update_proc(Layer* me, GContext* ctx) {
  const GPoint center = grect_center_point(&me->bounds);

  PblTm t;
  get_time(&t);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  s_data.dow_theta = TRIG_MAX_ANGLE * (((t.tm_hour % 12) * 6) + (t.tm_min / 10)) / (12 * 6);

  // hour hand
  gpath_rotate_to(&s_data.hour_arrow, s_data.dow_theta);
  gpath_draw_filled(ctx, &s_data.hour_arrow);
  //gpath_draw_outline(ctx, &s_data.hour_arrow);
  
  // minute hand
  s_data.dow_offset.x = center.x + (int)(HR_LEN * sin_lookup(s_data.dow_theta) / TRIG_MAX_RATIO);
  s_data.dow_offset.y = center.y - (int)(HR_LEN * cos_lookup(s_data.dow_theta) / TRIG_MAX_RATIO);
  
  gpath_move_to(&s_data.minute_arrow, s_data.dow_offset);
  gpath_rotate_to(&s_data.minute_arrow, TRIG_MAX_ANGLE * t.tm_min / 60);
  gpath_draw_filled(ctx, &s_data.minute_arrow);
  gpath_draw_outline(ctx, &s_data.minute_arrow);

  // dot in the middle
  //graphics_context_set_fill_color(ctx, GColorBlack);
  //graphics_fill_rect(ctx, GRect(center.x, center.y-1, 2, 2), 0, GCornerNone);
  //graphics_fill_rect(ctx, GRect(s_data.dow_offset.x-1, s_data.dow_offset.y-1, 2, 2), 0, GCornerNone);
}

static void dow_update_proc(Layer* me, GContext* ctx) {
  PblTm t;
  get_time(&t);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  int x = (int)(20.5 * ((DAY_FIRST + t.tm_wday) % 7));

  gpath_move_to(&s_data.dow_arrow, GPoint(x, 0));
  gpath_draw_filled(ctx, &s_data.dow_arrow);
  gpath_draw_outline(ctx, &s_data.dow_arrow);

}

static void handle_init(AppContextRef app_ctx) {
  window_init(&s_data.window, "Vector Watch");

  // init paths
  gpath_init(&s_data.minute_arrow, &MINUTE_HAND_POINTS);
  gpath_init(&s_data.hour_arrow, &HOUR_HAND_POINTS);

  const GPoint center = grect_center_point(&s_data.window.layer.bounds);
  gpath_move_to(&s_data.minute_arrow, center);
  gpath_move_to(&s_data.hour_arrow, center);

  // init layers
  layer_init(&s_data.simple_bg_layer, s_data.window.layer.frame);
  s_data.simple_bg_layer.update_proc = &bg_update_proc;
  layer_add_child(&s_data.window.layer, &s_data.simple_bg_layer);

  // init hands
  layer_init(&s_data.hands_layer, s_data.simple_bg_layer.frame);
  s_data.hands_layer.update_proc = &hands_update_proc;
  layer_add_child(&s_data.window.layer, &s_data.hands_layer);

  // init dow
  gpath_init(&s_data.dow_arrow, &DOW_POINTS);
  s_data.dow_layer.update_proc = &dow_update_proc;
  layer_add_child(&s_data.window.layer, &s_data.dow_layer);

  // Push the window onto the stack
  const bool animated = true;
  window_stack_push(&s_data.window, animated);
}

static void handle_minute_tick(AppContextRef ctx, PebbleTickEvent* t) {
  layer_mark_dirty(&s_data.window.layer);
}

void pbl_main(void* params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
