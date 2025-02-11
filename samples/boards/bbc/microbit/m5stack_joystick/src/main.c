#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>

static const struct device *i2c;

int main(void)
{
	uint8_t val[3];

	// I2Cの初期化
	i2c = DEVICE_DT_GET(DT_NODELABEL(i2c1));
	k_msleep(1000);

	while (1) {
		k_msleep(100);
		// I2Cアドレス0x52のレジスタ0x00から3バイト読み込み
		i2c_burst_read(i2c,0x52,0x00,val,3);

		// 結果の出力
		printk("(%d,%d):%d\n",val[0],val[1],val[2]);
	}
}
