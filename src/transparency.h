#pragma once
bool create_shadow_table();
void destroy_shadow_table();
uint8_t combine_colors(uint8_t color1, uint8_t color2);
void fill_rect(GContext *ctx, GRect rect, uint8_t color);