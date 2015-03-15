#include "pebble.h"
#include "transparency.h"

static uint8_t* shadowtable;

// Allocates and builds a 4x64 array (was shadowtable[4][64])
// Returns true if it can allocate the memory, false if it can't
//How it works:
// Calculates, for all 64 colors, and all 4 opacities, what a specified color would be if a specific opacity were applied
// Starts by assuming color is 100% of what it is (cause it is)
// full strength color, 100%, is easy: it's the same color.
// shading it to 0b10rrggbb (66% of full strength) takes each r, g and b and subtracts 1 (unless it's 0, then it stays 0)
// shading it to 0b01rrggbb (33% of full strength) takes 66%'s and subtracts 1 from each r g b again.
// shading it to 0b00rrggbb ( 0% of full strength) is easy, it's just 0 (well, an opaque 0b11000000)
bool create_shadow_table() {
  uint8_t r, g, b;
  shadowtable = malloc(256);
  if(shadowtable) {
    for(uint8_t i=0; i<64; i++) {
      shadowtable[0b11000000+i] = 0b11000000+i;     // 100%
      r = i & 0b00110000;
      g = i & 0b00001100;
      b = i & 0b00000011;
      if(r>0) r-=0b00010000;
      if(g>0) g-=0b00000100;
      if(b>0) b-=0b00000001;
      shadowtable[0b10000000+i] = 0b11000000+r+g+b; // 66%
      if(r>0) r-=0b00010000;
      if(g>0) g-=0b00000100;
      if(b>0) b-=0b00000001;
      shadowtable[0b01000000+i] = 0b11000000+r+g+b; // 33%
      shadowtable[0b00000000+i] = 0b11000000;       // 0%
    }
  } else {
    return false;
  }
  return true;
}

void destroy_shadow_table() {
   free(shadowtable); 
}

// #define combine_colors(color1, color2) ((shadowtable[((~color2)&0b11000000) + (color1&63)]&63) + shadowtable[color2])  // macro if you don't want to use the function

// Returns what color a pixel would be if a foreground rgba color is applied to a background rgb (no transparency for background)
//How it works:
// basically takes two rgb colors and one alpha, applies alpha to one color and ~alpha to other and adds them.
// google "Alpha compositing": Result = (rgb * alpha) + (rgb * (100% - alpha))
//
// First, things to know:
// argb = 0bAARRGGBB, the Alpha Channel is the first two bits (0bAA......)
// 00 =   0% (fully transparent)
// 01 =  33% (very transparent)
// 10 =  66% (less transparent)
// 11 = 100% (not transparent)
// Inverting all the alpha bits (heh) gives the opposite alpha amount (100% - transparency)
//  e.g. if it was 66%(10) it's now 33%(01).  if it was 0%(00) it's now 100%(11)
// 
//Ok, how it works:
// (~fg_color):      Alpha channel is extracted from foreground color, and inverted (giving us 100% - transparency)
// &0b11000000:      Mask out the alpha channel after inverting it
// bg_color&63:      Background color is stripped of alpha channel (is now just rgb)
// shadowtable[...]: shadowtable lookup gives us the color fg_color would be at the given transparency (built into fg_color's argb)
// shadowtable[...]: shadowtable lookup gives us the color bg_color would be at the given inverted transparency (extracted from fg_color's argb, then inverted)
// &63:              if shadowtable returned 0b00RRGGBB instead of 0b11RRGGBB, you could add both and add 0b11000000, but
//                   I decided to return 0b11RRGGBB cause it's useful to return a color under a shadow.
//                   So now you gotta either &63 both of 'em then add 0b11000000, or just &63 one of 'em.
uint8_t combine_colors(uint8_t bg_color, uint8_t fg_color) {
  return (shadowtable[((~fg_color)&0b11000000) + (bg_color&63)]&63) + shadowtable[fg_color];
}

void fill_rect(GContext *ctx, GRect rect, uint8_t color) {
  if(shadowtable) {       // make sure create_shadow_table was already successfully called
    #ifdef PBL_COLOR
      GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);
      if(framebuffer) {   // if successfully captured the framebuffer
        uint8_t* screen = gbitmap_get_data(framebuffer);
        for(uint16_t y_addr=rect.origin.y*144, row=0; row<rect.size.h; y_addr+=144, row++)
          for(uint16_t x_addr=rect.origin.x, x=0; x<rect.size.w; x_addr++, x++)
            screen[y_addr+x_addr] = combine_colors(screen[y_addr+x_addr], color);
        graphics_release_frame_buffer(ctx, framebuffer);
      }
    #else
      graphics_context_set_fill_color(ctx, color>>7);
      graphics_fill_rect(ctx, rect, 0, GCornerNone);
    #endif
  }
}

// Shades a region on the screen (darkening colors by an alpha amount)
void shadow_rect(GContext *ctx, GRect rect, uint8_t alpha) {
  if(shadowtable) {       // make sure create_shadow_table was already successfully called
    #ifdef PBL_COLOR
      GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);
      if(framebuffer) {   // if successfully captured the framebuffer
        uint8_t* screen = gbitmap_get_data(framebuffer);
        alpha &= 0b11000000; // sanitize alpha input
        for(uint16_t y_addr=rect.origin.y*144, row=0; row<rect.size.h; y_addr+=144, row++)
          for(uint16_t x_addr=rect.origin.x, x=0; x<rect.size.w; x_addr++, x++)
            screen[y_addr+x_addr] = shadowtable[alpha + (screen[y_addr+x_addr]&63)];
        graphics_release_frame_buffer(ctx, framebuffer);
      }
    #else
      graphics_context_set_fill_color(ctx, alpha>>7);
      graphics_fill_rect(ctx, rect, 0, GCornerNone);
    #endif
  }
}

// Same as above, but I like it better
// void fill_rect(GContext *ctx, GRect rect, uint8_t color) {
//   #ifdef PBL_COLOR
//     uint8_t bg_opacity = (~color)&0b11000000;
//     color = shadowtable[color]&63
//     uint8_t* screen = ((uint8_t*)*(uint32_t*)ctx);
//     for(uint16_t y_addr=rect.origin.y*144, row=0; row<rect.size.h; y_addr+=144, row++)
//       for(uint16_t x_addr=rect.origin.x, x=0; x<rect.size.w; x_addr++, x++)
//         screen[y_addr+x_addr] = shadowtable[bg_opacity + (screen[y_addr+x_addr]&63)] + color;
//   #else
//     graphics_context_set_fill_color(ctx, color>>7);
//     graphics_fill_rect(ctx, rect, 0, GCornerNone);
//   #endif
// }