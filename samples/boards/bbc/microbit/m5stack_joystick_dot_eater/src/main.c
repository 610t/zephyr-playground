#include <stdio.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>

#include <zephyr/display/mb_display.h>

bool game_flag = false;

static const struct gpio_dt_spec sw0_gpio = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct pwm_dt_spec pwm = PWM_DT_SPEC_GET(DT_PATH(zephyr_user));
static const struct device *i2c;

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

	// GPIOの初期化
	static struct gpio_callback button_cb_data;
	gpio_pin_configure_dt(&sw0_gpio, GPIO_INPUT|GPIO_ACTIVE_HIGH);
	gpio_pin_interrupt_configure_dt(&sw0_gpio, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_cb_data, button_pressed, BIT(sw0_gpio.pin));
	gpio_add_callback(sw0_gpio.port, &button_cb_data);

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

		for(int timer=0;timer<100;timer++) {
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
			pixel.row[e_y] = BIT(e_x);
			mb_display_image(disp, MB_DISPLAY_MODE_SINGLE, 250, &pixel, 1);
			printk("Pos:dot(%d,%d)==enemy(%d,%d)?\n",x,y,e_x,e_y);
			if(x==e_x && y==e_y) {
				score++;
				printk("score:%d\n",score);
				e_x=rand()%5;
				e_y=rand()%5;
				pwm_set_dt(&pwm, pwm.period, pwm.period / 2U);
				k_sleep(K_MSEC(60));
				pwm_set_dt(&pwm, 0, 0);
			}

			// ジョイスティックのボタンが押されているときの処理
			//// 画面に"B"を表示して、音を出す
			if(val[2]) {
				mb_display_print(disp, MB_DISPLAY_MODE_SINGLE, 1 * MSEC_PER_SEC, "B");
				pwm_set_dt(&pwm, pwm.period, pwm.period / 2U);
				k_sleep(K_MSEC(60));
				pwm_set_dt(&pwm, 0, 0);
			}
			k_msleep(25);
		}
		mb_display_print(disp, MB_DISPLAY_MODE_SINGLE, 0.25 * MSEC_PER_SEC, "GAME OVER!!");
		k_msleep(3000);

		mb_display_print(disp, MB_DISPLAY_MODE_SINGLE, 1 * MSEC_PER_SEC, "%d", score);
		k_msleep(3000);
		game_flag = false;
	}
}
