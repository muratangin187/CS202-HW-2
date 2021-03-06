#include "DecisionTree.h"
#include <iostream>
#include <fstream>
#include <cmath>

void DecisionTree::train(const bool** data, const int* labels, const int numSamples, const int numFeatures) {
	rootPtr = new DecisionTreeNode();
	bool* usedFeatures = new bool[numFeatures];
	bool* usedSamples = new bool[numSamples];
	for (size_t i = 0; i < numFeatures; i++) {
		usedFeatures[i] = false;
		usedSamples[i] = true;
	}
	train(rootPtr, data, labels, usedSamples, usedFeatures, numSamples, numFeatures);
	delete[] usedFeatures;
	delete[] usedSamples;
}

DecisionTree::~DecisionTree() {
	delete rootPtr;
}

void DecisionTree::train(DecisionTreeNode* nodePtr, const bool** data, const int* labels, bool* usedSamples, bool* usedFeatures, const int numSamples, const int numFeatures) {
	int currentLabel = -1;
	int lastLabel = -1;
	bool isPure = true;
	int valueForPureClass = -1; // class type if data is pure already
	for (size_t i = 0; i < numSamples; i++)
	{
		if (usedSamples[i]) {
			valueForPureClass = labels[i];
			if (currentLabel == -1) {
				currentLabel = labels[i];
			}else{
				lastLabel = currentLabel;
				currentLabel = labels[i];
				if (lastLabel != currentLabel) {
					isPure = false;
					break;
				}
			}
		}
	}
	//if (currentSampleCount == 1 || currentSampleCount == 0) isPure = true;
	if (lastLabel == -1) isPure = true;
	if (isPure) {	// if data is pure, return class=id as a result.
		nodePtr->setItemId(valueForPureClass);
		return;
	}

	bool isAllFeaturesUsed = true;
	for (size_t i = 0; i < numFeatures; i++) // check all features are used or not?
		if (usedFeatures[i] == false)
			isAllFeaturesUsed = false;

	// if all features are used while data is not pure,
	// make a decision by looking the majority class in data
	if (isAllFeaturesUsed) {
		// create a class node(leaf) by choosing the majority class within the set of samples.
		int max = -1;
		for (size_t i = 0; i < numSamples; i++)
		{
			if (usedSamples[i]) {
				if (max < labels[i]) max = labels[i];	// find the upper limit of class numbers
			}
		}
		int* classValues = new int[max];
		for (size_t i = 0; i < max; i++)
			classValues[i] = 0;

		for (size_t i = 0; i < numSamples; i++)
			if (usedSamples[i])
				classValues[labels[i] - 1]++;

		int resultClass = 1;
		for (size_t i = 0; i < max; i++)
		{
			if (classValues[resultClass-1] < classValues[i]) {
				resultClass = i+1;
			}
		}
		delete[] classValues;
		nodePtr->setItemId(resultClass);
		nodePtr->setLeftChild(nullptr);
		nodePtr->setRightChild(nullptr);
	}
	else {
		// make a split according to best choice
		double maxGain = 0.0;
		int selectedFeatureId = 0;	// best feature to split data
		for (size_t i = 0; i < numFeatures; i++)
		{
			if (usedFeatures[i] == false) {
				double currentGain = calculateInformationGain(data, labels, numSamples, numFeatures, usedSamples, i);
				if (maxGain < currentGain)
				{
					selectedFeatureId = i;
					maxGain = currentGain;
				}
			}
		}
		bool* leftUsedSamples = new bool[numSamples];
		bool* rightUsedSamples = new bool[numSamples];
		usedFeatures[selectedFeatureId] = true;

		for (size_t i = 0; i < numSamples; i++)
		{
			if (usedSamples[i]) {
				if (data[i][selectedFeatureId] == 0) {
					leftUsedSamples[i] = usedSamples[i];
					rightUsedSamples[i] = false;
				}
				else {
					rightUsedSamples[i] = usedSamples[i];
					leftUsedSamples[i] = false;
				}
			}
			else {
				leftUsedSamples[i] = false;
				rightUsedSamples[i] = false;
			}
		}

		nodePtr->setItemId(selectedFeatureId);
		nodePtr->setLeftChild(new DecisionTreeNode());
		nodePtr->setRightChild(new DecisionTreeNode());
		train(nodePtr->getLeftChild(), data, labels, leftUsedSamples, usedFeatures, numSamples, numFeatures);
		train(nodePtr->getRightChild(), data, labels, rightUsedSamples, usedFeatures, numSamples, numFeatures);
		delete[] leftUsedSamples;
		delete[] rightUsedSamples;
	}
	
}

