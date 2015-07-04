#include "pebble.h"
#include "transparency.h"

// static uint8_t* shadowtable;

// // Allocates and builds a 4x64 array (was shadowtable[4][64])
// // Returns true if it can allocate the memory, false if it can't
// //How it works:
// // Calculates, for all 64 colors, and all 4 opacities, what a specified color would be if a specific opacity were applied
// // Starts by assuming color is 100% of what it is (cause it is)
// // full strength color, 100%, is easy: it's the same color.
// // shading it to 0b10rrggbb (66% of full strength) takes each r, g and b and subtracts 1 (unless it's 0, then it stays 0)
// // shading it to 0b01rrggbb (33% of full strength) takes 66%'s and subtracts 1 from each r g b again.
// // shading it to 0b00rrggbb ( 0% of full strength) is easy, it's just 0 (well, an opaque 0b11000000)
// bool create_shadow_table() {
//   #ifdef PBL_COLOR
//     uint8_t r, g, b;
//     shadowtable = malloc(256);
//     if(shadowtable) {
//       for(uint8_t i=0; i<64; i++) {
//         shadowtable[0b11000000+i] = 0b11000000+i;     // 100%
//         r = i & 0b00110000;
//         g = i & 0b00001100;
//         b = i & 0b00000011;
//         if(r>0) r-=0b00010000;
//         if(g>0) g-=0b00000100;
//         if(b>0) b-=0b00000001;
//         shadowtable[0b10000000+i] = 0b11000000+r+g+b; // 66%
//         if(r>0) r-=0b00010000;
//         if(g>0) g-=0b00000100;
//         if(b>0) b-=0b00000001;
//         shadowtable[0b01000000+i] = 0b11000000+r+g+b; // 33%
//         shadowtable[0b00000000+i] = 0b11000000;       // 0%
//       }
//     } else {
//       return false;
//     }
//     return true;
//   #else
//     return false;  // No need for a table if black and white
//   #endif
// }
uint8_t shadowtable[] = {192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192, \
                         192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192, \
                         192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192, \
                         192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192, \
                         192,192,192,193,192,192,192,193,192,192,192,193,196,196,196,197, \
                         192,192,192,193,192,192,192,193,192,192,192,193,196,196,196,197, \
                         192,192,192,193,192,192,192,193,192,192,192,193,196,196,196,197, \
                         208,208,208,209,208,208,208,209,208,208,208,209,212,212,212,213, \
                         192,192,193,194,192,192,193,194,196,196,197,198,200,200,201,202, \
                         192,192,193,194,192,192,193,194,196,196,197,198,200,200,201,202, \
                         208,208,209,210,208,208,209,210,212,212,213,214,216,216,217,218, \
                         224,224,225,226,224,224,225,226,228,228,229,230,232,232,233,234, \
                         192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207, \
                         208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223, \
                         224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239, \
                         240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255};

// void logshadowtable() {
//   for(uint16_t i=0; i<256; i+=16) {
//     APP_LOG(APP_LOG_LEVEL_INFO, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", shadowtable[i], shadowtable[i+1], shadowtable[i+2], shadowtable[i+3], shadowtable[i+4], shadowtable[i+5], shadowtable[i+6], shadowtable[i+7], shadowtable[i+8], shadowtable[i+9], shadowtable[i+10], shadowtable[i+11], shadowtable[i+12], shadowtable[i+13], shadowtable[i+14], shadowtable[i+15]);
//   }
// }


