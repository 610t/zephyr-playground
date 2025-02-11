#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>

#include <zephyr/display/mb_display.h>

#include "lsm303_ll.h"

static const struct pwm_dt_spec pwm = PWM_DT_SPEC_GET(DT_PATH(zephyr_user));

int main(void)
{
	int ret;
	int x_accel,y_accel,z_accel;
	struct mb_display *disp = mb_display_get();
        int x,y;

	// 加速度センサーの初期化
	ret = lsm303_ll_begin();
	if (ret < 0) {
		printf("\nError initializing lsm303.  Error code = %d\n",ret);  
		while(1);
	}

	// PWMの初期化
	if (!pwm_is_ready_dt(&pwm)) {
		printk("%s: device not ready.\n", pwm.dev->name);
		return 0;
	}

	while (1) {
		k_msleep(100);

		// 加速度センサーの値を取得
		x_accel = lsm303_ll_readAccelX();
		y_accel = lsm303_ll_readAccelY();
		z_accel = lsm303_ll_readAccelZ();

		// 結果の出力
		printk("(%d,%d,%d)\n",x_accel,y_accel,z_accel);

		// ディスプレイに表示  
                x = (int)(x_accel+1000)*5/2000;
                y = 4-(int)(y_accel+1000)*5/2000;
		struct mb_image pixel = {};
		pixel.row[y] = BIT(x);
		mb_display_image(disp, MB_DISPLAY_MODE_SINGLE, 250, &pixel, 1);
	}
}
