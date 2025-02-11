#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>

#include <zephyr/display/mb_display.h>

static const struct pwm_dt_spec pwm = PWM_DT_SPEC_GET(DT_PATH(zephyr_user));
static const struct device *i2c;

int main(void)
{
	uint8_t val[3];
	struct mb_display *disp = mb_display_get();
        int x,y;

	// PWMの初期化
	if (!pwm_is_ready_dt(&pwm)) {
		printk("%s: device not ready.\n", pwm.dev->name);
		return 0;
	}

	// I2Cの初期化
	i2c = DEVICE_DT_GET(DT_NODELABEL(i2c1));
	k_msleep(1000);

	while (1) {
		k_msleep(100);
		// I2Cアドレス0x52のレジスタ0x00から3バイト読み込み
		i2c_burst_read(i2c,0x52,0x00,val,3);

		// 結果の出力
		printk("(%d,%d):%d\n",val[0],val[1],val[2]);

		// ディスプレイに表示  
                x = 4-(int)(val[0]*5/255);
                y = (int)(val[1]*5/255);
		struct mb_image pixel = {};
		pixel.row[y] = BIT(x);
		mb_display_image(disp, MB_DISPLAY_MODE_SINGLE, 250, &pixel, 1);

		// ジョイスティックのボタンが押されているときの処理
		//// 画面に"B"を表示して、音を出す
		if(val[2]) {
			mb_display_print(disp, MB_DISPLAY_MODE_SINGLE, 1 * MSEC_PER_SEC, "B");
			pwm_set_dt(&pwm, pwm.period, pwm.period / 2U);
			k_sleep(K_MSEC(60));
			pwm_set_dt(&pwm, 0, 0);
		}
	}
}
