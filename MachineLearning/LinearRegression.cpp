#pragma once
#include "LinearRegression.h"



LinearRegression::LinearRegression()
{
	biasAccumulatedGradient = 0;

}


void LinearRegression::setInputFeatureCount(int count)
{
	weights.resize(count);
	accumulatedGradient.resize(count);
	for (auto& i : accumulatedGradient)
	{
		i = 0;
	}
}


void LinearRegression::initWeight()
{
	std::default_random_engine de((unsigned int)time(0));
	std::normal_distribution<double> normalDistribution(0, 1 / double(weights.size()));

	for (size_t i = 0; i < weights.size(); i++)
	{
		weights[i] = normalDistribution(de);
	}
	biasWeight = normalDistribution(de);
}


double LinearRegression::activation(vector<double>& inputs)
{
	double result = 0;
	for (size_t i = 0; i < inputs.size(); i++)
	{
		result += weights[i] * inputs[i];
	}
	result += biasWeight;
	
	return result;
}


void LinearRegression::train(vector<vector<double>>& inputs, vector<double>& expectOutput)
{
	vector<double> gradientAccumulated(weights.size());
	double biasGradientAccumulated = 0;
	for (size_t rowIndex = 0; rowIndex < inputs.size(); rowIndex++)
	{
		double modelResult = activation(inputs[rowIndex]);
		double error = modelResult - expectOutput[rowIndex];
		for (size_t colIndex = 0; colIndex < inputs[rowIndex].size(); colIndex++)
		{
			gradientAccumulated[colIndex] += inputs[rowIndex][colIndex] * error;
		}
		biasGradientAccumulated += error;
	}

	for (size_t i = 0; i < weights.size(); i++)
	{
		printf("gradient, index: %d, gradient: %f \n", i, gradientAccumulated[i]);
		weights[i] -= (initLearningRate / sqrt(accumulatedGradient[i] + 0.01)) * gradientAccumulated[i];
		accumulatedGradient[i] += gradientAccumulated[i] * gradientAccumulated[i];
	}
	biasWeight -= (initLearningRate / sqrt(biasAccumulatedGradient + 0.01)) * biasGradientAccumulated;
	biasAccumulatedGradient += biasGradientAccumulated * biasGradientAccumulated;

	
}



void LinearRegression::numericGradientCalculate(vector<vector<double>>& inputs, vector<double>& expectOutput)
{
	for (size_t weightIndex = 0; weightIndex < weights.size(); weightIndex++)
	{
		double tmpGradient = 0;
		for (size_t rowIndex = 0; rowIndex < inputs.size(); rowIndex++)
		{
			double result = 0;
			for (size_t colIndex = 0; colIndex < inputs[rowIndex].size(); colIndex++)
			{
				if (colIndex == weightIndex)
				{
					result += (weights[colIndex] + 0.0001) * inputs[rowIndex][colIndex];
				}
				else
				{
					result += weights[colIndex] * inputs[rowIndex][colIndex];
				}
			}
			result += biasWeight;
			tmpGradient += 0.5 * (result - expectOutput[rowIndex]) * (result - expectOutput[rowIndex]);
		}

		double tmpGradientDesc = 0;
		for (size_t rowIndex = 0; rowIndex < inputs.size(); rowIndex++)
		{
			double result = 0;
			for (size_t colIndex = 0; colIndex < inputs[rowIndex].size(); colIndex++)
			{
				if (colIndex == weightIndex)
				{
					result += (weights[colIndex] - 0.0001) * inputs[rowIndex][colIndex];
				}
				else
				{
					result += weights[colIndex] * inputs[rowIndex][colIndex];
				}
			}
			result += biasWeight;
			tmpGradientDesc += 0.5 * (result - expectOutput[rowIndex]) * (result - expectOutput[rowIndex]);
		}

		printf("numeric gradient: index: %d, gradient: %f \n", weightIndex, (tmpGradient - tmpGradientDesc) / 0.0002);
	}
}


void LinearRegression::serialize()
{
	string filePath;
	filePath = "./bwapi-data/write/LR_model";
	fstream LRModel;

	LRModel << std::fixed << std::setprecision(30);
	LRModel.open(filePath.c_str(), ios::out);

	for (size_t i = 0; i < weights.size(); i++)
	{
		if (i != weights.size() - 1)
			LRModel << weights[i] << "&";
		else
			LRModel << weights[i];
	}
	LRModel << "|";

	for (size_t i = 0; i < accumulatedGradient.size(); i++)
	{
		if (i != accumulatedGradient.size() - 1)
			LRModel << accumulatedGradient[i] << "&";
		else
			LRModel << accumulatedGradient[i];
	}
	LRModel << "|";
	LRModel << biasWeight << "|" << biasAccumulatedGradient << "|" << initLearningRate;

	LRModel.close();
}


void LinearRegression::deserialize()
{
	string filePath;
	filePath = "./bwapi-data/write/LR_model";

	fstream LRModel;
	LRModel.open(filePath.c_str(), ios::in);

	if (LRModel.is_open())
	{
		std::vector<string> itemList;
		string item;
		while (getline(LRModel, item, '|'))
		{
			itemList.push_back(item);
		}

		std::stringstream ss2(itemList[0]);
		while (getline(ss2, item, '&'))
		{
			weights.push_back(std::stod(item));
		}

		std::stringstream ss3(itemList[1]);
		while (getline(ss3, item, '&'))
		{
			accumulatedGradient.push_back(std::stod(item));
		}

		biasWeight = std::stod(itemList[2]);
		biasAccumulatedGradient = std::stod(itemList[3]);
		initLearningRate = std::stod(itemList[4]);

	}
}


