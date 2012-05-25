# analyze_accuracy.py
# Chris Whiten - 2012
#
# Parses through the results of object recognition and outputs the recognition accuracy.
# Also graphs the accuracy over frames with matplotlib's graphing functionality
# [TODO]: Add precision/recall and other relevant statistics.
# Also, it's all pretty hacky... Make it more extensible and "elegant" at some point.

import matplotlib.pyplot as plt
from os import path

if __name__ == '__main__':
	stem = "recognition_results.txt"
	file_path = path.relpath("../Experiments/" + stem)
	f = open(file_path)
	line = f.readline()
	match_lines = []
	frames = []
	while (line != ""):
		words = line.split(" ")
		for word in words:
			# if we are in a matching line.
			if word.startswith("frame") and word.endswith("matches"):
				match_lines.append(line)
				
		line = f.readline()
	
	for match in match_lines:
		words = match.split("(")
		if (len(words) > 1):
			results = []
			del words[0] # remove the prefix.
			for word in words:
				word = word.strip(")").strip(" ")
				if word.endswith("1"):
					results.append(1)
				elif word.endswith("0"):
					results.append(0)
					
			frames.append(results)
			
	# now plot this.
	x_coords = []
	y_coords = []
	x = 0
	y = 0
	total_count = 0
	correct_count = 0
	incorrect_count = 0
	errors = []
	for frame in frames:
		if frame[0] == 1:
			correct_count += 1
			y = 1
		else:
			incorrect_count += 1
			y = 0
		errors.append(incorrect_count)
			
		x_coords.append(x)
		x += 1
		total_count += 1
		current_accuracy = (float(total_count - errors[-1])/float(total_count)*100)
		y_coords.append(current_accuracy)
		
		
	print "good matches:", correct_count
	print "bad matches:", incorrect_count
	print "accuracy: ", float(correct_count)/float(correct_count + incorrect_count)*100
	
	
	
	# print csv.
	csv_path = path.relpath("../Experiments/" + stem + ".csv")
	csv = open(csv_path, "w")
	csv.write(stem + ", ")
	for i in range(len(y_coords)):
		csv.write(str(y_coords[i]) + ", ")
	csv.close()
	
	plt.plot(x_coords, y_coords, "bo")
	plt.ylabel('match or no match')
	plt.xlabel('frame')
	plt.show()