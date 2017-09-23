#pragma once
#include "neuron.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <set>
#include <math.h>


enum lossFunctionType {POLICY_GRADIENT, MSE, LOGISTIC};

class Network
{
	public:
		Network();
		void copy(Network n);
		void linkNeurons();
		void setNumInputs(int ninp);
		void setDepth(int depth);
		void setNodeCountAtLevel(int count, int level, activeFunctionType af);
		void setLearningRate(double rate);

		void initNetwork();

		void numericGradientCalculate(vector<vector<double>> multiInputs, vector<int> actualChosenIndex, vector<vector<int>> outputChosen, bool isMSE, vector<double> expectValue);
		vector<double> feedForward(vector<double> inputs);

		double train(vector<vector<double>>& multiInputs, lossFunctionType loss,
			const vector<double>& expectValue = vector<double>(), 
			const vector<int>& actualChosen = vector<int>(),
			const vector<vector<int>>& outputChosen = vector<vector<int>>(),
			const vector<double>& qValues = vector<double>(),
			const vector<int>& indexValue = vector<int>(),
			double totalMatchCount = 1);

		void printNetwork();

		void serialize(string modelName, string pathName);
		int deserialize(string modelName, string pathName);

		void targetNetworkUpdate(Network& n);

	private:

		//network only include hidden layer and output layer
		vector<vector<Neuron>> network;
};

