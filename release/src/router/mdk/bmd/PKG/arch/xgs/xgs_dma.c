/*
 * $Id: xgs_dma.c,v 1.7 Broadcom SDK $
 *
 * $Copyright: Copyright 2013 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 */

#include <bmd/bmd.h>

#ifdef CDK_CONFIG_ARCH_XGS_INSTALLED

#include <bmdi/arch/xgs_dma.h>

#include <cdk/cdk_error.h>
#include <cdk/arch/xgs_cmic.h>

/*
 * DMA_STAT: control bits
 *
 *  xxx_SET and xxx_CLR can be WRITTEN to CMIC_DMA_STAT
 *  xxx_TST can be masked against values read from CMIC_DMA_STAT.
 *  Argument required: 0 <= ch <= 3
 */

#define DS_DMA_ACTIVE(ch)               (0x00040000 << (ch))
#define DS_DMA_EN_SET(ch)               (0x80|(ch))
#define DS_DMA_EN_CLR(ch)               (0x00|(ch))
#define DS_DMA_EN_TST(ch)               (0x00000001 << (ch))

#define DS_CHAIN_DONE_SET(ch)           (0x80|(4+(ch)))
#define DS_CHAIN_DONE_CLR(ch)           (0x00|(4+(ch)))
#define DS_CHAIN_DONE_TST(ch)           (0x00000010 << (ch))

#define DS_DESC_DONE_SET(ch)            (0x80|(8+(ch)))
#define DS_DESC_DONE_CLR(ch)            (0x00|(8+(ch)))
#define DS_DESC_DONE_TST(ch)            (0x00000100 << (ch))

#define DC_ABORT_DMA(ch)                (0x04 << (8 * (ch)))


/*
 * Per-channel operations. 
 * These are the basis for the TX/RX functions
 */

static int
_clear_chan(int unit, int chan)
{
    CMIC_DMA_STATr_t dma_stat;

    CMIC_DMA_STATr_SET(dma_stat, DS_DMA_EN_CLR(chan));
    WRITE_CMIC_DMA_STATr(unit, dma_stat);

    CMIC_DMA_STATr_SET(dma_stat, DS_CHAIN_DONE_CLR(chan));
    WRITE_CMIC_DMA_STATr(unit, dma_stat);

    CMIC_DMA_STATr_SET(dma_stat, DS_DESC_DONE_CLR(chan));
    WRITE_CMIC_DMA_STATr(unit, dma_stat);

    CDK_CONFIG_MEMORY_BARRIER; 

    return CDK_E_NONE; 
}

int 
bmd_xgs_dma_chan_init(int unit, int chan, int dir)
{
    CMIC_DMA_CTRLr_t cdc; 

    READ_CMIC_DMA_CTRLr(unit, &cdc); 

    switch (chan) {
    case 0: 
        CMIC_DMA_CTRLr_CH0_DIRECTIONf_SET(cdc, dir);
        break; 
    case 1: 
        CMIC_DMA_CTRLr_CH1_DIRECTIONf_SET(cdc, dir);
        break; 
    case 2: 
        CMIC_DMA_CTRLr_CH2_DIRECTIONf_SET(cdc, dir);
        break; 
    case 3: 
        CMIC_DMA_CTRLr_CH3_DIRECTIONf_SET(cdc, dir);
        break; 
    default: 
        return CDK_E_UNAVAIL; 
    }

    WRITE_CMIC_DMA_CTRLr(unit, cdc); 

    return CDK_E_NONE; 
}

int 
bmd_xgs_dma_chan_start(int unit, int chan, dma_addr_t dcb)
{
    /* Write the DCB address to the DESC address for this channel */
    CDK_DEV_WRITE32(unit, CMIC_DMA_DESC0r + 4*chan, dcb);
    
    CDK_CONFIG_MEMORY_BARRIER; 

    /* Kick it off */
    CDK_DEV_WRITE32(unit, CMIC_DMA_STATr, DS_DMA_EN_SET(chan)); 

    CDK_CONFIG_MEMORY_BARRIER; 

    return CDK_E_NONE; 
}

