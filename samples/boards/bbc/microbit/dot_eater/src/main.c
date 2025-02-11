#include <stdio.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>

#include <zephyr/display/mb_display.h>

#include "lsm303_ll.h"

bool game_flag = false;

static const struct gpio_dt_spec sw0_gpio = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

// ボタンコールバック: ゲームスタート用
static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (pins & BIT(sw0_gpio.pin)) {
		printk("A pressed\n");
		game_flag = true;
	}
}

int main(void)
{
	int ret;
        int x,y;
	int x_accel,y_accel,z_accel;
	struct mb_display *disp = mb_display_get();
	static struct gpio_callback button_cb_data;

	// GPIOの初期化
	gpio_pin_configure_dt(&sw0_gpio, GPIO_INPUT|GPIO_ACTIVE_HIGH);
	gpio_pin_interrupt_configure_dt(&sw0_gpio, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_cb_data, button_pressed, BIT(sw0_gpio.pin));
	gpio_add_callback(sw0_gpio.port, &button_cb_data);

	// 加速度センサーの初期化
	ret = lsm303_ll_begin();
	if (ret < 0) {
		printf("\nError initializing lsm303.  Error code = %d\n",ret);  
		while(1);
	}

	int score=0;
	while (1) {
		int e_x,e_y;

		// Aボタンでゲーム開始
		while(!game_flag) {
			mb_display_print(disp, MB_DISPLAY_MODE_SINGLE, MSEC_PER_SEC, "A");
			k_msleep(100);
		}

		e_x=rand()%5;
		e_y=rand()%5;

		for(int timer=0;timer<400;timer++) {
			// 加速度センサーの値を取得
			x_accel = lsm303_ll_readAccelX();
			y_accel = lsm303_ll_readAccelY();
			z_accel = lsm303_ll_readAccelZ();
			printk("Accel:(%d,%d,%d),t:%d\n",x_accel,y_accel,z_accel,timer);

			// ディスプレイに表示  
			x = (int)(x_accel+1000)*5/2000;
			y = 4-(int)(y_accel+1000)*5/2000;
			struct mb_image pixel = {};
			pixel.row[y] = BIT(x);
			pixel.row[e_y] = BIT(e_x);
			mb_display_image(disp, MB_DISPLAY_MODE_SINGLE, SYS_FOREVER_US, &pixel, 1);
			printk("Pos:dot(%d,%d)==enemy(%d,%d)?\n",x,y,e_x,e_y);
			if(x==e_x && y==e_y) {
				score++;
				printk("score:%d\n",score);
				e_x=rand()%5;
				e_y=rand()%5;
			}
			k_msleep(25);
		}
		mb_display_print(disp, MB_DISPLAY_MODE_SINGLE, 0.25 * MSEC_PER_SEC, "GAME OVER!!");
		k_msleep(3000);

		mb_display_print(disp, MB_DISPLAY_MODE_SINGLE, 1 * MSEC_PER_SEC, "%d", score);
		k_msleep(2000);
		game_flag = false;
	}
}
