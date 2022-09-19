
#include "ADS1x9x_ECG_Processing.h"
#include "sys.h"  
#include <lcd.h>



#define L 100
#define N 11
#define M 5
#define TRUE	1
#define FALSE	0


unsigned int sample_index[MAX_PEAK_TO_SEARCH+2] ;
int index_=0;  //3000 ecg data
volatile float ECG_DATA[1000]={0};
volatile u8 ecg_data_flag=0;
/*  Pointer which points to the index in B4 buffer where the processed data */
/*  has to be filled */
static unsigned short QRS_B4_Buffer_ptr = 0 ;

/* Variable which will hold the calculated heart rate */
unsigned short QRS_Heart_Rate = 0 ;
unsigned char HR_flag;

/* 	Variable which holds the threshold value to calculate the maxima */
short QRS_Threshold_Old = 0;
short QRS_Threshold_New = 0;


/* Variables to hold the sample data for calculating the 1st and 2nd */
/* differentiation                                                   */
int QRS_Second_Prev_Sample = 0 ;
int QRS_Prev_Sample = 0 ;
int QRS_Current_Sample = 0 ;
int QRS_Next_Sample = 0 ;
int QRS_Second_Next_Sample = 0 ;

/* Flag which identifies the duration for which sample count has to be incremented */
unsigned char Start_Sample_Count_Flag = 0;
unsigned char first_peak_detect = FALSE ;
unsigned int sample_count = 0 ;
volatile unsigned short QRS_Heart_Rate1;



  u16 i=0,j,count=0;
  u8  stop_count=0;

  float x0=0,x1=0;
  float y0=0,y1=0,y2=0;
  float buf_N[N]={0};                   //SG平滑滤波数组
  float IIR_Filter_[100];           //缓存数组，储存滤波之后的时域信号 float类型
  
  
  double w1[3] ={0};
  double w2[3] ={0};
  double w0[3] ={0};


	void IIR_Reset(float *w0,float *w1,float *w2)
{
   for(i=0;i<3;i--)
 	 {
     w0[i]=0;
		 w1[i]=0;
		 w2[i]=0;
   }
	  y0=0;
	  y1=0;
    y2=0;
    	 
}



/******************************50hz_IIR_Fnotch_Filter**************************/
 const float IIR_50Notch_B[3] = {
   0.815292716,    1.319171309,    0.815292716
};

 const float IIR_50Notch_A[3] = {
	  1,    1.319171309,   0.6305854321
};



/*****************************0.5Hz_IIR__High_Filter****************************/
const float Gain=1;
const float IIR_High_B[3]={
        1,             -2,              1 
};

const float IIR_High_A[3]={
   1,   -1.974557996,   0.9751817584 //1,   -1.974557996,   0.9751817584
};


/*****************************33Hz_IIR__Low_Filter 3order****************************/
 const float IIR_LOW_B[3]={1,2,1};
 const float IIR_LOW_A[3]={  1,  -0.4188560843,   0.3554467559    };//
 
 

float ecg_filters(float samples,float *w0,float *w1,float *w2)
{
	
	
 float y=0;
//		  w0[0]=IIR_50Notch_A[0]*samples-IIR_50Notch_A[1]*w0[1]-IIR_50Notch_A[2]*w0[2];
//			y0=IIR_50Notch_B[0]*w0[0]+IIR_50Notch_B[1]*w0[1]+IIR_50Notch_B[2]*w0[2];
//			 
//			w1[0]=IIR_High_A[0]*y0-IIR_High_A[1]*w1[1]-IIR_High_A[2]*w1[2];
//			y1=Gain*(IIR_High_B[0]*w1[0]+IIR_High_B[1]*w1[1]+IIR_High_B[2]*w1[2]);
//		 
//		  w2[0]=IIR_LOW_A[0]*y1-IIR_LOW_A[1]*w2[1]-IIR_LOW_A[2]*w2[2];
//			y2=Gain*(IIR_LOW_B[0]*w2[0]+IIR_LOW_B[1]*w2[1]+IIR_LOW_B[2]*w2[2]);
//		
	
			w1[0]=IIR_High_A[0]*samples-IIR_High_A[1]*w1[1]-IIR_High_A[2]*w1[2];
			y1=Gain*(IIR_High_B[0]*w1[0]+IIR_High_B[1]*w1[1]+IIR_High_B[2]*w1[2]);
		 
		  w2[0]=IIR_LOW_A[0]*y1-IIR_LOW_A[1]*w2[1]-IIR_LOW_A[2]*w2[2];
			y2=Gain*(IIR_LOW_B[0]*w2[0]+IIR_LOW_B[1]*w2[1]+IIR_LOW_B[2]*w2[2]);
		
	
		 if(count>500){
		//IIR_Filter_Data[i]=y1;         //缓存数组储存滤波之后的信?		   //printf("\r\n%.5f",y2);
			 y=y2; 
			 
		 if(index_<1000)
			 {
				 
			 ECG_DATA[index_]=y2;
			 
			}
			 
			else {
			     
				     ecg_data_flag =1;
			       index_=0;
			}
			 index_++;
       stop_count=1;				
		}
			 
			w0[2]=w0[1];
			w0[1]=w0[0];
			w1[2]=w1[1];
			w1[1]=w1[0]; 
      w2[2]=w2[1];
			w2[1]=w2[0];
			
			if(!stop_count)
       count++;	
		
		QRS_Algorithm_Interface(y);
		 return y;
}

