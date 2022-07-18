/*
 * sensor_config.h
 *
 *  Created on: 15.07.2022
 *      Author: Marks_Markus
 */

#ifndef SENSOR_CONFIG_H_
#define SENSOR_CONFIG_H_

/*
 * data rate for normal and fast mode
 */
typedef enum
{
    N_DR_20_SPS = 0,
    N_DR_45_SPS,
    N_DR_90_SPS,
    N_DR_175_SPS,
    N_DR_330_SPS,
    N_DR_600_SPS,
    N_DR_1000_SPS,
    N_DR_NA,
    F_DR_40_SPS,
    F_DR_90_SPS,
    F_DR_180_SPS,
    F_DR_350_SPS,
    F_DR_660_SPS,
    F_DR_1200_SPS,
    F_DR_2000_SPS,
    F_DR_NA
} RSC_DATA_RATE;

/*
 * modi for rsc sensor
 */
typedef enum
{
    NORMAL_MODE = 0,
    NA_MODE,
    FAST_MODE
} RSC_MODE;


/*
 * Parameter for the rsc sensors
 */
struct sensor_config
{
	RSC_DATA_RATE datarate;
	RSC_MODE mode;
	uint8_t delay;
};



#endif /* SENSOR_CONFIG_H_ */
