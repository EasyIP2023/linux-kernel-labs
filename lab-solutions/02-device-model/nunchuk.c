// SPDX-License-Identifier: GPL-2.0

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>

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
	pr_info("Called %s\n", __func__);
	return 0;
}

static void nunchuk_remove(struct i2c_client *client)
{
	pr_info("Called %s\n", __func__);
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
