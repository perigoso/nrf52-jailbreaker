/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#include "util/data.h"
#include "util/types.h"

#include "tusb.h"

/* Device Descriptors */
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    /* Use Interface Association Descriptor (IAD) for CDC */
    /* As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1) */
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCAFE,
    .idProduct          = 0x0420,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x00,

    .bNumConfigurations = 0x01
};

/* Invoked when received GET DEVICE DESCRIPTOR */
/* Application return pointer to descriptor */
u8 const * tud_descriptor_device_cb(void)
{
	return (u8 const *)&desc_device;
};


/* Configuration Descriptor */
enum
{
  ITF_NUM_CDC = 0,
  ITF_NUM_CDC_DATA,
  ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

#define EPNUM_CDC_NOTIF   0x81
#define EPNUM_CDC_DATA    0x02

u8 const desc_configuration[] =
{
  /* Config number, interface count, string index, total length, attribute, power in mA */
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

  /* CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size. */
  TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 0, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_DATA, 0x80 | EPNUM_CDC_DATA, CFG_TUD_CDC_EP_BUFSIZE),
};

/* Invoked when received GET CONFIGURATION DESCRIPTOR */
/* Application return pointer to descriptor */
/* Descriptor contents must exist long enough for transfer to complete */
u8 const * tud_descriptor_configuration_cb(u8 index)
{
  (void) index; /* for multiple configurations */

  return desc_configuration;
}

/* String Descriptors */

/* array of pointer to string descriptors */
char const *string_desc_arr[] = {
	(const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
	"github/perigoso", // 1: Manufacturer
	PROJECT_NAME, // 2: Product
};

static u16 _desc_str[32];

/* Invoked when received GET STRING DESCRIPTOR request */
/* Application return pointer to descriptor, whose contents must exist long enough for transfer to complete */
u16 const *tud_descriptor_string_cb(u8 index, u16 langid)
{
	(void) langid;

	u8 chr_count;

	if (index == 0) {
		memcpy(&_desc_str[1], string_desc_arr[0], 2);
		chr_count = 1;
	} else {
		/* Convert ASCII string into UTF-16 */

		if (index > 2)
			return NULL;

		const char *str = string_desc_arr[index];

		/* Cap at max char */
		chr_count = strlen(str);
		if (chr_count > 31)
			chr_count = 31;

		for (u8 i = 0; i < chr_count; i++) {
			_desc_str[1 + i] = str[i];
		}
	}

	/* first byte is length (including header), second byte is string type */
	_desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

	return _desc_str;
}
