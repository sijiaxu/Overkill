#include <iostream>
#include "network.h"
#include <time.h>
#include "LinearRegression.h"
#include "SumTree.h"


double getActionProb(Network& m, vector<double>& input, int actualChosen, vector<int>& outputChosen)
{
	vector<double> results = m.feedForward(input);

	double softmaxTotal = 0;
	for (auto o : outputChosen)
	{
		softmaxTotal += results[o];
	}
	for (auto o : outputChosen)
	{
		results[o] = results[o] / softmaxTotal;
	}
	return results[actualChosen];
}




void experienceDataInit(SumTree& replayData, int initCount)
{
	string filePath;
	filePath =  "./bwapi-data/write/RL_data";

	fstream historyModel;

	historyModel.open(filePath.c_str(), ios::in);

	if (historyModel.is_open())
	{
		string content;
		int count = 0;
		while (getline(historyModel, content))
		{
			if (content == "")
				continue;
			std::stringstream instance(content);
			string entry;
			std::vector<string> itemList;
			while (getline(instance, entry, '|'))
			{
				itemList.push_back(entry);
			}
			double priority = std::stod(itemList.back());
			itemList.pop_back();

			replayData.add(priority, itemList);
			count += 1;
			if (count == initCount)
			{
				break;
			}
		}
	}
	
	historyModel.close();
}

struct sampleParsedInfo
{
	vector<double> modelInputs;
	int actualChosen;
	double targetQValue;
};


sampleParsedInfo sampleParse(std::vector<std::string>& sample, Network QValueModel, Network targetQValueModel)
{
	sampleParsedInfo result;

	std::stringstream ss;
	ss.str("");
	ss.clear();
	ss << sample[0];
	string item;
	while (getline(ss, item, ':'))
	{
		if (item == "")
			continue;
		result.modelInputs.push_back(std::stod(item));
	}
	result.actualChosen = std::stoi(sample[1]);

	double targetQValue = 0;
	//terminal state-- next state value is empty
	if (sample[3] == "")
	{
		targetQValue = std::stod(sample[2]);
	}
	else
	{
		ss.str("");
		ss.clear();
		ss << sample[3];
		std::vector<double> nextItemList;
		while (getline(ss, item, ':'))
		{
			if (item == "")
				continue;
			nextItemList.push_back(std::stod(item));
		}
		//double Q learning
		vector<double> allActionValues = QValueModel.feedForward(nextItemList);
		vector<double> targetAllActionValues = targetQValueModel.feedForward(nextItemList);

		ss.str("");
		ss.clear();
		ss << sample[5];
		std::set<int> outList;
		while (getline(ss, item, ':'))
		{
			if (item == "")
				continue;
			outList.insert(std::stoi(item));
		}
		double maxValue = -999999;
		int	maxActionIndex = 0;
		for (size_t i = 0; i < allActionValues.size(); i++)
		{
			if (outList.find(i) != outList.end() && allActionValues[i] > maxValue)
			{
				maxValue = allActionValues[i];
				maxActionIndex = i;
			}
		}

		double targetMaxValue = targetAllActionValues[maxActionIndex];
		targetQValue = std::stod(sample[2]) + std::pow(0.99, 1) * targetMaxValue;
	}
	result.targetQValue = targetQValue;

	return result;
}



double getSamplePriority(std::vector<std::string>& sample, Network QValueModel, Network target)
{
	sampleParsedInfo result = sampleParse(sample, QValueModel, target);
	vector<double> allActionValues = QValueModel.feedForward(result.modelInputs);
	double TDError = allActionValues[result.actualChosen] - result.targetQValue;
	return  std::pow(std::abs(TDError) + 0.01, 0.6);
}


