// SPDX-License-Identifier: GPL-2.0

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/input.h>


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

static void nunchuk_poll(struct input_dev *input) {
	u8 recv_buf[6];
	int recv_bufsz = sizeof(recv_buf) / sizeof(u8);
	int zpressed = 0, cpressed = 0, xaxis = 0, yaxis = 0;

	/* Retrieve address that points to i2c device */
	struct i2c_client *client = (struct i2c_client *) input_get_drvdata(input);

	/* Retrieve the real state of the device */
	if (nunchuk_read_registers(client, recv_buf, recv_bufsz) < 0)
		return;

	// Can find in nunchuk.pdf
	xaxis = recv_buf[0]; // X-axis data of the joystick
	yaxis = recv_buf[1]; // Y-axis data of the joystick
	zpressed = (recv_buf[5] & BIT(0)) ? 0 : 1;
	cpressed = (recv_buf[5] & BIT(1)) ? 0 : 1;

	/* Send events to the INPUT subsystem */
	input_report_key(input, BTN_Z, zpressed);
	input_report_key(input, BTN_C, cpressed);
	input_report_abs(input, ABS_X, xaxis);
	input_report_abs(input, ABS_Y, yaxis);

	input_sync(input);
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
	u8 buf[2];
	int bufsz = sizeof(buf) / sizeof(u8);

	/*
	 * Input subsystem can be broken into two parts
	 * Device Drivers: talk to hardware and provide events to the input core
	 * Event Handlers: acquire events from drivers and pass them where needed
	 *                 mostly through evdev
	 */
	struct input_dev *input; // For device driver
	//struct input_handler *inp_handler = NULL; // For the event handler

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
	 * Allocate and associate struct input_dev memory with client (struct device)
	 * struct input_dev automatically free'd
	 */
	input = devm_input_allocate_device(&client->dev);
	if (!input) {
		dev_err(&client->dev, "devm_input_allocate_device: failed allocate struct input_dev instance\n");
		return -ENOMEM;
	}

	/*
	 * Set address that points to i2c client device as we will need it
	 * later in nunchuk_poll to retrieve the state of the device.
	 */
	input_set_drvdata(input, client);

	/*
	 * For buttons only EV_KEY
	 */
	input->name = "Wii Nunchuk";
	input->id.bustype = BUS_I2C;

	// EV_KEY: used for keys and buttons
	set_bit(EV_KEY, input->evbit); // Sets the type of event the device can generate
	set_bit(BTN_C, input->keybit); // Event Code
	set_bit(BTN_Z, input->keybit);

	// EV_ABS: Absolute values supplied by the device
	set_bit(EV_ABS, input->evbit); // Set the type of event the device can generate
	set_bit(ABS_X, input->absbit); // Event Code
	set_bit(ABS_Y, input->absbit);

	/*
	 * Populate fields in the input_dev struct for each absolute axis the device has
	 *
	 * input->absmin[ABS_X] = 30;
	 * input->absmax[ABS_X] = 220;
	 * input->absfuzz[ABS_X] = 4;
	 * input->absflat[ABS_X] = 8;
	 */
	input_set_abs_params(input, ABS_X, 30, 220, 4, 8);
	input_set_abs_params(input, ABS_Y, 40, 200, 4, 8);

	/* Classic buttons: To make the joystick usable */
	set_bit(BTN_TL, input->keybit);
	set_bit(BTN_SELECT, input->keybit);
	set_bit(BTN_MODE, input->keybit);
	set_bit(BTN_START, input->keybit);
	set_bit(BTN_TR, input->keybit);
	set_bit(BTN_TL2, input->keybit);
	set_bit(BTN_B, input->keybit);
	set_bit(BTN_Y, input->keybit);
	set_bit(BTN_A, input->keybit);
	set_bit(BTN_X, input->keybit);
	set_bit(BTN_TR2, input->keybit);

	/* Register and configure polling function */
	ret = input_setup_polling(input, nunchuk_poll);
	if (ret < 0) {
		dev_err(&client->dev, "input_setup_polling: failed status %d\n", ret);
		return ret;
	}

	input_set_poll_interval(input, 50);

	/* Register a (struct device) with the input framework */
	ret = input_register_device(input);
	if (ret < 0) {
		dev_err(&client->dev, "input_register_device: failed status %d\n", ret);
		return ret;
	}

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
	.probe_new = nunchuk_probe,
	.remove = nunchuk_remove,
};

module_i2c_driver(nunchuk_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Underview");
