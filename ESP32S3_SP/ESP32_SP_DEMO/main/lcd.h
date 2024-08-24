#ifndef __LCD_H__
#define __LCD_H__

#include "pretty_effect.h"

void lcd_spi_init(spi_device_handle_t * spi);
void lcd_init(spi_device_handle_t spi);
void display_pretty_colors(spi_device_handle_t spi);

#endif

