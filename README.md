# electrocardiogram-system-using-stm32f103
This is an electrocardiogram acquisition device using an ADS1292 Texa instrument IC interfaced with an SMT32F103 microcontroller. 
All the algorithms including, the heart rate calculation, the digital filters were done on MATLAB, then transferred to STM32 microcontroller. The data was also sent to an android app to display the waveform in real-time.
A comparison of the heart rate of the device was done against the standard device heart rate . The error rate was less than 1%.

The short video illustrate the system running without transfering the data to the mobile phone.


[![electrocardiogram-system-using-stm32f103](https://img.youtube.com/vi/JplmBYr1th0/0.jpg)](https://www.youtube.com/watch?v=JplmBYr1th0)

The following diagram illustrate the components of the system. As we can see, the system has a HC05 module, a microcontroller, an ADS1292R electrocardiogram analog front end (AFE), a LCD. The AFE communicates with the STM32F103 througth the SPI ptotocol.


![BBBBB](https://user-images.githubusercontent.com/22806623/190268504-d89c6fc2-6d02-4b60-9f50-7fe7364c47a6.png)


Below shows the system working along with the bluetooth module and the android app:


[![electrocardiogram-system-using-stm32f103](https://img.youtube.com/vi/aHwbFrkWZwA/0.jpg)](https://www.youtube.com/watch?v=aHwbFrkWZwA)
[![electrocardiogram-system-using-stm32f103](https://img.youtube.com/vi/aHwbFrkWZwA/0.jpg)](https://www.youtube.com/watch?v=aHwbFrkWZwA)

Software and programming language used in this project:

1. MATLAB
2. Keil
3. C programming language 
4. android studio
