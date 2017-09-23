#pragma once

#include <vector>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <random>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <sstream>


using namespace std;


class LinearRegression
{
public:
	LinearRegression();

	void setInputFeatureCount(int count);
	void setInitLearningRate(double init) { initLearningRate = init; }
	void initWeight();

	double activation(vector<double>& inputs);
	void train(vector<vector<double>>& inputs, vector<double>& expectOutput);
	void numericGradientCalculate(vector<vector<double>>& inputs, vector<double>& expectOutput);

	void serialize();
	void deserialize();

private:
	vector<double> weights;
	vector<double> accumulatedGradient;
	double biasWeight;
	double biasAccumulatedGradient;

	double initLearningRate;
};
