import java.io.IOException;
import java.util.Iterator;
import org.apache.hadoop.conf.Configuration; 
import org.apache.hadoop.conf.Configured; 
import org.apache.hadoop.fs.Path; 
import org.apache.hadoop.io.LongWritable; 
import org.apache.hadoop.io.IntWritable; 
import org.apache.hadoop.io.FloatWritable;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text; 
import org.apache.hadoop.mapreduce.Job; 
import org.apache.hadoop.mapreduce.Mapper; 
import org.apache.hadoop.mapreduce.Reducer; 
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat; 
import org.apache.hadoop.mapreduce.lib.input.TextInputFormat; 
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat; 
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat; 
import org.apache.hadoop.util.Tool; 
import org.apache.hadoop.util.ToolRunner; 
 
public class MaxValue extends Configured implements Tool { 
	
	public static class MapClass extends Mapper<LongWritable, Text, Text, DoubleWritable> { 
		private double maxNum = 0.0; 
		public void map(LongWritable key, Text value, Context context) 
				throws IOException, InterruptedException { 
			String[] str = value.toString().split(" "); 
            try {
                double temp = Double.parseDouble(str[str.length-1]);
                if (temp > maxNum) {
                    maxNum = temp;
                }
			} catch (NumberFormatException e) { 
			} 
		} 
		@Override 
		protected void cleanup(Context context) throws IOException, 
		InterruptedException { 
			context.write(new Text("Max"), new DoubleWritable(maxNum)); 
		} 
	} 
 
	public static class Reduce extends Reducer<Text, DoubleWritable, Text, DoubleWritable> { 
		private double maxNum = 0.0; 
		private Text one = new Text();
		public void reduce(Text key, Iterable<DoubleWritable> values, Context context) 
				throws IOException, InterruptedException { 
            Iterator<DoubleWritable> it = values.iterator();
            // Method 1
            while (it.hasNext()){
                double temp = ((DoubleWritable)(it.next())).get();
                if (temp > maxNum) {
                    maxNum = temp;
                }
            }
            // Method 2
			// for (DoubleWritable val : values) { 
			// 	if ( val.get() > maxNum) { 
			// 		maxNum = val.get(); 
			// 	} 
			// } 
			one = key;
		} 
		@Override 
		protected void cleanup(Context context) throws IOException, 
		InterruptedException { 
			context.write(one, new DoubleWritable(maxNum));
		} 
	} 
 
	public int run(String[] args) throws Exception { 
		Configuration conf = getConf(); 
		conf.set("mapred.jar","mv.jar");
		Job job = new Job(conf, "MaxNum"); 
		job.setJarByClass(MaxValue.class); 
		FileInputFormat.setInputPaths(job, new Path(args[0])); 
		FileOutputFormat.setOutputPath(job, new Path(args[1])); 
		job.setMapperClass(MapClass.class); 
		job.setCombinerClass(Reduce.class); 
		job.setReducerClass(Reduce.class); 
		job.setInputFormatClass(TextInputFormat.class); 
		job.setOutputFormatClass(TextOutputFormat.class); 
		job.setOutputKeyClass(Text.class); 
		job.setOutputValueClass(DoubleWritable.class); 
		System.exit(job.waitForCompletion(true) ? 0 : 1); 
		return 0; 
	} 
 
	public static void main(String[] args) throws Exception { 
		long start = System.nanoTime(); 
		int res = ToolRunner.run(new Configuration(), new MaxValue(), args); 
		System.out.println(System.nanoTime()-start); 
		System.exit(res); 
	} 
}
