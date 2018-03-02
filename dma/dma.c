#include "dma.h"
#include "mutex/mutex.h"

#define DMA_CHANNELS_COUNT 12

static mutex_t dma_mutexes[DMA_CHANNELS_COUNT] = {MUTEX_UNLOCKED};

uint32_t dma_channel_number(DMA_Channel_TypeDef* dma_channel)
{
    if((uint32_t)dma_channel >= DMA1_Channel1_BASE &&
       (uint32_t)dma_channel <= DMA1_Channel7_BASE){
        
        return ((uint32_t)dma_channel - DMA1_Channel1_BASE) / 0x14 + 1;
        
    }else if((uint32_t)dma_channel >= DMA2_Channel1_BASE &&
             (uint32_t)dma_channel <= DMA2_Channel5_BASE){
        
        return ((uint32_t)dma_channel - DMA2_Channel1_BASE) / 0x14 + 8;
    }
    return 0;
}

DMA_TypeDef* dma_channel_dma_periph(DMA_Channel_TypeDef* dma_channel)
{
    if((uint32_t)dma_channel >= DMA1_Channel1_BASE &&
       (uint32_t)dma_channel <= DMA1_Channel7_BASE){
        
        return DMA1;
        
    }else if((uint32_t)dma_channel >= DMA2_Channel1_BASE &&
             (uint32_t)dma_channel <= DMA2_Channel5_BASE){
        
        return DMA2;
    }
    return 0;
}

uint32_t dma_channel_it_flag(DMA_Channel_TypeDef* dma_channel, uint32_t dma_it_flag)
{
    uint32_t channel_number = dma_channel_number(dma_channel);
    
    // Error.
    if(channel_number == 0) return 0;
    // DMA1.
    if(channel_number <= 7) return dma_it_flag << ((channel_number - 1) * 4);
    // DMA2.
    return (dma_it_flag << ((channel_number - 8) * 4)) | DMA_PERIPH_SEL;
}

bool dma_channel_it_flag_status(uint32_t dma_ch_it_flag)
{
    // DMA1
    if(!(dma_ch_it_flag & DMA_PERIPH_SEL)){
        return (DMA1->ISR & dma_ch_it_flag) != 0;
    }
    // DMA2.
    else{
        return (DMA2->ISR & (dma_ch_it_flag & ~DMA_PERIPH_SEL)) != 0;
    }
}

void dma_channel_it_flag_clear(uint32_t dma_ch_it_flag)
{
    // DMA1
    if(!(dma_ch_it_flag & DMA_PERIPH_SEL)){
        DMA1->IFCR = dma_ch_it_flag;
    }
    // DMA2.
    else{
        DMA2->IFCR = (dma_ch_it_flag & ~DMA_PERIPH_SEL);
    }
}

err_t dma_channel_lock(DMA_Channel_TypeDef* dma_channel)
{
    uint32_t channel_number = dma_channel_number(dma_channel);
    
    if(channel_number == 0) return E_INVALID_VALUE;
    
    mutex_lock(&dma_mutexes[channel_number - 1]);
    
    return E_NO_ERROR;
}

bool dma_channel_trylock(DMA_Channel_TypeDef* dma_channel)
{
    uint32_t channel_number = dma_channel_number(dma_channel);
    
    if(channel_number == 0) return false;
    
    return mutex_trylock(&dma_mutexes[channel_number - 1]);
}

err_t dma_channel_unlock(DMA_Channel_TypeDef* dma_channel)
{
    uint32_t channel_number = dma_channel_number(dma_channel);
    
    if(channel_number == 0) return E_INVALID_VALUE;
    
    mutex_unlock(&dma_mutexes[channel_number - 1]);
    
    return E_NO_ERROR;
}
