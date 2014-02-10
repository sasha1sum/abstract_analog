#include "abstract_analog.h"

#include "pebble.h"

#include "string.h"
#include "stdlib.h"

static GPath *minute_arrow;
static GPath *hour_arrow;
static GPath *dow_arrow;

Layer *simple_bg_layer;
Layer *hands_layer;
Layer *dow_layer;

Window *window;

static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const GPoint center = grect_center_point(&bounds);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  GPoint hourPt;

  int32_t hourAngle = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 60) + t->tm_min) / (12 * 60);

  hourPt.x = center.x + (int)(HR_LEN * sin_lookup(hourAngle) / TRIG_MAX_RATIO);
  hourPt.y = center.y - (int)(HR_LEN * cos_lookup(hourAngle) / TRIG_MAX_RATIO);

  // minute/hour hand
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  gpath_move_to(hour_arrow, center);
  gpath_rotate_to(hour_arrow, hourAngle);
  gpath_draw_filled(ctx, hour_arrow);

  gpath_move_to(minute_arrow, hourPt);
  gpath_rotate_to(minute_arrow, (int32_t)(TRIG_MAX_ANGLE * t->tm_min / 60));
  gpath_draw_filled(ctx, minute_arrow);
  gpath_draw_outline(ctx, minute_arrow);
}

static void dow_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  int x = (int)(20.5 * (t->tm_wday % 7));

  gpath_move_to(dow_arrow, GPoint(x, 0));
  gpath_draw_filled(ctx, dow_arrow);
  gpath_draw_outline(ctx, dow_arrow);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // background layer
  simple_bg_layer = layer_create(bounds);
  layer_set_update_proc(simple_bg_layer, bg_update_proc);
  layer_add_child(window_layer, simple_bg_layer);

  // dow layer
  dow_layer = layer_create(GRect(0,0,144,12));
  layer_set_update_proc(dow_layer, dow_update_proc);
  layer_add_child(window_layer, dow_layer);

  // hands
  hands_layer = layer_create(GRect(0,12,144,144));
  layer_set_update_proc(hands_layer, hands_update_proc);
  layer_add_child(window_layer, hands_layer);
}

static void window_unload(Window *window) {
  layer_destroy(simple_bg_layer);
  layer_destroy(dow_layer);
  layer_destroy(hands_layer);
}

static void init(void) {
  window = window_create();

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  // init hand paths
  minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  hour_arrow = gpath_create(&HOUR_HAND_POINTS);
  dow_arrow = gpath_create(&DOW_POINTS);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  const GPoint center = grect_center_point(&bounds);

  gpath_move_to(hour_arrow, center);

  // Push the window onto the stack
  const bool animated = false;
  window_stack_push(window, animated);

  tick_timer_service_subscribe(SECOND_UNIT, handle_minute_tick);
}

static void deinit(void) {
  gpath_destroy(dow_arrow);
  gpath_destroy(minute_arrow);
  gpath_destroy(hour_arrow);

  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