// void destroy_shadow_table() {
//    free(shadowtable); 
// }

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
    GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);
    if(framebuffer) {   // if successfully captured the framebuffer
      uint8_t* screen = gbitmap_get_data(framebuffer);
      rect.size.w  += rect.origin.x; rect.size.h  += rect.origin.y;                      // convert rect.size.w and rect.size.h to rect.x2 and rect.y2
      rect.size.w   = rect.size.w   < 0 ? 0 : rect.size.w   > 144 ? 144 : rect.size.w;   // make sure rect.x2 is within screen bounds
      rect.origin.x = rect.origin.x < 0 ? 0 : rect.origin.x > 144 ? 144 : rect.origin.x; // make sure rect.x1 is within screen bounds
      rect.size.h   = rect.size.h   < 0 ? 0 : rect.size.h   > 168 ? 168 : rect.size.h;   // make sure rect.y2 is within screen bounds
      rect.origin.y = rect.origin.y < 0 ? 0 : rect.origin.y > 168 ? 168 : rect.origin.y; // make sure rect.y1 is within screen bounds
    
    #ifdef PBL_COLOR
      rect.origin.y*=144; rect.size.h*=144;
      //if(shadowtable)  // make sure create_shadow_table was already successfully called
        for (uint16_t y_addr=rect.origin.y; y_addr<rect.size.h; y_addr+=144)
          for(uint16_t x_addr=rect.origin.x; x_addr<rect.size.w; x_addr++)
            screen[y_addr+x_addr] = combine_colors(screen[y_addr+x_addr], color);
    #else
      //TODO: THIS IS BAD!  Need to completely redo this.
      // first two alpha bits = 00 = don't do anything, 11 = solid fill, 01 = & 10 = |, then rgb = black, white, 50%
      // if SolidFill, check rgb.  if rgb=0 then fill black, if 0b111111 fill white, if 0b101010 use 50% grey, if 0b010101 use other 50% grey
      if(color>0b00111111 && color<0b11111111) {        // TODO: FIX THIS MESSY IF.  if alpha is 00 = black, 01 or 10 = 50% gray or 11 = white
        uint8_t data[] = {170, 85};  // 50% Gray
        GPoint addr;
        addr.y = rect.origin.y*20;
        uint8_t l_mask = 255 << (rect.origin.x%8); // mask for the left side
        uint8_t r_mask = 255 << (rect.size.w%8);   // mask for the right side
        // screen = ((uint8_t*)(((GBitmap*)ctx)->addr))
        for (int16_t y=0; y<(rect.size.h-rect.origin.y); y++, addr.y+=20) {
          if(addr.y>=0 && addr.y<168*20) {   // if y row is on the screen
            addr.x = rect.origin.x>>3;       // init X memory address
            if(addr.x >= 0 && addr.x < 19) screen[addr.y + addr.x] = (data[y&1] & l_mask) + (screen[addr.y + addr.x] & ~l_mask); // fill left-side of row
            for(addr.x++; addr.x<(rect.size.w>>3); addr.x++) if(addr.x >= 0 && addr.x < 19) screen[addr.y + addr.x] = data[y&1]; // fill middle of row
            if(addr.x >= 0 && addr.x < 19) screen[addr.y + addr.x] = (screen[addr.y + addr.x] & r_mask) + (data[y&1] & ~r_mask); // fill right-side of row
          }
        }
      } else { // solid white if alpha=11 or solid black if alpha=00
        graphics_context_set_fill_color(ctx, color>>7); // set color to most significant bit
        graphics_fill_rect(ctx, rect, 0, GCornerNone);
      }
    #endif
    graphics_release_frame_buffer(ctx, framebuffer);
  }  // endif successfully captured framebuffer
}


// use below for notes
  uint8_t graydata[] = {170, 85};  // 50% Gray    
//uint8_t graydata[] = {85, 170};  // 50% Gray    
// Shades a region on the screen (darkening colors by an alpha amount)
// alpha should only be 0b??111111 where ?? = 00 (full shade), 01 (much shade), 10 (some shade), 11 (none shade)
void shadow_rect(GContext *ctx, GRect rect, uint8_t alpha) {
  alpha |= 0b00111111; // sanitize alpha input
  #ifdef PBL_COLOR
    GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);
    if(framebuffer) {   // if successfully captured the framebuffer
      uint8_t* screen = gbitmap_get_data(framebuffer);
      for(uint16_t y_addr=rect.origin.y*144, row=0; row<rect.size.h; y_addr+=144, ++row)
        for(uint16_t x_addr=rect.origin.x, x=0; x<rect.size.w; ++x_addr, ++x)
        screen[y_addr+x_addr] = shadowtable[alpha & screen[y_addr+x_addr]];
      graphics_release_frame_buffer(ctx, framebuffer);
    }
  #else
  //graydata[0]=~graydata[0];graydata[1]=~graydata[1];
    if(alpha>0 && alpha<0b11000000) {        // TODO: FIX THIS MESSY IF.  if alpha is 00 = black, 01 or 10 = 50% gray or 11 = white
      GPoint addr;
      rect.size.w  += rect.origin.x;
      rect.size.h  += rect.origin.y;
      rect.size.w   = rect.size.w   < 0 ? 0 : rect.size.w   > 144 ? 144 : rect.size.w;
      rect.origin.x = rect.origin.x < 0 ? 0 : rect.origin.x > 144 ? 144 : rect.origin.x;
      rect.size.h   = rect.size.h   < 0 ? 0 : rect.size.h   > 168 ? 168 : rect.size.h;
      rect.origin.y = rect.origin.y < 0 ? 0 : rect.origin.y > 168 ? 168 : rect.origin.y;
      addr.y = rect.origin.y*20;

      uint8_t l_mask = 255 << (rect.origin.x%8); // mask for the left side
      uint8_t r_mask = 255 << (rect.size.w%8);   // mask for the right side
      #define screens ((uint8_t*)(((GBitmap*)ctx)->addr))
      for (int16_t y=0; y<(rect.size.h-rect.origin.y); y++, addr.y+=20) {
        if(addr.y>=0 && addr.y<168*20) {   // if y row is on the screen
          addr.x = rect.origin.x>>3;       // init X memory address
          // note: change those 3 "|=" to "&=" to darken instead of lighten
          if(addr.x >= 0 && addr.x < 19) screens[addr.y + addr.x] |= (graydata[y&1] & l_mask) + (screens[addr.y + addr.x] & ~l_mask); // fill left-side of row
          //       for (addr.x++; addr.x<(rect.size.w>>3); addr.x++) if(addr.x >= 0 && addr.x < 19) screen[addr.y + addr.x] |= graydata[y&1];           // fill middle of row
          for (addr.x++; addr.x<(rect.size.w>>3); addr.x++) if(addr.x >= 0 && addr.x < 19) screens[addr.y + addr.x] = ~(screens[addr.y + addr.x] | graydata[y&1]);           // fill middle of row
          if(addr.x >= 0 && addr.x < 19) screens[addr.y + addr.x] |= (screens[addr.y + addr.x] & r_mask) + (graydata[y&1] & ~r_mask); // fill right-side of row
        }
      }
    } else { // solid white if alpha=11 or solid black if alpha=00
      graphics_context_set_fill_color(ctx, alpha>>7); // set color to most significant bit
      graphics_fill_rect(ctx, rect, 0, GCornerNone);
    }
  #endif
}