int
bmd_xgs_dma_chan_poll(int unit, int chan, int polls)
{
    int p; 
    uint32_t dma_stat;
    
    for (p = 0; p < polls; p++) {
        CDK_DEV_READ32(unit, CMIC_DMA_STATr, &dma_stat);
        if (dma_stat & DS_DESC_DONE_TST(chan)) {
            /* DMA complete. Clear the channel */
            _clear_chan(unit, chan); 
            return p; 
        }
    }
    
    return CDK_E_TIMEOUT; 
}

int
bmd_xgs_dma_chan_abort(int unit, int chan, int polls)
{
    uint32_t ctrl, dma_stat; 
    int p; 
    
    /* Clear enable */
    CDK_DEV_WRITE32(unit, CMIC_DMA_STATr, DS_DMA_EN_CLR(chan)); 

    CDK_CONFIG_MEMORY_BARRIER; 
    
    /* Abort the channel */
    CDK_DEV_READ32(unit, CMIC_DMA_CTRLr, &ctrl); 
    CDK_DEV_WRITE32(unit, CMIC_DMA_CTRLr, ctrl | DC_ABORT_DMA(chan)); 

    CDK_CONFIG_MEMORY_BARRIER; 
    
    /* Poll for abort completion */
    for (p = 0; p < polls; p++) {
        CDK_DEV_READ32(unit, CMIC_DMA_STATr, &dma_stat);
        if (!(dma_stat & DS_DMA_ACTIVE(chan))) {
            /* Restore previous control value */            
            CDK_DEV_WRITE32(unit, CMIC_DMA_CTRLr, ctrl); 

            CDK_CONFIG_MEMORY_BARRIER; 
            
            /* Clear up channel */
            _clear_chan(unit, chan); 
            
            return polls; 
        }
    }

    return CDK_E_TIMEOUT; 
}

int 
bmd_xgs_dma_init(int unit)
{
    CMIC_CONFIGr_t cc; 
    int ioerr = 0;
    
    /* Read the current CMIC_CONFIG register */
    ioerr += READ_CMIC_CONFIGr(unit, &cc); 

    /* Enable scatter/gather and reload */
    CMIC_CONFIGr_SG_ENABLEf_SET(cc, 1);
    CMIC_CONFIGr_SG_RELOAD_ENABLEf_SET(cc, 1);

    /* Allow unaligned Tx buffers */
    CMIC_CONFIGr_IGNORE_ADR_ALIGN_ENf_SET(cc, 1);

    /* Write the config */
    ioerr += WRITE_CMIC_CONFIGr(unit, cc); 
    
    /* Initialize TX and RX channels */
    bmd_xgs_dma_chan_init(unit, XGS_DMA_TX_CHAN, XGS_DMA_CHAN_DIR_TX); 
    bmd_xgs_dma_chan_init(unit, XGS_DMA_RX_CHAN, XGS_DMA_CHAN_DIR_RX); 

    return ioerr ? CDK_E_IO : CDK_E_NONE; 
}


int 
bmd_xgs_dma_tx_start(int unit, dma_addr_t dcb)
{       
    return bmd_xgs_dma_chan_start(unit, XGS_DMA_TX_CHAN, dcb); 
}

int
bmd_xgs_dma_tx_poll(int unit, int num_polls)
{
    return bmd_xgs_dma_chan_poll(unit, XGS_DMA_TX_CHAN, num_polls); 
}

int
bmd_xgs_dma_tx_abort(int unit, int num_polls)
{
    return bmd_xgs_dma_chan_abort(unit, XGS_DMA_TX_CHAN, num_polls); 
}

int 
bmd_xgs_dma_rx_start(int unit, dma_addr_t dcb)
{       
    return bmd_xgs_dma_chan_start(unit, XGS_DMA_RX_CHAN, dcb); 
}

int
bmd_xgs_dma_rx_poll(int unit, int num_polls)
{
    return bmd_xgs_dma_chan_poll(unit, XGS_DMA_RX_CHAN, num_polls); 
}

int
bmd_xgs_dma_rx_abort(int unit, int num_polls)
{
    return bmd_xgs_dma_chan_abort(unit, XGS_DMA_RX_CHAN, num_polls); 
}

#endif /* CDK_CONFIG_ARCH_XGS_INSTALLED */
