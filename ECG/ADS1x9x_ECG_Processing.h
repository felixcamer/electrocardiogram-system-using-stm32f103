

#ifndef ADS1x9x_ECG_PROCESSING_H_
#define ADS1x9x_ECG_PROCESSING_H_
#include "sys.h"  
///extern	volatile unsigned short QRS_Heart_Rate1;

//#define TRUE	1
//#define FALSE	0
//! \brief MAX_PEAK_TO_SEARCH
//!
#define MAX_PEAK_TO_SEARCH 				8

//! \brief MAXIMA_SEARCH_WINDOW
//!
#define MAXIMA_SEARCH_WINDOW			30 //40

//! \brief MINIMUM_SKIP_WINDOW
//!
#define MINIMUM_SKIP_WINDOW				40  //50

//! \brief SAMPLING_RATE
//!
#define SAMPLING_RATE					125

//! \brief TWO_SEC_SAMPLES
//!
#define TWO_SEC_SAMPLES  				2 * SAMPLING_RATE

/*threshold = 0.7 * maxima*/
#define QRS_THRESHOLD_FRACTION	0.7					

//! \brief MAXCHAN
//!




float ecg_filters(float sample,float *w0,float *w1,float *w2);

void IIR_Reset(float *w0,float *w1,float *w2);

//static void QRS_check_sample_crossing_threshold( unsigned short scaled_result );

void QRS_Algorithm_Interface(short CurrSample);
#endif /*ADS1x9x_ECG_PROCESSING_H_*/
