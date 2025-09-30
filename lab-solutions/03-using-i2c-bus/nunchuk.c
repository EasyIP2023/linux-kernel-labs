// SPDX-License-Identifier: GPL-2.0

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>

static int nunchuk_read_registers(struct i2c_client *client, u8 *recv_buf, int recv_bufsz) {
	int ret = 0;
	u8 buf = 0x00;

	/* add 10-20 ms delay */
	usleep_range(10000, 20000);

	/* Sending 0x00 to the bus allows for reading of the device registers. */
	ret = i2c_master_send(client, &buf, 1);
	if (ret < 0) {
		dev_err(&client->dev, "i2c_master_send: failed status %d\n", ret);
		return ret;
	}

	/* add another 10-20 ms delay to wait for response */
	usleep_range(10000, 20000);

	ret = i2c_master_recv(client, recv_buf, recv_bufsz);
	if (ret < 0) {
		dev_err(&client->dev, "i2c_master_recv: failed status %d\n", ret);
		return ret;
	}

	return 0;
}

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
	u8 buf[2], recv_buf[6];
	int bufsz = sizeof(buf) / sizeof(u8);
	int recv_bufsz = sizeof(recv_buf) / sizeof(u8);

	struct _buttons {
		int zpressed : 1;
		int cpressed : 1;
	} buttons;

	/* Initialize wii nunchuk register one */
	buf[0] = 0xf0;
	buf[1] = 0x55;

	ret = i2c_master_send(client, buf, bufsz);
	if (ret < 0) {
		dev_err(&client->dev, "i2c_master_send: failed status %d\n", ret);
		return ret;
	}

	udelay(1000);

	/* Initialize wii nunchuk register two */
	buf[0] = 0xfb;
	buf[1] = 0x00;

	ret = i2c_master_send(client, buf, bufsz);
	if (ret < 0) {
		dev_err(&client->dev, "i2c_master_send: failed status %d\n", ret);
		return ret;
	}

	/*
	 * Have to make a dummy read as the device seems to update the
	 * state of its internal registers only after they have been read.
	 */
	ret = nunchuk_read_registers(client, recv_buf, recv_bufsz);
	if (ret < 0)
		return ret;

	/* Retrieve the real state of the device */
	ret = nunchuk_read_registers(client, recv_buf, recv_bufsz);
	if (ret < 0)
		return ret;

	buttons.zpressed = (recv_buf[5] & BIT(0)) ? 0 : 1;
	if (buttons.zpressed)
		dev_info(&client->dev, "Z button pressed\n");

	buttons.cpressed = (recv_buf[5] & BIT(1)) ? 0 : 1;
	if (buttons.cpressed)
		dev_info(&client->dev, "C button pressed\n");

	return 0;
}

static void nunchuk_remove(struct i2c_client *client)
{
	return;
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
	.probe = nunchuk_probe,
	.remove = nunchuk_remove,
};

module_i2c_driver(nunchuk_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Underview");
