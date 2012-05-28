# analyze_accuracy.py
# Chris Whiten - 2012
#
# Dependent on 'matplotlib', which is used for displaying the graphs.
#
# Run as: 'pythong analyze_accuracy.py'.  This should be in the root VideoParser folder, 
# and it will go back one folder to look for the 'Experiments' folder to grab the results files.

"""
Parses through the results of object recognition and outputs the recognition accuracy.
Also graphs the accuracy over frames with matplotlib's graphing functionality
[TODO]: Add precision/recall and other relevant statistics.
"""
__author__ = 'Chris Whiten'

import matplotlib.pyplot as plt
from os import path

def parseSimpleMatchFile(stem):
	"""Parse simple results file printed by Shape Context recognition"""
	results = []
	file_path = path.relpath("../Experiments/" + stem)
	
	with open(file_path) as f:
		line = f.readline()
		while (line != ""):
			# this assumes a format of: ([query frame], [matched frame], [matching score], [1 if match])
			individual_pieces = line.replace("(", "").replace(")", "").strip().split(",")
			# and all we need right now is the last piece...
			results.append((int)(individual_pieces[-1].strip()))
			line = f.readline()
	return results
	
def parseComplexMatchFile(stem):
	"""Parse the more complex results file printed by SPG recognition"""
	match_lines = []
	file_path = path.relpath("../Experiments/" + stem)
	
	# first, parse through the file and get all lines containing matches.
	with open(file_path) as f:
		line = f.readline()
		while (line != ""):
			words = line.split(" ")
			if words[0].startswith("frame") and words[0].endswith("matches"):
				match_lines.append(line)
			line = f.readline()
		
	# next, go through and append the matching result from each line.
	# here we actually get the results for all recorded matches,
	# not just the first one.  This is just part of the transition
	# to including precision/recall... for now, we only end up 
	# using the first one.
	frames = []
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
	return frames

def parseRecognitionResults(spg_matching_stems, shape_context_stem):
	"""Parse object recognition results and put them into a graphable format"""
	spg_results = []
	for stem in spg_matching_stems:
		spg_results.append(parseComplexMatchFile(stem))
	shape_context_results = parseSimpleMatchFile(shape_context_stem)
	return (spg_results, shape_context_results)

	
def computeMAPAssignments(results):
	"""Select the best matched result from SPG recognition"""
	MAP_assignments = []
	for experiment in results:
		MAP_assignment = []
		for frame in experiment:
			MAP_assignment.append(frame[0])
		MAP_assignments.append(MAP_assignment)
	return MAP_assignments
	
def computeGraphableAccuracy(experiment):
	"""Construct graphable list from parsed object recognition results"""
	results = []
	total_matches = 0
	correct_matches = 0
	
	for match in experiment:
		total_matches += 1
		if match == 1:
			correct_matches += 1
		results.append(float(correct_matches)/float(total_matches)*100)
	return results

def graphRecognitionAccuracy(spg_results, spg_legends, shape_context_results):
	"""Graph object recognition accuracy with matplotlib"""
	# first, get the accuracy to graph at each y-coordinate.
	graphable_results = []
	for experiment in spg_results:
		graphable_result = computeGraphableAccuracy(experiment)
		graphable_results.append(graphable_result)
		
	graphable_shapecontext_results = computeGraphableAccuracy(shape_context_results)
		
	# add shape context to results.
	results = graphable_results
	results.append(graphable_shapecontext_results)
	legends = spg_legends
	legends.append("Shape Context")
	
	# report results in the console
	i = 0
	for legend in legends:
		print "-----------------------"
		print legend
		print "-----------------------"
		print "Final accuracy:", results[i][-1], "%"
		i += 1
	
	# now graph it
	for y_data, name in zip(results, legends):
		plt.plot(y_data, label = name)
	plt.legend(loc = 'best')
	plt.ylabel('Recognition accuracy (percentage)')
	plt.xlabel('Frame number')
	plt.show()
			
if __name__ == '__main__':
	spg_matching_stems = ["recognition_results.txt"] # add to this list (and the spg_legends list) to graph multiple results against each other.
	spg_legends = ["SPG match"]
	shape_context_stem = "shapecontext_results.txt"
	
	# spg_results is of the form: spg_results[i] = results for file i.  
	# spg_results[i][j] = results for the j'th frame in file i
	# spg_results[i][j][k] = kth best result for the jth frame in file i.
	#
	# shape_context_reults is of the form shape_context_results[i] = whether or not frame i's MAP assignment was correct.
	spg_results, shape_context_results = parseRecognitionResults(spg_matching_stems, shape_context_stem)
	MAP_assignments = computeMAPAssignments(spg_results)
	graphRecognitionAccuracy(MAP_assignments, spg_legends, shape_context_results)