#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <cstdlib>
#include <math.h>
#include <random>
#include <time.h>
#include <sstream>

using namespace std;

enum activeFunctionType {SIGMOD, SOFTMAX, SUM, Relu };

class Neuron
{
	public:
		Neuron();
		double activation(vector<double>& inputs, vector<int> noneZeroIndex = vector<int>());
		double activationChange(vector<double> inputs, int weightIndex, bool addEpsilon);

		double getBpDerivative();

		void setActiveFunction(activeFunctionType af) { functionType = af; }
		void setLearningRate(double nLearn);
		void adjustForError(bool isDescent);
		void debugPrintGradient(int layer, int neuronIndex);
		void setNumInputs(int numInputs);
		void resetWeights(int layer, int index);
		void print();

		double sigmoid(double val);
		double softmax(double val);

		void setErrorSignal(double e);
		int getInputNum() { return weights.size(); }
		double getWeight(int index) { return weights[index]; }
		double getErrorSignal() { return error; }
		double getActivationResult() { return activationResult; }
		vector<double>& getAllWeight() { return weights; }

		string serialize();
		void deserialize(string input);
		

		vector<double> weights;
		double biasWeight;
	private:
		activeFunctionType functionType;
		vector<double> accumulatedGradient;
		double biasAccumulatedGradient;
		double learningRate;

		int	visitCount;
		double error;

		vector<double> lastInp;
		double activationResult;

		vector<double> batchGradient;
		double batchBiasGradient;
		
};

