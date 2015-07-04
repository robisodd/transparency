#include "pebble.h"
#include "transparency.h"

#define UPDATE_MS 50
  
static Window *main_window;
static Layer *root_layer;
static GBitmap *dog_bitmap;
uint8_t rect1_color=0b10000011, rect2_color=0b10110000;
//uint8_t rect1_color=0b01100011, rect2_color=0b01110000;
uint8_t *pattern;
 uint8_t brickpattern[] = {187, 95,174, 93,186,117,234,245, 0b11110000, 0b11000000};  // Brick
//uint8_t brickpattern[] = { 64,192,200,120,120, 72,  0,  0}; // "Scottie"

// ------------------------------------------------------------------------ //
//  Timer Functions
// ------------------------------------------------------------------------ //
static void timer_callback(void *data) {
  pattern_offset(pattern, 2, 0, 0);
  layer_mark_dirty(root_layer);  // Schedule redraw of screen
  app_timer_register(UPDATE_MS, timer_callback, NULL); // Schedule a callback
}

// ------------------------------------------------------------------------ //
//  Button Functions
// ------------------------------------------------------------------------ //
void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {}//{logshadowtable();}
void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  rect1_color = rand() % 255;
  rect2_color = rand() % 255;
  layer_mark_dirty(root_layer);  // Schedule redraw of screen
}
void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {}
  
void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
}


// ------------------------------------------------------------------------ //
//  Drawing Functions
// ------------------------------------------------------------------------ //
static void root_layer_update(Layer *me, GContext *ctx) {
//   graphics_draw_bitmap_in_rect(ctx, dog_bitmap, GRect(0,0,144,168));
  fill_window(ctx, pattern);  // Fill window with Brick, inverted and shifted
  
  fill_rect(ctx, GRect(50, 50, 60, 60), rect1_color);
  fill_rect(ctx, GRect(10, 10, 70, 120), rect2_color);

  shadow_rect(ctx, GRect(0, 74, 144, 20), 0b01111100);  // shadow bar in the middle
}
  
// ------------------------------------------------------------------------ //
//  Main Functions
// ------------------------------------------------------------------------ //
static void main_window_load(Window *window) {
  root_layer = window_get_root_layer(window);
  layer_set_update_proc(root_layer, root_layer_update);
}

static void init(void) {
  // Set up and push main window
  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load   = main_window_load
  });
  window_set_click_config_provider(main_window, click_config_provider);
  //window_set_fullscreen(main_window, true);
  
  srand(time(NULL));
  dog_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOG);
  //create_shadow_table();
  
  pattern = brickpattern;
  pattern_offset(pattern, 0, 4, 255); // invert brick pattern

  //Begin
  window_stack_push(main_window, true); // Display window (true = animated)
  app_timer_register(UPDATE_MS, timer_callback, NULL); // Schedule a callback

}
  
static void deinit(void) {
  //destroy_shadow_table();
  gbitmap_destroy(dog_bitmap);
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
