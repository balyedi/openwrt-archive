/*
 *  Mikrotik RouterBOARD 1xx series support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  NAND initialization code was based on a driver for Linux 2.6.19+ which
 *  was derived from the driver for Linux 2.4.xx published by Mikrotik for
 *  their RouterBoard 1xx and 5xx series boards.
 *    Copyright (C) 2007 David Goodenough <david.goodenough@linkchoose.co.uk>
 *    Copyright (C) 2007 Florian Fainelli <florian@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "rb-1xx.h"

#define RB1XX_NAND_CHIP_DELAY	25

static struct adm5120_pci_irq rb1xx_pci_irqs[] __initdata = {
	PCIIRQ(1, 0, 1, ADM5120_IRQ_PCI0),
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI1),
	PCIIRQ(3, 0, 1, ADM5120_IRQ_PCI2)
};

#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition rb1xx_nor_parts[] = {
	{
		.name	= "booter",
		.offset	= 0,
		.size	= 64*1024,
		.mask_flags = MTD_WRITEABLE,
	} , {
		.name	= "firmware",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	}
};

static struct mtd_partition rb1xx_nand_parts[] = {
	{
		.name	= "kernel",
		.offset	= 0,
		.size	= 4 * 1024 * 1024,
	} , {
		.name	= "rootfs",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= MTDPART_SIZ_FULL
	}
};
#endif /* CONFIG_MTD_PARTITIONS */

/*
 * We need to use the OLD Yaffs-1 OOB layout, otherwise the RB bootloader
 * will not be able to find the kernel that we load.  So set the oobinfo
 * when creating the partitions
 */
static struct nand_ecclayout rb1xx_nand_ecclayout = {
	.eccbytes	= 6,
	.eccpos		= { 8, 9, 10, 13, 14, 15 },
	.oobavail	= 9,
	.oobfree	= { { 0, 4 }, { 6, 2 }, { 11, 2 }, { 4, 1 } }
};

/*--------------------------------------------------------------------------*/

static int rb1xx_nand_fixup(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

	if (mtd->writesize == 512)
		chip->ecc.layout = &rb1xx_nand_ecclayout;

	return 0;
}

struct platform_nand_data rb1xx_nand_data __initdata = {
	.chip = {
		.nr_chips	= 1,
#ifdef CONFIG_MTD_PARTITIONS
		.nr_partitions	= ARRAY_SIZE(rb1xx_nand_parts),
		.partitions	= rb1xx_nand_parts,
#endif /* CONFIG_MTD_PARTITIONS */
		.chip_delay	= RB1XX_NAND_CHIP_DELAY,
		.options	= NAND_NO_AUTOINCR,
		.chip_fixup	= rb1xx_nand_fixup,
	},
};

struct gpio_button rb1xx_gpio_buttons[] __initdata = {
	{
		.desc		= "reset_button",
		.type		= EV_KEY,
		.code		= BTN_0,
		.threshold	= 5,
		.gpio		= ADM5120_GPIO_PIN7,
	}
};

static void __init rb1xx_mac_setup(void)
{
	int i, j;

	if (!rb_hs.mac_base)
		return;

	for (i = 0; i < 6; i++) {
		for (j = 0; j < 5; j++)
			adm5120_eth_macs[i][j] = rb_hs.mac_base[j];
		adm5120_eth_macs[i][5] = rb_hs.mac_base[5]+i;
	}
}

void __init rb1xx_add_device_flash(void)
{
	/* setup data for flash0 device */
#ifdef CONFIG_MTD_PARTITIONS
	adm5120_flash0_data.nr_parts = ARRAY_SIZE(rb1xx_nor_parts);
	adm5120_flash0_data.parts = rb1xx_nor_parts;
#endif /* CONFIG_MTD_PARTITIONS */
	adm5120_flash0_data.window_size = 128*1024;

	adm5120_add_device_flash(0);
}

void __init rb1xx_add_device_nand(void)
{
	/* enable NAND flash interface */
	adm5120_nand_enable();

	/* initialize NAND chip */
	adm5120_nand_set_spn(1);
	adm5120_nand_set_wpn(0);

	adm5120_add_device_nand(&rb1xx_nand_data);
}

void __init rb1xx_generic_setup(void)
{
	if (adm5120_package_bga())
		adm5120_pci_set_irq_map(ARRAY_SIZE(rb1xx_pci_irqs),
					rb1xx_pci_irqs);

	adm5120_add_device_gpio_buttons(ARRAY_SIZE(rb1xx_gpio_buttons),
					rb1xx_gpio_buttons);

	rb1xx_add_device_flash();
	rb1xx_mac_setup();
}
