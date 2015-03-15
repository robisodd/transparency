#include "pebble.h"
#include "transparency.h"

static uint8_t shadowtable[256];

void build_shadow_table() {
  uint8_t r, g, b;
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
}

uint8_t combine_colors(uint8_t color1, uint8_t color2) {
  return (shadowtable[((~color2)&0b11000000) + (color1&63)]&63) + shadowtable[color2];
}

void fill_rect(GContext *ctx, GRect rect, uint8_t color) {
  #ifdef PBL_COLOR
    uint8_t bg_opacity = (~color)&0b11000000;
    uint8_t* screen = ((uint8_t*)*(uint32_t*)ctx);
    for(uint16_t y_addr=rect.origin.y*144, row=0; row<rect.size.h; y_addr+=144, row++)
      for(uint16_t x_addr=rect.origin.x, x=0; x<rect.size.w; x_addr++, x++)
        screen[y_addr+x_addr] = (shadowtable[bg_opacity + (screen[y_addr+x_addr]&63)]&63) + shadowtable[color];
      //screen[y_addr+x_addr] = combine_colors(screen[y_addr+x_addr], color);
  #else
    graphics_context_set_fill_color(ctx, color>>7);
    graphics_fill_rect(ctx, rect, 0, GCornerNone);
  #endif
}