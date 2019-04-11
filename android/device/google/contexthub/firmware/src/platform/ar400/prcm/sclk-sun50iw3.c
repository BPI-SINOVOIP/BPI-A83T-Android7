/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                	  	  clock control unit module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : sclk.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-7
* Descript: system clock management.
* Update  : date                auther      ver     notes
*           2012-5-7 8:43:10	Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "ccu_i-sun50iw3.h"

#if (defined CONFIG_ARCH_SUN50IW3P1)

/*
*********************************************************************************************************
*                                      SET SOURCE FREQUENCY
*
* Description: 	set the frequency of a specific source clock.
*
* Arguments  : 	sclk : the source clock ID which we want to set frequency.
*				freq : the frequency which we want to set.
*
* Returns    : 	OK if set source frequency succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_set_sclk_freq(u32 sclk, u32 freq)
{
	switch	(sclk)
	{
		case CCU_SYS_CLK_PLL1:
		{
			ccu_pll1_factor_t       factor;
			ccu_pll_c0_cpux_reg0000_t  pll1;

			//calc pll1 factors by freq.
			ccu_calc_pll1_factor(&factor, freq);

			//set factor
			pll1 = *(ccu_pll_c0_cpux_reg_addr);

			//try to increase factor p first
			if (pll1.factor_p < factor.factor_p)
			{
				pll1.factor_p = factor.factor_p;
				*(ccu_pll_c0_cpux_reg_addr) = pll1;
				time_cdelay(2000);					//delay 10us, cpu clock 200m
			}

			//try to increase factor m first
			if (pll1.factor_m < factor.factor_m)
			{
				pll1.factor_m = factor.factor_m;
				*(ccu_pll_c0_cpux_reg_addr) = pll1;
				time_cdelay(2000);					//delay 10us, cpu clock 200m
			}

			//write factor n * k
			pll1.factor_n = factor.factor_n;
			*(ccu_pll_c0_cpux_reg_addr) = pll1;

			//wait for lock change first
			time_cdelay(20);

			//wait for PLL1 stable
			//maybe the fpga have not to wait PLLx stable
			//by superm at 2013-5-15 13:07:21
			time_mdelay(1);

			//decease factor m
			if (pll1.factor_m > factor.factor_m)
			{
				pll1.factor_m = factor.factor_m;
				*(ccu_pll_c0_cpux_reg_addr) = pll1;
				time_cdelay(2000);					//delay 10us, cpu clock 200m
			}

	    		//decease factor p
	    		if (pll1.factor_p > factor.factor_p)
	    		{
	    			pll1.factor_p = factor.factor_p;
	    			*(ccu_pll_c0_cpux_reg_addr) = pll1;
	    			time_cdelay(2000);					//delay 10us, cpu clock 200m
	    		}
	            INF("PLL1 Freq %d N %d M %d P %d\n", freq, factor.factor_n, factor.factor_m, factor.factor_p);
	    		return OK;
	    	}
	    	default:
	    	{
	    		WRN("invaid clock id (%d) when set freq\n", sclk);
	    		return -EINVAL;
	    	}
	}
	//un-reached
}

/*
*********************************************************************************************************
*                                     GET SOURCE FREQUENCY
*
* Description: 	get the frequency of a specific source clock.
*
* Arguments  : 	sclk : the source clock ID which we want to get frequency.
*
* Returns    : 	frequency of the specific source clock.
*********************************************************************************************************
*/
s32 ccu_get_sclk_freq(u32 sclk)
{
	switch (sclk) {
	case CCU_SYS_CLK_LOSC:
	{
		return losc_freq;
	}
    	case CCU_SYS_CLK_HOSC:
    	{
    		return CCU_HOSC_FREQ;
    	}
    	case CCU_SYS_CLK_PLL1:
    	{
		//maybe should delete
    		ccu_pll_c0_cpux_reg0000_t pll_c0 = *(ccu_pll_c0_cpux_reg_addr);
		return (CCU_HOSC_FREQ * (pll_c0.factor_n + 1)) / ((pll_c0.factor_m + 1) * (1 << pll_c0.factor_p));
    	}
    	case CCU_SYS_CLK_CPUS:
    	{
    		switch (ccu_reg_addr->cpus_clk_cfg.src_sel)
    		{
    			case 0:
    			{
    				//cpus clock source is losc
    				return CCU_HOSC_FREQ;
    			}
    			case 1:
    			{
    				//cpus clock source is hosc
    				return losc_freq;
    			}
    			case 2:
    			{
				//cpus clock source is internal-osc
    				return iosc_freq;
    			}
    			case 3:
    			{
    				//cpus clock source is pll6
    				return ccu_get_sclk_freq(CCU_SYS_CLK_PLL5) / (ccu_reg_addr->cpus_clk_cfg.factor_m + 1);
    			}
    			default :
    			{
    				return 0;
    			}
    		}
    	}
    	case CCU_SYS_CLK_AHBS:
    	{
    		return ccu_get_sclk_freq(CCU_SYS_CLK_CPUS);
    	}
    	case CCU_SYS_CLK_APBS1:
	{
		return ccu_get_sclk_freq(CCU_SYS_CLK_AHBS) / ccu_get_mclk_div(CCU_MOD_CLK_APBS1);
	}
	case CCU_SYS_CLK_APBS2:
	{
		switch (ccu_reg_addr->apbs2_cfg.src_sel)
    		{
    			case 0:
    			{
    				//cpus clock source is losc
    				return CCU_HOSC_FREQ;
    			}
    			case 1:
    			{
    				//cpus clock source is hosc
    				return losc_freq;
    			}
    			case 2:
    			{
				//cpus clock source is internal-osc
    				return iosc_freq;
    			}
    			case 3:
    			{
    				//cpus clock source is pll6
    				return ccu_get_sclk_freq(CCU_SYS_CLK_PLL5) / (ccu_reg_addr->apbs2_cfg.factor_m + 1) / (1 << ccu_reg_addr->apbs2_cfg.factor_n);
    			}
    			default :
    			{
    				return 0;
    			}
    		}
	}
	case CCU_SYS_CLK_PLL5:
	{
		/* output=24M*N*K/2 */
    		ccu_pll_periph0_reg0020_t pll_periph0 = *(ccu_pll_periph0_reg_addr);
		return (CCU_HOSC_FREQ * (pll_periph0.factor_n + 1) / (pll_periph0.factor_m0 + 1) / (pll_periph0.factor_m1 + 1)) / 4;
	}
	case CCU_SYS_CLK_SPI:
	{
		if (ccu_reg_addr->r_spi_clk.sclk_gate == 1) {
			switch (ccu_reg_addr->r_spi_clk.src_sel)
			{
				case 0:
				{
					return CCU_HOSC_FREQ;
				}
				case 1:
				{
					return ccu_get_sclk_freq(CCU_SYS_CLK_PLL_PERI0_1X);
				}
				case 2:
				{
					return ccu_get_sclk_freq(CCU_SYS_CLK_PLL_PERI1_1X);
				}
				case 3:
				{
					return ccu_get_sclk_freq(CCU_SYS_CLK_PLL_PERI0_2X);
				}
				case 4:
				{
					return ccu_get_sclk_freq(CCU_SYS_CLK_PLL_PERI1_2X);
				}
				case 5:
				{
					return iosc_freq;
				}
				case 6:
				{
					return losc_freq;
				}
				default:
				{
					//invalid source id for sys clock
					return -EINVAL;
				}
			}
		} else {
			return ccu_get_sclk_freq(CCU_SYS_CLK_APBS1);
		}

	}
	}
	WRN("invalid clock id for get source freq\n");
	return 0;
}

#endif
