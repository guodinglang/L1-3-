#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Encoder_AB_Init(void);
int32_t Encoder_AB_GetCount(void);
void Encoder_AB_ResetCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __ENCODER_H__ */
