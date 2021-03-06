/*
 * Copyright (C) 2012 by Elecard-STB.
 * Written by Andrey Kuleshov <Andrey.Kuleshov@elecard.ru>
 *            Anton Sergeev <Anton.Sergeev@elecard.ru>
 *
 * MxL9201S01 dvb-t2/dvb-c nim based on Sony MxL201RF and CX2820R.
 * Also supports connective CX2820R with TDA665x.
 */

/******************************************************************
* INCLUDE FILES                                                   *
*******************************************************************/
#ifdef USE_LINUXTV
#include <dvb/frontend.h>
#endif
#include <dvb_frontend.h>

#include <dvb-pll.h>

#include "sonydvbt2.h"
#include "st_dvb.h"
#include "cxd2820r.h"
#include "mxl201rf_tuner.h"
//#include "MxL201RF_Common.h"

/******************************************************************
* LOCAL MACROS                                                    *
*******************************************************************/
#define  CXD2820R_I2C_U1 0x6c
#define  CXD2820R_I2C_U2 0x6d

#define dprintk(format, args...) if (st_dvb_debug) { printk("%s[%d]: " format, __FILE__, __LINE__, ##args); }

/******************************************************************
* LOCAL TYPEDEFS                                                  *
*******************************************************************/

typedef enum {
	ePllType_mxl201,
	ePllType_tda6651,

	ePllType_count,
} ePllType;

typedef struct {
	char		*name;
	ePllType	type;
	uint8_t		i2c_addr;
} pllDescription_t;

/******************************************************************
* STATIC DATA                                                     *
*******************************************************************/
static struct cxd2820r_config	cxd2820r_cfg = {
	.i2c_address = CXD2820R_I2C_U1,
#ifdef USE_LINUXTV
	.ts_mode     = CXD2820R_TS_PARALLEL_MSB,
#else
	.ts_mode     = CXD2820R_TS_PARALLEL_MSB | CXD2820R_TS_CLK_ACTIVE,
	.if_agc_polarity = 0,
	.if_dvbt_6  = 5000,
	.if_dvbt_7  = 5000,
	.if_dvbt_8  = 5000,
	.if_dvbt2_5 = 5000,
	.if_dvbt2_6 = 5000,
	.if_dvbt2_7 = 5000,
	.if_dvbt2_8 = 5000,
	.if_dvbc    = 5000,
	.gpio_dvbt   = { CXD2820R_GPIO_D, CXD2820R_GPIO_E | CXD2820R_GPIO_O | CXD2820R_GPIO_L, CXD2820R_GPIO_D },
	.gpio_dvbc   = { CXD2820R_GPIO_D, CXD2820R_GPIO_E | CXD2820R_GPIO_O | CXD2820R_GPIO_L, CXD2820R_GPIO_D },
	.gpio_dvbt2  = { CXD2820R_GPIO_D, CXD2820R_GPIO_E | CXD2820R_GPIO_O | CXD2820R_GPIO_L, CXD2820R_GPIO_D },
#endif
};

pllDescription_t pllDescripton[] = {
	{"Maxlinear MXL201 pll on path1",	ePllType_mxl201,	MxL210RF_I2C_ADDRESS_PATH1},
	{"Maxlinear MXL201 pll on path2",	ePllType_mxl201,	MxL210RF_I2C_ADDRESS_PATH2},
	{"Philips TDA6650/TDA6651 pll",		ePllType_tda6651,	0x61},
};

/******************************************************************
* FUNCTION IMPLEMENTATION                     <Module>_<Word>+    *
*******************************************************************/
int sonydvbt2_check_pll(struct dvb_frontend *fe, struct i2c_adapter *adapter, unsigned char tuner_addr)
{
	int ret;
	u8 buf[1];
	struct i2c_msg msg[1] = {
		{
			.addr = tuner_addr,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = buf,
		}
	};
	if(fe->ops.i2c_gate_ctrl) {
		fe->ops.i2c_gate_ctrl(fe, 1);
	}
	ret = i2c_transfer(adapter, msg, 1);
	if(ret != 1) {
		dprintk("Read from tuner address %02x: error %d\n", tuner_addr, ret);
	}
	if(fe->ops.i2c_gate_ctrl) {
		fe->ops.i2c_gate_ctrl(fe, 0);
	}
	return ret;
}

struct dvb_frontend* sonydvbt2_init_frontend(struct i2c_adapter *adapter)
{
	uint32_t	pllId;
	struct dvb_frontend *fe = NULL;
	struct dvb_frontend *pll = NULL;

#ifdef USE_LINUXTV
	fe = dvb_attach(cxd2820r_attach, &cxd2820r_cfg, adapter, NULL);
#else
	fe = dvb_attach(cxd2820r_attach, &cxd2820r_cfg, adapter);
#endif
	if(!fe) {
		dprintk("cant attach cxd2820r\n");
		return NULL;
	}
	fe->dtv_property_cache.delivery_system = SYS_DVBT;
	fe->ops.info.type = FE_OFDM;
	dprintk("cxd2820r (DVB-T) attached\n");
// 	fe->dtv_property_cache.delivery_system = SYS_DVBC_ANNEX_A;
// 	fe->ops.info.type = FE_QAM;
// 	dprintk("cxd2820r (DVB-C) attached\n");

	for(pllId = 0; pllId < ARRAY_SIZE(pllDescripton); pllId++) {
		pllDescription_t *pllDescr = pllDescripton + pllId;
		if(sonydvbt2_check_pll(fe, adapter, pllDescr->i2c_addr) == 1)
			break;
	}
	if(pllId >= ARRAY_SIZE(pllDescripton)) {
		dprintk("failed to detect pll\n");
		goto tuner_error;
	}

	if(pllDescripton[pllId].type == ePllType_mxl201) {
		pll = dvb_attach(mxl201rf_attach, fe,
						adapter, pllDescripton[pllId].i2c_addr, MxL_IF_5_MHZ);
	} else if(pllDescripton[pllId].type == ePllType_tda6651) {
		pll = dvb_attach(dvb_pll_attach, fe,
						pllDescripton[pllId].i2c_addr, adapter, DVB_PLL_TDA665X);
	}
	if(!pll) {
		dprintk("can't attach %s\n", pllDescripton[pllId].name);
		goto tuner_error;
	}
	dprintk("%s attached\n", pllDescripton[pllId].name);
	return fe;

tuner_error:
	dvb_frontend_detach(fe);
	return NULL;
}