void DecisionTree::train(const string fileName, const int numSamples, const int numFeatures) {
	string lineString;		// string contains current line in file
	ifstream file;
	file.open(fileName);	// open file

	int* labels = new int[numSamples];

	bool** array = new bool*[numSamples];	// create a temp array for const bool array
	for (size_t i = 0; i < numSamples; i++)
	{
		array[i] = new bool[numFeatures];
	}
	int i = 0;
	while (!(file.eof()))		// until there is no new line in file
	{
		getline(file, lineString);	// get line to lineString
		int pos = 0;
		int j = 0;
		while (pos = lineString.find(" ") != std::string::npos) {	// if lineString contains space
			string selected = lineString.substr(0, pos);
			array[i][j] = std::stoi(selected); // get binary number to array
			lineString.erase(0, pos + 1);
			j++;
		}
		if(i != numSamples)
			labels[i] = std::stoi(lineString);	// get class result from last digits of line
		i++;
	}

	file.close();

	const bool* const* data = array;

	train((const bool**)data, labels, numSamples, numFeatures);
	delete[] labels;
	for (size_t i = 0; i < numSamples; i++)
	{
		//delete[] data[i];
		delete[] array[i];
	}
	delete[] array;
}

int DecisionTree::predict(const bool* data) {
	DecisionTreeNode* currentNode = rootPtr;
	while (currentNode->getLeftChild() != nullptr && currentNode->getRightChild() != nullptr) {
		// if not leaf
		int featureId = currentNode->getItemId();
		if (data[featureId] == 0)
			currentNode = currentNode->getLeftChild();
		else
			currentNode = currentNode->getRightChild();
	}
	return currentNode->getItemId();
}

double DecisionTree::test(const bool** data, const int* labels, const int numSamples) {
	int trueCount = 0;
	for (size_t i = 0; i < numSamples; i++)
		if (labels[i] == predict(data[i]))
			trueCount++;

	return (double)trueCount / numSamples;
}

double DecisionTree::test(const string fileName, const int numSamples) {
	string lineString;
	ifstream file;
	file.open(fileName);
	streampos oldpos = file.tellg(); // store file beginning
	string lineForNumber;
	getline(file, lineForNumber);	// get first line of file
	file.seekg(oldpos);	// come back to file beginning
	int j = 0;	// feature size
	while (lineForNumber.find(" ") != std::string::npos) {
		lineForNumber.erase(0, lineForNumber.find(" ") + 1);
		j++;
	}
	bool** array = new bool* [numSamples];
	int* labels = new int[numSamples];

	for (size_t i = 0; i < numSamples; i++)
	{
		array[i] = new bool[j];	// create array in feature size(j)
	}
	int i = 0;
	while (!(file.eof()))
	{
		getline(file, lineString);
		int pos = 0;
		int j = 0;
		while (pos = lineString.find(" ") != std::string::npos) {
			string selected = lineString.substr(0, pos);
			array[i][j] = std::stoi(selected);
			lineString.erase(0, pos + 1);
			j++;
		}
		if (i != numSamples)
			labels[i] = std::stoi(lineString);
		i++;
	}
	const bool* const* data = array;

	file.close();

	double result = test((const bool**)data, labels, numSamples);
	delete[] labels;
	for (size_t i = 0; i < numSamples; i++)
	{
		delete[] data[i];
	}
	delete[] data;
	return result;
}

