#include "pebble.h"
#include "transparency.h"

static Window *main_window;
static Layer *root_layer;
static GBitmap *dog_bitmap;
uint8_t rect1_color=0b01100011, rect2_color=0b01110000;

// ------------------------------------------------------------------------ //
//  Button Functions
// ------------------------------------------------------------------------ //
void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {}
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
  graphics_draw_bitmap_in_rect(ctx, dog_bitmap, GRect(0,0,144,168));
  fill_rect(ctx, GRect(50, 50, 60, 60), rect1_color);
  fill_rect(ctx, GRect(10, 10, 70, 120), rect2_color);
  shadow_rect(ctx, GRect(0, 74, 144, 20), 0b01000000);  // shadow bar in the middle
  
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
  window_set_fullscreen(main_window, true);
  
  srand(time(NULL));
  dog_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOG);
  create_shadow_table();
  
  //Begin
  window_stack_push(main_window, true); // Display window (true = animated)
}
  
static void deinit(void) {
  destroy_shadow_table();
  gbitmap_destroy(dog_bitmap);
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
