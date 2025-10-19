// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/processor.h>
#include <linux/serial_reg.h>
#include <linux/pm_runtime.h> /* pm_*() */
#include <linux/of.h>
#include <linux/io.h> /* readl()/writel() */
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

#include "uapi/serial-uart.h"

struct serial_uart {
	void __iomem *regs;
	char __user *buf;
	long char_count;
	struct miscdevice miscdev;
};

static unsigned int read_reg (struct serial_uart *serial, unsigned int reg) {
	usleep_range(50, 100);
	return readl(serial->regs + (reg*4));
}

static void write_reg (struct serial_uart *serial, unsigned int reg, u32 val) {
	usleep_range(50, 100);
	writel(val, serial->regs + (reg*4));
}

static ssize_t serial_read (struct file *file, char __user *data, size_t size, loff_t *offset) {
	return -EINVAL;
}

static ssize_t serial_write (struct file *file, const char __user *data, size_t size, loff_t *offset) {
	int ret;
	size_t s;
	char buf;
	unsigned int reg_val = 0;
	struct serial_uart *serial;

	serial = container_of(file->private_data, struct serial_uart, miscdev);

	if (size >= PAGE_SIZE) {
		return -EINVAL;
	}

	/* Poll the Line Status Register (LSR) */
	while (!reg_val) {
		reg_val = (read_reg(serial, UART_LSR) & UART_LSR_THRE);
		cpu_relax();
	}

	ret = copy_from_user(serial->buf, (char*) data, size);
	if (ret) {
		return -EFAULT;
	}

	for (s = 0; s < size; s++) {
		buf = serial->buf[s];
		write_reg(serial, UART_TX, (u32)buf);
		serial->char_count++;

		if (buf == '\n') {
			write_reg(serial, UART_TX, '\r');
			serial->char_count++;
		}
	}

	return size;
}

static long serial_ioctl (struct file *file, unsigned int cmd, unsigned long data) {
	long ret = 0;

	struct serial_uart *serial;

	serial = container_of(file->private_data, struct serial_uart, miscdev);

	switch (cmd) {
		case SERIAL_RESET_COUNTER:
			serial->char_count = 0;
			break;
		case SERIAL_GET_COUNTER:
			ret = serial->char_count;
			break;
		default:
			return -EINVAL;
	}

	return ret;
}

static int serial_uart_probe (struct platform_device *pdev) {
	int ret;

	struct resource *res;

	u32 uartclk, baud_divisor;

	struct serial_uart *serial;

	static const struct file_operations fops = {
		.owner = THIS_MODULE,
		.read = serial_read,
		.write = serial_write,
		.unlocked_ioctl = serial_ioctl,
	};

	serial = devm_kzalloc(&pdev->dev, sizeof(struct serial_uart), GFP_KERNEL);
	if (!serial)
		return -ENOMEM;

	serial->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(serial->regs))
		return PTR_ERR(serial->regs);

	serial->buf = (char *) get_zeroed_page(GFP_USER);
	if (!(serial->buf))
		return -ENOMEM;

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

	platform_set_drvdata(pdev, serial);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pm_runtime_disable(&pdev->dev);
		dev_err(&pdev->dev, "couldn't find resource\n");
		return -ENODEV;
	}

	serial->miscdev.name = devm_kasprintf(&pdev->dev, GFP_KERNEL, "serial-\%x", res->start);
	serial->miscdev.minor = MISC_DYNAMIC_MINOR;
	serial->miscdev.fops = &fops;
	serial->miscdev.mode = 0666;

	ret = misc_register(&serial->miscdev);
	if (ret) {
		pm_runtime_disable(&pdev->dev);
		dev_err(&pdev->dev, "failed to register with misc framework\n");
		return ret;
	}

	return 0;
}

static void serial_uart_remove (struct platform_device *pdev) {
	struct serial_uart *serial;

	serial = platform_get_drvdata(pdev);

	free_page((unsigned long)serial->buf);
	pm_runtime_disable(&pdev->dev);
	misc_deregister(&serial->miscdev);
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