void pattern_offset(uint8_t *data, int8_t x_offset, int8_t y_offset, uint8_t invert) {  // invert = [0 or 255] for [no or yes] ([1-254] inverts vertical stripes based on bit mask)
  uint8_t temp[8];
  uint8_t mask = ((x_offset%8)+8)%8;  // sanatize x_offset [-128 to 127] to [0 to  7]
  uint8_t row  = ((y_offset%8)+8);    // sanatize y_offset [-128 to 127] to [0 to 15]
  for(uint8_t i=0; i<8; i++)
    temp[i] = ((data[(row+i)%8] << mask) + (data[(row+i)%8] >> (8-mask))) ^ invert; // shift 8bit segment x&y offset, then invert using XOR
  for(uint8_t i=0; i<8; i++) data[i] = temp[i];
}

void fill_window(GContext *ctx, uint8_t *data) {
//   uint8_t* screen = ((uint8_t*)*(uint32_t*)ctx);
     GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);
     uint8_t* screen = gbitmap_get_data(framebuffer);
//   #define screen ((uint8_t*)*(uint32_t*)ctx)
#ifdef PBL_COLOR
  for(uint16_t y_addr=0, row=0; y_addr<168*144; y_addr+=144, row=(row+1)&7)
    for(uint16_t x_addr=0; x_addr<144; x_addr++)
      screen[y_addr+x_addr] = ((data[row]>>(x_addr&7))&1) ? data[8] : data[9]; //colorwhite : colorblack;
#else
  for(uint16_t y_addr=0, row=0; y_addr<168*20; y_addr+=20, row=(row+1)&7)
    for(uint16_t x_addr=0; x_addr<19; x_addr++)
      screen[y_addr+x_addr] = data[row];
#endif
  graphics_release_frame_buffer(ctx, framebuffer);
}

// void (GContext *ctx, GRect rect, uint8_t color) {
//   if(shadowtable) {       // make sure create_shadow_table was already successfully called
//     #ifdef PBL_COLOR
//       GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);
//       if(framebuffer) {   // if successfully captured the framebuffer
//         uint8_t* screen = gbitmap_get_data(framebuffer);
//         for(uint16_t y_addr=rect.origin.y*144, row=0; row<rect.size.h; y_addr+=144, row++)
//           for(uint16_t x_addr=rect.origin.x, x=0; x<rect.size.w; x_addr++, x++)
//             screen[y_addr+x_addr] = combine_colors(screen[y_addr+x_addr], color);
//         graphics_release_frame_buffer(ctx, framebuffer);
//       }
//     #else
//       graphics_context_set_fill_color(ctx, color>>7);
//       graphics_fill_rect(ctx, rect, 0, GCornerNone);
//     #endif
//   }
// }


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