void ECG_ProcessCurrSample(int16_t *CurrAqsSample, int16_t *FilteredOut)
{
  
            float y=0;

			w1[0]=IIR_High_A[0]*(*CurrAqsSample)-IIR_High_A[1]*w1[1]-IIR_High_A[2]*w1[2];
			y1=Gain*(IIR_High_B[0]*w1[0]+IIR_High_B[1]*w1[1]+IIR_High_B[2]*w1[2]);
		 
		  w2[0]=IIR_LOW_A[0]*y1-IIR_LOW_A[1]*w2[1]-IIR_LOW_A[2]*w2[2];
			y2=Gain*(IIR_LOW_B[0]*w2[0]+IIR_LOW_B[1]*w2[1]+IIR_LOW_B[2]*w2[2]);
		
	
		 if(count>500){
		//IIR_Filter_Data[i]=y1;         //缓存数组储存滤波之后的信?		   //printf("\r\n%.5f",y2);
			 y=y2; 
			 
		 if(index_<1000)
			 {
				 
			 ECG_DATA[index_]=y2;
			 
			}
			 
			else {
			     
				     ecg_data_flag =1;
			       index_=0;
			}
			 index_++;
       stop_count=1;				
		}
			 
			w0[2]=w0[1];
			w0[1]=w0[0];
			w1[2]=w1[1];
			w1[1]=w1[0]; 
      w2[2]=w2[1];
			w2[1]=w2[0];
			
			if(!stop_count)
       count++;	
		
		QRS_Algorithm_Interface(y);
		 return y;
  
  
  return ;
}


	





