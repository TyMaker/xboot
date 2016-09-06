/*
 * driver/clk/clk-gate.c
 *
 * Copyright(c) 2007-2016 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <clk/clk.h>

struct clk_gate_pdata_t {
	virtual_addr_t virt;
	char * parent;
	int shift;
	int invert;
};

static void clk_gate_set_parent(struct clk_t * clk, const char * pname)
{
}

static const char * clk_gate_get_parent(struct clk_t * clk)
{
	struct clk_gate_pdata_t * pdat = (struct clk_gate_pdata_t *)clk->priv;
	return pdat->parent;
}

static void clk_gate_set_enable(struct clk_t * clk, bool_t enable)
{
	struct clk_gate_pdata_t * pdat = (struct clk_gate_pdata_t *)clk->priv;

	if(enable)
		write32(pdat->virt, (read32(pdat->virt) & ~(0x1 << pdat->shift)) | ((pdat->invert ? 0x0 : 0x1) << pdat->shift));
	else
		write32(pdat->virt, (read32(pdat->virt) & ~(0x1 << pdat->shift)) | ((pdat->invert ? 0x1 : 0x0) << pdat->shift));
}

static bool_t clk_gate_get_enable(struct clk_t * clk)
{
	struct clk_gate_pdata_t * pdat = (struct clk_gate_pdata_t *)clk->priv;

	if(read32(pdat->virt) & (0x1 << pdat->shift))
		return pdat->invert ? FALSE : TRUE;
	return pdat->invert ? TRUE : FALSE;
}

static void clk_gate_set_rate(struct clk_t * clk, u64_t prate, u64_t rate)
{
}

static u64_t clk_gate_get_rate(struct clk_t * clk, u64_t prate)
{
	return prate;
}

static struct device_t * clk_gate_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct clk_gate_pdata_t * pdat;
	struct clk_t * clk;
	struct device_t * dev;
	virtual_addr_t virt = phys_to_virt(dt_read_address(n));
	char * parent = dt_read_string(n, "parent", NULL);
	char * name = dt_read_string(n, "name", NULL);
	int shift = dt_read_int(n, "shift", -1);

	if(!parent || !name || (shift < 0))
		return NULL;

	if(!search_clk(parent) || search_clk(name))
		return NULL;

	pdat = malloc(sizeof(struct clk_gate_pdata_t));
	if(!pdat)
		return NULL;

	clk = malloc(sizeof(struct clk_t));
	if(!clk)
	{
		free(pdat);
		return NULL;
	}

	pdat->virt = virt;
	pdat->parent = strdup(parent);
	pdat->shift = shift;
	pdat->invert = dt_read_bool(n, "invert", 0);

	clk->name = strdup(name);
	clk->count = 0;
	clk->set_parent = clk_gate_set_parent;
	clk->get_parent = clk_gate_get_parent;
	clk->set_enable = clk_gate_set_enable;
	clk->get_enable = clk_gate_get_enable;
	clk->set_rate = clk_gate_set_rate;
	clk->get_rate = clk_gate_get_rate;
	clk->priv = pdat;

	if(!register_clk(&dev, clk))
	{
		free(pdat->parent);

		free(clk->name);
		free(clk->priv);
		free(clk);
		return NULL;
	}
	dev->driver = drv;

	return dev;
}

static void clk_gate_remove(struct device_t * dev)
{
	struct clk_t * clk = (struct clk_t *)dev->priv;
	struct clk_gate_pdata_t * pdat = (struct clk_gate_pdata_t *)clk->priv;

	if(clk && unregister_clk(clk))
	{
		free(pdat->parent);

		free(clk->name);
		free(clk->priv);
		free(clk);
	}
}

static void clk_gate_suspend(struct device_t * dev)
{
}

static void clk_gate_resume(struct device_t * dev)
{
}

static struct driver_t clk_gate = {
	.name		= "clk-gate",
	.probe		= clk_gate_probe,
	.remove		= clk_gate_remove,
	.suspend	= clk_gate_suspend,
	.resume		= clk_gate_resume,
};

static __init void clk_gate_driver_init(void)
{
	register_driver(&clk_gate);
}

static __exit void clk_gate_driver_exit(void)
{
	unregister_driver(&clk_gate);
}

driver_initcall(clk_gate_driver_init);
driver_exitcall(clk_gate_driver_exit);
