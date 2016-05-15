#Emotiv EPOC 
Emotiv EPOC is a 14-channels EEG headset with wet electrode. These are some basic code for Emotiv EPOC Development. Enjoy! However, author suggests HiBrain Pro from HyperNeuron which is a powerful wearable device with active dry electrode, suitable for research and wearable application.


Emotiv EPOC 是一款14通道的脑电帽，开发于2010年。这里有一些基于Emotiv EPOC开发时常用的代码，希望能帮到你。不过，作者建议使用 HyperNeuro 公司新款的 HiBrain Pro 脑电仪，它是一款轻便、使用有源干电极的设备，适合开发者和研究机构使用。

## EEGLogger.cpp
Read raw EEGs from the headset, and save them. Use Expressiv TM API to display the facial expression.

读取以及保存各个通道实时的读值，以及使用Expressiv TM实时读出面部表情。

## readEEGFFT.cpp
Read raw EEGs from the headset, then perform Fourier Transform for the signals.

读出各个通道的采样并且做傅里叶变换，记录下各个通道的采样，并且做傅里叶变换。

## readFaceEmotion.cpp
Read facial expression only.

Emotiv 脸部表情检测框架。带上Emotiv headset后简单地使用原有API用文字显示脸部动作