static void QRS_check_sample_crossing_threshold( unsigned short scaled_result )
{
	/* array to hold the sample indexes S1,S2,S3 etc */
	
	static unsigned short s_array_index = 0 ;
	static unsigned short m_array_index = 0 ;
	
	static unsigned char threshold_crossed = FALSE ;
	static unsigned short maxima_search = 0 ;
	static unsigned char peak_detected = FALSE ;
	static unsigned short skip_window = 0 ;
	static long maxima_sum = 0 ;
	static unsigned int peak = 0;
	static unsigned int sample_sum = 0;
	static unsigned int nopeak=0;
	unsigned short max = 0 ;
	unsigned short HRAvg;

	
	if( TRUE == threshold_crossed  )
	{
		/*
		Once the sample value crosses the threshold check for the
		maxima value till MAXIMA_SEARCH_WINDOW samples are received
		*/
		sample_count ++ ;
		maxima_search ++ ;

		if( scaled_result > peak )
		{
			peak = scaled_result ;
		}

		if( maxima_search >= MAXIMA_SEARCH_WINDOW )
		{
			// Store the maxima values for each peak
			maxima_sum += peak ;
			maxima_search = 0 ;

			threshold_crossed = FALSE ;
			peak_detected = TRUE ;
		}

	}
	else if( TRUE == peak_detected )
	{
		/*
		Once the sample value goes below the threshold
		skip the samples untill the SKIP WINDOW criteria is meet
		*/
		sample_count ++ ;
		skip_window ++ ;

		if( skip_window >= MINIMUM_SKIP_WINDOW )
		{
			skip_window = 0 ;
			peak_detected = FALSE ;
		}

		if( m_array_index == MAX_PEAK_TO_SEARCH )
		{
			sample_sum = sample_sum / (MAX_PEAK_TO_SEARCH - 1);
			HRAvg =  (unsigned short) sample_sum  ;
//#if 0
		//	if((LeadStatus & 0x0005)== 0x0000)
		//	{
				
			//QRS_Heart_Rate = (unsigned short) 60 *  SAMPLING_RATE;
			//QRS_Heart_Rate =  QRS_Heart_Rate/ HRAvg ;
			//	if(QRS_Heart_Rate > 250)
			//		QRS_Heart_Rate = 250 ;
			//}
			//else
			//{
			//	QRS_Heart_Rate = 0;
			//}
//#else
			// Compute HR without checking LeadOffStatus
			QRS_Heart_Rate1 = (unsigned short) 60 *  SAMPLING_RATE;
			QRS_Heart_Rate1 =  QRS_Heart_Rate1/ HRAvg ;
			if(QRS_Heart_Rate1 > 250)
				QRS_Heart_Rate1 = 250 ;
			
//#endif

			/* Setting the Current HR value in the ECG_Info structure*/

			HR_flag = 1;

			maxima_sum =  maxima_sum / MAX_PEAK_TO_SEARCH;
			max = (short) maxima_sum ;
			/*  calculating the new QRS_Threshold based on the maxima obtained in 4 peaks */
			maxima_sum = max * 7;
			maxima_sum = maxima_sum/10;
			QRS_Threshold_New = (short)maxima_sum;

			/* Limiting the QRS Threshold to be in the permissible range*/
			if(QRS_Threshold_New > (4 * QRS_Threshold_Old))
			{
				QRS_Threshold_New = QRS_Threshold_Old;
	 		}

	 		sample_count = 0 ;
	 		s_array_index = 0 ;
	 		m_array_index = 0 ;
	 		maxima_sum = 0 ;
			sample_index[0] = 0 ;
			sample_index[1] = 0 ;
			sample_index[2] = 0 ;
			sample_index[3] = 0 ;
			Start_Sample_Count_Flag = 0;

			sample_sum = 0;
		}
	}
	else if( scaled_result > QRS_Threshold_New )
	{
		/*
			If the sample value crosses the threshold then store the sample index
		*/
		Start_Sample_Count_Flag = 1;
		sample_count ++ ;
		m_array_index++;
		threshold_crossed = TRUE ;
		peak = scaled_result ;
		nopeak = 0;

		/*	storing sample index*/
	   	sample_index[ s_array_index ] = sample_count ;
		if( s_array_index >= 1 )
		{
			sample_sum += sample_index[ s_array_index ] - sample_index[ s_array_index - 1 ] ;
		}
		s_array_index ++ ;
	}

	else if(( scaled_result < QRS_Threshold_New ) && (Start_Sample_Count_Flag == 1))
	{
		sample_count ++ ;
        nopeak++;	
        if (nopeak > (3 * SAMPLING_RATE))
        { 
        	sample_count = 0 ;
	 		s_array_index = 0 ;
	 		m_array_index = 0 ;
	 		maxima_sum = 0 ;
			sample_index[0] = 0 ;
			sample_index[1] = 0 ;
			sample_index[2] = 0 ;
			sample_index[3] = 0 ;
			Start_Sample_Count_Flag = 0;
			peak_detected = FALSE ;
			sample_sum = 0;
        	    	
        	first_peak_detect = FALSE;
	      	nopeak=0;

			QRS_Heart_Rate = 0;
			HR_flag = 1;
        }
	}
   else
   {
    nopeak++;	
   	if (nopeak > (3 * SAMPLING_RATE))
     { 
		/* Reset heart rate computation sate variable in case of no peak found in 3 seconds */
 		sample_count = 0 ;
 		s_array_index = 0 ;
 		m_array_index = 0 ;
 		maxima_sum = 0 ;
		sample_index[0] = 0 ;
		sample_index[1] = 0 ;
		sample_index[2] = 0 ;
		sample_index[3] = 0 ;
		Start_Sample_Count_Flag = 0;
		peak_detected = FALSE ;
		sample_sum = 0;
     	first_peak_detect = FALSE;
	 	nopeak = 0;
		QRS_Heart_Rate = 0;
		HR_flag = 1;

     }
   }

}

static void QRS_process_buffer( void )
{

	short first_derivative = 0 ;
	short scaled_result = 0 ;

	static short max = 0 ;

	/* calculating first derivative*/
	first_derivative = QRS_Next_Sample - QRS_Prev_Sample  ;

	/*taking the absolute value*/

	if(first_derivative < 0)
	{
		first_derivative = -(first_derivative);
	}

	scaled_result = first_derivative;
	
	

	if( scaled_result > max )
	{
		max = scaled_result ;
	}

	QRS_B4_Buffer_ptr++;
	if (QRS_B4_Buffer_ptr ==  TWO_SEC_SAMPLES)
	{
		QRS_Threshold_Old = ((max *7) /10 ) ;
		QRS_Threshold_New = QRS_Threshold_Old ;
		if(max > 70)
		first_peak_detect = TRUE ;
		max = 0;
		QRS_B4_Buffer_ptr = 0;
	}


	if( TRUE == first_peak_detect )
	{
		QRS_check_sample_crossing_threshold( scaled_result ) ;
	}
}



void QRS_Algorithm_Interface(short CurrSample)
{
//	static FILE *fp = fopen("ecgData.txt", "w");
	static short prev_data[32] ={0};
	short i;
	long Mac=0;
	prev_data[0] = CurrSample;
	for ( i=31; i > 0; i--)
	{
		Mac += prev_data[i];
		prev_data[i] = prev_data[i-1];

	}
	Mac += CurrSample;
	Mac = Mac >> 2;
	CurrSample = (short) Mac;
	QRS_Second_Prev_Sample = QRS_Prev_Sample ;
	QRS_Prev_Sample = QRS_Current_Sample ;
	QRS_Current_Sample = QRS_Next_Sample ;
	QRS_Next_Sample = QRS_Second_Next_Sample ;
	QRS_Second_Next_Sample = CurrSample ;
	QRS_process_buffer();
}