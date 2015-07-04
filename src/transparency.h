#pragma once
//bool create_shadow_table();
//void destroy_shadow_table();
uint8_t combine_colors(uint8_t color1, uint8_t color2);
void fill_rect(GContext *ctx, GRect rect, uint8_t color);
void shadow_rect(GContext *ctx, GRect rect, uint8_t alpha);

void pattern_offset   (uint8_t *data, int8_t x_offset, int8_t y_offset, uint8_t invert);
void  fill_window     (GContext *ctx, uint8_t *data);
//void logshadowtable ();