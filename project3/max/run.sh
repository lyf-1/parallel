hadoop fs -rm -r /tmp
hadoop fs -rm -r /output/MaxNewYork2013
hadoop jar MaxValue.jar MaxValue /input/NewYork2013.txt /output/MaxNewYork2013
hadoop fs -rm -r /tmp
hadoop fs -rm -r /output/MaxLondon2013
hadoop jar MaxValue.jar MaxValue /input/London2013.txt /output/MaxLondon2013
hadoop fs -rm -r /tmp
hadoop fs -rm -r /output/MaxMumbai2013
hadoop jar MaxValue.jar MaxValue /input/Mumbai2013.txt /output/MaxMumbai2013
hadoop fs -rm -r /tmp
hadoop fs -rm -r /output/MaxSFO2012
hadoop jar MaxValue.jar MaxValue /input/SFO2012.txt /output/MaxSFO2012
hadoop fs -rm -r /tmp
hadoop fs -rm -r /output/MaxSFO2013
hadoop jar MaxValue.jar MaxValue /input/SFO2013.txt /output/MaxSFO2013
