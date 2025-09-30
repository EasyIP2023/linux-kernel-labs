// SPDX-License-Identifier: GPL-2.0

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

static int serial_uart_probe(struct platform_device *device) {
	pr_info("In probe function.\n");
	return 0;
}

static void serial_uart_remove(struct platform_device *device) {
	pr_info("In remove function.\n");
}

static const struct of_device_id serial_uart_dt_match[] = {
    { .compatible = "underview,serial-uart" },
    { },
};

/* This macro describes which devices each specific driver can support. */
MODULE_DEVICE_TABLE(of, serial_uart_dt_match);

static struct platform_driver serial_uart_driver = {
    .driver = {
        .name = "serial-uart",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(serial_uart_dt_match)
    },
    .probe = serial_uart_probe,
    .remove = serial_uart_remove,
};

module_platform_driver(serial_uart_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Underview");
