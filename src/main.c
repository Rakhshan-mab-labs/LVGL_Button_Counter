#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

#define SW0_NODE DT_ALIAS(sw0)
#define SW1_NODE DT_ALIAS(sw1)
#define SW2_NODE DT_ALIAS(sw2)
#define SW3_NODE DT_ALIAS(sw3)
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);
static const struct gpio_dt_spec button4 = GPIO_DT_SPEC_GET(SW3_NODE, gpios);

static volatile bool button_0_pressed_flag = false;
static volatile bool button_1_pressed_flag = false;
static volatile bool button_2_pressed_flag = false;
static volatile bool button_3_pressed_flag = false;

static struct gpio_callback button_cb_data_0;
static struct gpio_callback button_cb_data_1;
static struct gpio_callback button_cb_data_2;
static struct gpio_callback button_cb_data_3;

static uint32_t count = 0;

#define INC_VAL_1 1
#define INC_VAL_2 10
#define INC_VAL_3 100
#define INC_VAL_4 1000

void button_0_pressed()
{
	button_0_pressed_flag = true;
}

void button_1_pressed()
{
	button_1_pressed_flag = true;
}

void button_2_pressed()
{
	button_2_pressed_flag = true;
}

void button_3_pressed()
{
	button_3_pressed_flag = true;
}

int main(void)
{

	int ret0;
    int ret1;
    int ret2;
    int ret3;

	if (!device_is_ready(button1.port) || !device_is_ready(button2.port) ||
        !device_is_ready(button3.port) || !device_is_ready(button4.port)) {
		LOG_ERR("Button not ready");
		return -1;
	}

	/* Configure button as input with interrupt */
    ret0 = gpio_pin_configure_dt(&button1, GPIO_INPUT | GPIO_PULL_UP);
    ret1 = gpio_pin_configure_dt(&button2, GPIO_INPUT | GPIO_PULL_UP);
    ret2 = gpio_pin_configure_dt(&button3, GPIO_INPUT | GPIO_PULL_UP);
    ret3 = gpio_pin_configure_dt(&button4, GPIO_INPUT | GPIO_PULL_UP);
    if (ret0 < 0 || ret1 < 0 || ret2 < 0 || ret3 < 0) {
        LOG_ERR("Failed to configure button pins");
        return -1;
    }

    ret0 = gpio_pin_interrupt_configure_dt(&button1, GPIO_INT_EDGE_TO_ACTIVE);
    ret1 = gpio_pin_interrupt_configure_dt(&button2, GPIO_INT_EDGE_TO_ACTIVE);
    ret2 = gpio_pin_interrupt_configure_dt(&button3, GPIO_INT_EDGE_TO_ACTIVE);
    ret3 = gpio_pin_interrupt_configure_dt(&button4, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret0 < 0 || ret1 < 0 || ret2 < 0 || ret3 < 0) {
        LOG_ERR("Failed to configure button interrupts");
        return -1;
    }

	gpio_init_callback(&button_cb_data_0, button_0_pressed, BIT(button1.pin));
    gpio_add_callback(button1.port, &button_cb_data_0);

	gpio_init_callback(&button_cb_data_1, button_1_pressed, BIT(button2.pin));
    gpio_add_callback(button2.port, &button_cb_data_1);

	gpio_init_callback(&button_cb_data_2, button_2_pressed, BIT(button3.pin));
    gpio_add_callback(button3.port, &button_cb_data_2);

	gpio_init_callback(&button_cb_data_3, button_3_pressed, BIT(button4.pin));
    gpio_add_callback(button4.port, &button_cb_data_3);

	LOG_INF("Buttons initialized");

    const struct device *display_dev;

    display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        LOG_ERR("Display device not ready");
        return 0;
    }

    lv_init();

    /* Make background white */
    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

    lv_obj_t *count_label;
    count_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_bg_color(count_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(count_label, LV_OPA_TRANSP, LV_PART_MAIN);  // Transparent background
    lv_obj_set_width(count_label, 100);   // ensure label box doesn't resize
    lv_obj_set_height(count_label, 50);  // ensure label box doesn't resize
    lv_label_set_long_mode(count_label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(count_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(count_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(count_label, &lv_font_montserrat_48, 0);
    lv_label_set_text(count_label, "0");
    lv_obj_align(count_label, LV_ALIGN_CENTER, 0, 0);

    /* Turn on the display */
    display_blanking_off(display_dev);

    /* Draw UI once */
    lv_timer_handler();
    lv_refr_now(NULL);

    while (1) {

        // Check if any button is pressed
        if (button_0_pressed_flag || button_1_pressed_flag || button_2_pressed_flag || button_3_pressed_flag) {

            if (button_0_pressed_flag) count += INC_VAL_1;
            if (button_1_pressed_flag) count += INC_VAL_2;
            if (button_2_pressed_flag) count += INC_VAL_3;
            if (button_3_pressed_flag) count += INC_VAL_4;

            if (count > 9999) count = 0;

            button_0_pressed_flag = false;
            button_1_pressed_flag = false;
            button_2_pressed_flag = false;
            button_3_pressed_flag = false;

			LOG_INF("Button pressed, count: %u", count);

            static char buf[32];
            snprintf(buf, sizeof(buf), "%u", count);
            
            lv_label_set_text(count_label, buf);
            lv_obj_invalidate(count_label);

            lv_timer_handler();
		}
		
		k_sleep(K_MSEC(10));
    }
}