void DecisionTree::print() {
	print(rootPtr, 0);
}

void DecisionTree::print(DecisionTreeNode* nodePtr, int height) {
	if (nodePtr == nullptr)
		return;
	for (size_t i = 0; i < height; i++)
		cout << '\t';
	if (nodePtr->getLeftChild() == nullptr && nodePtr->getRightChild() == nullptr)
		cout << "class=";
	cout << nodePtr->getItemId() << endl;
	print(nodePtr->getLeftChild(), height + 1);
	print(nodePtr->getRightChild(), height + 1);
}

double DecisionTree::calculateEntropy(const int* classCounts, const int numClasses) {
	double result = 0;
	double sum = 0.0;
	for (size_t i = 0; i < numClasses; i++)
		sum += classCounts[i];
	for (size_t i = 0; i < numClasses; i++)
		if (classCounts[i] != 0)
			result += classCounts[i] * log2(classCounts[i] / sum) / sum;
	return -result;
}

double DecisionTree::calculateInformationGain(const bool** data, const int* labels,
	const int numSamples, const int numFeatures, const bool* usedSamples,
	const int featureId) {

	double hP = 0;	// hP = calculateEntropy()
	double hS = 0;	// hS = pL * hL + pR* hR
	double pL = 0;
	double hL = 0;  // hL = calculateEntropy()
	double pR = 0;
	double hR = 0;  // hR = calculateEntropy()
	int classSize = 0;

	bool** usedData;
	int* usedLabels;
	int usedDataCount = 0;
	for (size_t i = 0; i < numSamples; i++)
		if (usedSamples[i]) {
			usedDataCount++;
			if (labels[i] > classSize)
				classSize = labels[i];
		}

	usedData = new bool* [usedDataCount];	// my data contains only used ones
	usedLabels = new int[usedDataCount];	// my data's labels contains only used ones

	int indexOfUsed = 0;
	for (size_t i = 0; i < numSamples; i++)
	{
		if (usedSamples[i]) {
			usedData[indexOfUsed] = new bool[numFeatures];
			for (size_t j = 0; j < numFeatures; j++)
			{
				usedData[indexOfUsed][j] = data[i][j];
			}
			usedLabels[indexOfUsed] = labels[i];
			indexOfUsed++;
		}
	}

	int* classCounts = new int[classSize];
	int* leftClassCounts = new int[classSize];
	int leftSize = 0;
	int* rightClassCounts = new int[classSize];
	int rightSize = 0;

	for (size_t i = 0; i < classSize; i++)
	{
		classCounts[i] = 0;
		leftClassCounts[i] = 0;
		rightClassCounts[i] = 0;
	}

	for (size_t i = 0; i < usedDataCount; i++)
	{
		for (size_t j = 0; j < classSize; j++)
			if (usedLabels[i] == j + 1)
				classCounts[j]++;

		if (usedData[i][featureId] == 0) {
			leftSize++;
			for (size_t j = 0; j < classSize; j++)
				if (usedLabels[i] == j + 1)
					leftClassCounts[j]++;
		}
		else {
			rightSize++;
			for (size_t j = 0; j < classSize; j++)
				if (usedLabels[i] == j + 1)
					rightClassCounts[j]++;
		}
	}
	for (size_t i = 0; i < usedDataCount; i++)
	{
		delete[] usedData[i];
	}
	delete[] usedData;
	delete[] usedLabels;

	hP = calculateEntropy(classCounts, classSize);
	hL = calculateEntropy(leftClassCounts, classSize);
	hR = calculateEntropy(rightClassCounts, classSize);

	delete[] classCounts;
	delete[] leftClassCounts;
	delete[] rightClassCounts;

	pL = (double)leftSize / usedDataCount;
	pR = (double)rightSize / usedDataCount;
	hS = pL * hL + pR * hR;
	return hP - hS;
}