// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/serial_reg.h>
#include <linux/io.h> /* readl()/writel() */

struct serial_uart {
	void __iomem *regs;
};

static unsigned int reg_read (struct serial_uart *serial, unsigned int reg) {
	return readl(serial->regs + (reg*4));
}

static void write_reg (struct serial_uart *serial, unsigned int reg, u32 val) {
	writel(val, serial->regs + (reg*4));
}

static int serial_uart_probe(struct platform_device *pdev) {
	struct serial_uart *serial;

	serial = devm_kzalloc(&pdev->dev, sizeof(struct serial_uart), GFP_KERNEL);
	if (!serial)
		return -ENOMEM;

	serial->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(serial->regs))
		return PTR_ERR(serial->regs);

	pr_info("End of probe\n");

	return 0;
}

static void serial_uart_remove(struct platform_device *pdev) {
	pr_info("End of remove\n");
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
MODULE_DESCRIPTION("Serial Uart Driver Implementation");