int main()
{

	/*
	for (int i = 0; i < 50; i++)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dis(0, 10);
		int randomPriority = dis(gen);

		randomPriority = rand() % 11;
		printf("%d \n", randomPriority);
	}




	SumTree s(10);
	vector<string> tmp = { "1", "2", "3" };
	for (int i = 0; i < 30; i++)
	{
		s.add(i, tmp);
	}
	map<int, vector<string>> t = s.sample(3);
	t = s.sample(4);
	for (auto a : t)
	{
		s.updatePriority(a.first, 20);
	}
	
	int a = 1;
	*/

	/*

	Network n;
	n.setDepth(3);
	n.setNodeCountAtLevel(50, 0, Relu);
	n.setNodeCountAtLevel(30, 1, Relu);
	n.setNodeCountAtLevel(2, 2, SUM);
	n.setNumInputs(3);
	n.linkNeurons();
	n.setLearningRate(0.001);
	n.initNetwork();


	vector<vector<double>> test;
	for (int i = 0; i < 10; i++)
	{
		vector<double> testInput(3);
		testInput[0] = std::rand() % 100;
		testInput[1] = std::rand() % 100;
		testInput[2] = std::rand() % 100;
		test.push_back(testInput);
	}

	vector<int> actualChosenOutput(10);
	for (int i = 0; i < 10; i++)
	{
		if (i < 5)
		{
			actualChosenOutput[i] = 1;
		}
		else
		{
			actualChosenOutput[i] = 1;
		}
	}

	vector<vector<int>> outputChosen;

	vector<double> qvalue;

	vector<double> expect(10);
	for (int i = 0; i < 10; i++)
	{
		expect[i] = std::rand() % 100;
	}


	//n.numericGradientCalculate(test, actualChosenOutput, outputChosen, true, expect);

	double totalLoss = 0;
	for (int round = 0; round < 10000; round++)
	{
		double loss = n.train(test, MSE, expect, actualChosenOutput);
		totalLoss += loss;
		if (round % 10 == 0)
		{
			printf("round %d: %f \n", round, totalLoss);
			totalLoss = 0;
		}
	}
	

	int b = 1;

	*/

	




	
	Network QValueModel;
	//QValueModel.deserialize("QValue");
		
	QValueModel.setDepth(2);
	QValueModel.setNodeCountAtLevel(100, 0, Relu);
	//QValueModel.setNodeCountAtLevel(80, 1, Relu);
	//QValueModel.setNodeCountAtLevel(50, 2, Relu);
	QValueModel.setNodeCountAtLevel(28, 1, SUM);
	QValueModel.setNumInputs(122);
	QValueModel.linkNeurons();
	QValueModel.setLearningRate(0.00025);
	QValueModel.initNetwork();



	Network targetValueModel;
	//targetValueModel.deserialize("targetQValueModel");
	targetValueModel.copy(QValueModel);

	SumTree	replayData;
	
	replayData.setCapacity(500);
	experienceDataInit(replayData, 500);
	vector<double> loss;
	for (int round = 0; round <= 10000; round++)
	{
		if (round % 1000 == 0 && round != 0)
		{
			int initCount = 1000 * ((round / 1000) + 1);
			replayData.setCapacity(initCount > 5000 ? 5000 : initCount);
			experienceDataInit(replayData, initCount);
		}

		map<int, vector<string>> sampleDatas = replayData.sample(8);
		vector<vector<double>> modelInputs;
		vector<int> actualChosen;
		vector<double> targetValues;

		
		for (auto instance : sampleDatas)
		{
			double priority = getSamplePriority(instance.second, QValueModel, targetValueModel);
			replayData.updatePriority(instance.first, priority);
			//importance sampling weight

			sampleParsedInfo parseData = sampleParse(instance.second, QValueModel, targetValueModel);
			modelInputs.push_back(parseData.modelInputs);
			actualChosen.push_back(parseData.actualChosen);
			targetValues.push_back(parseData.targetQValue);
		}

		//vector<double> result = QValueModel.feedForward(modelInputs[0]);
		//printf("round: %d, before train, loss: %f \n", round, std::pow(result[actualChosen[0]] - targetValues[0], 1));

		double curError = QValueModel.train(modelInputs, MSE, targetValues, actualChosen);
		//result = QValueModel.feedForward(modelInputs[0]);
		//printf("round: %d, after train, loss: %f \n", round, std::pow(result[actualChosen[0]] - targetValues[0], 1));
		
		if (round % 1000 == 0)
		{
			targetValueModel.copy(QValueModel);
		}
		

		loss.push_back(curError);
		if (round % 10 == 0)
		{
			double total = 0;
			for (auto e : loss)
			{
				total += e;
			}
			printf("round: %d, loss: %f \n", round, total / 10);
			loss.clear();
		}

	}
	

	int a = 1;


	/*


	std::vector<std::vector<std::string>> experienceData;
	experienceDataInit(experienceData);

	vector<vector<gameData>> parsedDataWin;
	vector<vector<gameData>> parsedDataLose;
	vector<vector<gameData>> parsedDataTotal;

	std::stringstream ss;
	for (int matchIndex = 0; matchIndex < experienceData.size(); matchIndex++)
	{
		vector<gameData> matchData;
		for (int index = 0; index < experienceData[matchIndex].size(); index++)
		{
			gameData newData;
			ss.str("");
			ss.clear();
			ss << experienceData[matchIndex][index][0];
			std::vector<double> itemList;
			string item;
			while (getline(ss, item, ':'))
			{
				if (item == "")
					continue;
				itemList.push_back(std::stod(item));
			}
			newData.modelInputs = itemList;
			ss.str("");
			ss.clear();
			ss << experienceData[matchIndex][index][4];
			std::vector<int> outList;
			while (getline(ss, item, ':'))
			{
				if (item == "")
					continue;
				outList.push_back(std::stoi(item));
			}
			newData.outputChosen = outList;
			newData.expectValue = std::stod(experienceData[matchIndex][index][1]);
			newData.actualChosen = std::stoi(experienceData[matchIndex][index][3]);
			newData.qValues = std::stod(experienceData[matchIndex][index][2]);
			newData.indexValue = std::stoi(experienceData[matchIndex][index][5]);

			matchData.push_back(newData);
		}
		if (matchData[0].qValues < 0)
		{
			parsedDataLose.push_back(matchData);
		}
		else
		{
			parsedDataWin.push_back(matchData);
		}

		parsedDataTotal.push_back(matchData);
	}

	vector<gameData> parsedTestData;
	for (int i = 0; i < 5; i++)
	{
		parsedTestData.push_back(parsedDataWin[0][i]);
	}

	std::srand((unsigned int)std::time(0));
	double miniSizePercent = 0.5;
	int GDRound = 3000;

	for (int i = 0; i < GDRound; i++)
	{
		vector<vector<double>> modelInputs;
		vector<double> expectValue;
		vector<int> actualChosen;
		vector<vector<int>> outputChosen;
		vector<double> qValues;
		vector<int> indexValue;
		std::stringstream ss;

		for (int matchCount = 0; matchCount < 10; matchCount++)
		{
			int matchIndex = 0;
			vector<gameData> parsedData;
			
			if (GDRound % 2 == 0)
			{
				matchIndex = std::rand() % parsedDataLose.size();
				parsedData = parsedDataLose[matchIndex];
			}
			else
			{
				matchIndex = std::rand() % parsedDataWin.size();
				parsedData = parsedDataWin[matchIndex];
			}

			//parsedData = parsedDataTotal[std::rand() % parsedDataTotal.size()];
			

			int miniBatch = 3;
			

			for (int j = 0; j < miniBatch; j++)
			{
				int index = std::rand() % parsedData.size();

				modelInputs.push_back(parsedData[index].modelInputs);
				outputChosen.push_back(parsedData[index].outputChosen);
				expectValue.push_back(parsedData[index].expectValue);
				actualChosen.push_back(parsedData[index].actualChosen);
				//reinforce with baseline
				qValues.push_back(parsedData[index].qValues - parsedData[index].expectValue);
				//qValues.push_back(parsedData[matchIndex][index].qValues);
				indexValue.push_back(parsedData[index].indexValue);
			}
		}

		
		policyModel.train(modelInputs, POLICY_GRADIENT, vector<double>(), actualChosen, outputChosen, qValues, indexValue);

		printf("match:%d \n", i);
		for (auto instance : parsedTestData)
		{
			double p = getActionProb(policyModel, instance.modelInputs, 11, instance.outputChosen);
			printf("%f \n", p);
		}
		printf("\n");
	}

	

	int a = 1;

	*/
	
	 
	/*
	Network policyModel;
	policyModel.deserialize("policy");

	std::vector<std::vector<std::string>> experienceData;
	string filePath = "./bwapi-data/write/RL_data";
	fstream historyModel;

	historyModel.open(filePath.c_str(), ios::in);
	if (historyModel.is_open())
	{
		string content;
		//store the (St-1, action, r, St) info
		//scheme Q(St-1) | action | Q(st) | reward
		while (getline(historyModel, content))
		{
			if (content == "")
				continue;
			std::stringstream ss(content);
			std::vector<string> itemList;
			string item;
			while (getline(ss, item, '|'))
			{
				itemList.push_back(item);
			}
			experienceData.push_back(itemList);
		}
	}

	vector<vector<double>> modelInputs;
	vector<double> expectValue;
	vector<int> actualChosen;
	vector<vector<int>> outputChosen;
	vector<double> qValues;
	std::stringstream ss;

	for (size_t i = 0; i < 2; i++)
	{
		int randomIndex = i;

		ss.str("");
		ss.clear();
		ss << experienceData[randomIndex][0];
		std::vector<double> itemList;
		string item;
		while (getline(ss, item, ':'))
		{
			if (item == "")
				continue;
			itemList.push_back(std::stod(item));
		}
		modelInputs.push_back(itemList);
		ss.str("");
		ss.clear();
		ss << experienceData[randomIndex][4];
		std::vector<int> outList;
		while (getline(ss, item, ':'))
		{
			if (item == "")
				continue;
			outList.push_back(std::stoi(item));
		}
		outputChosen.push_back(outList);
		expectValue.push_back(std::stod(experienceData[randomIndex][2]));
		actualChosen.push_back(std::stoi(experienceData[randomIndex][3]));
		//reinforce with baseline
		qValues.push_back(std::stod(experienceData[randomIndex][2]) - std::stod(experienceData[randomIndex][1]));
	}

	for (size_t i = 0; i < modelInputs.size(); i++)
	{
		printf("instance %d : \n", i);
		for (auto j : outputChosen[i])
		{
			printf("action %d, score: %f\n", j, getActionProb(policyModel, modelInputs[i], j, outputChosen[i]));
		}
	}

	for (int t = 0; t < 10; t++)
	{
		policyModel.train(modelInputs, false, vector<double>(), actualChosen, outputChosen, qValues);
		printf("after %d\n", t);
		for (size_t i = 0; i < modelInputs.size(); i++)
		{
			printf("instance %d : \n", i);
			for (auto j : outputChosen[i])
			{
				printf("action %d, score: %f\n", j, getActionProb(policyModel, modelInputs[i], j, outputChosen[i]));
			}
		}
	}
	

	int a = 1;
	*/

/*
	
	srand((unsigned int)time(NULL));

	Network n;
	n.setDepth(3);
	n.setNodeCountAtLevel(3, 0, Relu);
	n.setNodeCountAtLevel(3, 1, Relu);
	n.setNodeCountAtLevel(3, 2, SOFTMAX);
	n.setNumInputs(3);
	n.linkNeurons();
	n.setLearningRate(0.2);

	vector<vector<double>> test;
	vector<double> testInput(3);
	testInput[0] = 2;
	testInput[1] = 0;
	testInput[2] = 4;

	test.push_back(testInput);

	vector<double> testInput2(3);
	testInput2[0] = 2;
	testInput2[1] = 2;
	testInput2[2] = 2;

	test.push_back(testInput2);

	vector<int> actualChosenOutput(2);
	actualChosenOutput[0] = 1;
	actualChosenOutput[1] = 1;

	vector<vector<int>> outputChosen =
	{ {0, 1, 2},
	  {0, 1, 2 } };

	vector<double> qvalue(2);
	qvalue[0] = 1;
	qvalue[1] = 1;

	vector<double> expect(2);
	expect[0] = 3.32;
	expect[1] = 4.11;


	n.initNetwork(); //Randomize Weights
	
	n.numericGradientCalculate(test, actualChosenOutput, outputChosen, false, expect);
	n.train(test, POLICY_GRADIENT, expect, actualChosenOutput, outputChosen, qvalue);

	int b = 1;
	

	
	n.serialize("test");


	Network m;
	m.deserialize("test");

	n.numericGradientCalculate(test, actualChosenOutput, outputChosen, false, expect);
	n.train(test, POLICY_GRADIENT, expect, actualChosenOutput, outputChosen, qvalue);

 	int a = 1;
	
	*/
}
