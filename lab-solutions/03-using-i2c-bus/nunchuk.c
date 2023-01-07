// SPDX-License-Identifier: GPL-2.0

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>

/*
 * probe function responsible for:
 *
 * Initializing the device
 * Register the device to the proper kernel framework
 * Mapping I/O memory
 * Registering the interrupt handlers
 */
static int nunchuk_probe(struct i2c_client *client)
{
	int ret = 0;
	u8 buf[2];

	/* Initialize device */
	buf[0] = 0xf0;
	buf[1] = 0x55;

	ret = i2c_master_send(client, buf, 2);
	if (ret < 0) {
		dev_err(&client->dev, "i2c_master_send: failed status %d\n", ret);
		return ret;
	}

	udelay(1000);

	buf[0] = 0xfb;
	buf[1] = 0x00;

	ret = i2c_master_send(client, buf, 2);
	if (ret < 0) {
		dev_err(&client->dev, "i2c_master_send: failed status %d\n", ret);
		return ret;
	}

	return 0;
}

static void nunchuk_remove(struct i2c_client *client)
{

}

/*
 * Specification of supported Device Tree devices
 * to bind to the driver
 */
static const struct of_device_id nunchuk_dt_match[] = {
	{ .compatible = "nintendo,nunchuk" },
	{ },
};

/* This macro describes which devices each specific driver can support. */
MODULE_DEVICE_TABLE(of, nunchuk_dt_match);

/* Driver declaration */
static struct i2c_driver nunchuk_driver = {
	.driver = {
		.name = "nunchuk",
		.of_match_table = nunchuk_dt_match,
	},
	.probe_new = nunchuk_probe,
	.remove = nunchuk_remove,
};

module_i2c_driver(nunchuk_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Underview");
