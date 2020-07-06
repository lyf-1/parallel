hadoop fs -rm -r /tmp
hadoop fs -rm -r /output/MinNewYork2013
hadoop jar MinValue.jar MinValue /input/NewYork2013.txt /output/MinNewYork2013
hadoop fs -rm -r /tmp
hadoop fs -rm -r /output/MinLondon2013
hadoop jar MinValue.jar MinValue /input/London2013.txt /output/MinLondon2013
hadoop fs -rm -r /tmp
hadoop fs -rm -r /output/MinMumbai2013
hadoop jar MinValue.jar MinValue /input/Mumbai2013.txt /output/MinMumbai2013
hadoop fs -rm -r /tmp
hadoop fs -rm -r /output/MinSFO2012
hadoop jar MinValue.jar MinValue /input/SFO2012.txt /output/MinSFO2012
hadoop fs -rm -r /tmp
hadoop fs -rm -r /output/MinSFO2013
hadoop jar MinValue.jar MinValue /input/SFO2013.txt /output/MinSFO2013