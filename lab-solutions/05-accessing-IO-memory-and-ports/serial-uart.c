// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/processor.h>
#include <linux/serial_reg.h>
#include <linux/pm_runtime.h> /* pm_*() */
#include <linux/of.h>
#include <linux/io.h> /* readl()/writel() */

struct serial_uart {
	void __iomem *regs;
};

static unsigned int read_reg (struct serial_uart *serial, unsigned int reg) {
	return readl(serial->regs + (reg*4));
}

static void write_reg (struct serial_uart *serial, unsigned int reg, u32 val) {
	writel(val, serial->regs + (reg*4));
}

static void write_to_serial (struct serial_uart *serial, unsigned char val) {
	/* Poll the Line Status Register (LSR) */
	while (!(read_reg(serial, UART_LSR) & UART_LSR_THRE)) {
		cpu_relax();
	}

	write_reg(serial, UART_TX, val);
}

static int serial_uart_probe(struct platform_device *pdev) {
	int ret;

	u32 uartclk, baud_divisor;

	struct serial_uart *serial;

	serial = devm_kzalloc(&pdev->dev, sizeof(struct serial_uart), GFP_KERNEL);
	if (!serial)
		return -ENOMEM;

	serial->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(serial->regs))
		return PTR_ERR(serial->regs);

	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	/*
	 * How we acquire a property defined in
	 * am335x-customboneblack.dts.
	 */
	ret = of_property_read_u32(pdev->dev.of_node, "clock-frequency", &uartclk);
	if (ret) {
		pm_runtime_disable(&pdev->dev);
		dev_err(&pdev->dev, "clock-frequency property not found in Device Tree\n");
		return ret;
	}

	/* Configure the baud rate to 115200 */
	baud_divisor = uartclk / 16 / 115200;
	write_reg(serial, UART_OMAP_MDR1, 0x07);
	write_reg(serial, UART_LCR, 0x00);
	write_reg(serial, UART_LCR, UART_LCR_DLAB);
	write_reg(serial, UART_DLL, baud_divisor & 0xff);
	write_reg(serial, UART_DLM, (baud_divisor >> 8) & 0xff);
	write_reg(serial, UART_LCR, UART_LCR_WLEN8);
	write_reg(serial, UART_OMAP_MDR1, 0x00);

	/* Clear UART FIFOs */
	write_reg(serial, UART_FCR, UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);

	write_to_serial(serial, 'B');

	return 0;
}

static void serial_uart_remove(struct platform_device *pdev) {
	pm_runtime_disable(&pdev->dev);
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
