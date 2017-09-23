#include "neuron.h"

Neuron::Neuron() 
{
	learningRate = 0.5;
	functionType = SIGMOD;
	visitCount = 0;
}


void Neuron::setErrorSignal(double e)
{
	visitCount += 1;
	error = e;
	for (size_t wt = 0; wt < weights.size(); wt++)
	{
		batchGradient[wt] += error * lastInp[wt];
	}
	batchBiasGradient += error;
}


double Neuron::activationChange(vector<double> inputs, int weightIndex, bool addEpsilon)
{
	lastInp = inputs;
	activationResult = 0;
	for (size_t i = 0; i < inputs.size(); i++)
	{
		if (i == weightIndex)
		{
			if (addEpsilon)
				activationResult += inputs[i] * (weights[i] + 0.0001);
			else
				activationResult += inputs[i] * (weights[i] - 0.0001);
		}
		else
		{
			activationResult += inputs[i] * weights[i];
		}
	}

	if (functionType == SIGMOD)
	{
		activationResult = sigmoid(activationResult + biasWeight);
	}
	else if (functionType == SOFTMAX)
	{
		activationResult = softmax(activationResult + biasWeight);
	}
	else if (functionType == SUM)
	{
		activationResult = activationResult + biasWeight;
	}
	else if (functionType == Relu)
	{
		activationResult = activationResult + biasWeight > 0 ? activationResult + biasWeight : 0;
	}
	else
	{
		activationResult = sigmoid(activationResult + biasWeight);
	}
	return activationResult;
}

//Evaluate the neuron with given inputs
double Neuron::activation(vector<double>& inputs, vector<int> noneZeroIndex)
{
	lastInp = inputs;
	activationResult = 0;

	if (noneZeroIndex.size() > 0)
	{
		for (auto entry : noneZeroIndex)
		{
			activationResult += inputs[entry] * weights[entry];
		}
	}
	else
	{
		for (size_t i = 0; i < inputs.size(); i++)
		{
			activationResult += inputs[i] * weights[i];
		}
	}


	if (functionType == SIGMOD)
	{
		activationResult = sigmoid(activationResult + biasWeight);
	}
	else if (functionType == SOFTMAX)
	{
		activationResult = softmax(activationResult + biasWeight);
	}
	else if (functionType == SUM)
	{
		activationResult = activationResult + biasWeight;
	}
	else if (functionType == Relu)
	{
		activationResult = activationResult + biasWeight > 0 ? activationResult + biasWeight : 0;
	}
	else
	{
		activationResult = sigmoid(activationResult + biasWeight);
	}
	return activationResult;
}


double Neuron::getBpDerivative()
{
	double derivative = 0;
	if (functionType == SIGMOD)
	{
		derivative = activationResult * (1 - activationResult);
	}
	else if (functionType == Relu)
	{
		derivative = activationResult > 0 ? 1 : 0;
	}
	else
	{
		printf("derivative error !!!! \n");
	}
	return derivative;
}


void Neuron::setLearningRate(double Nlearn) 
{
	learningRate = Nlearn;
}

void Neuron::setNumInputs(int numinputs) 
{
	weights.resize(numinputs);
	accumulatedGradient.resize(numinputs);
	for (auto& i : accumulatedGradient)
	{
		i = 0;
	}
	biasAccumulatedGradient = 0;
	batchGradient.resize(numinputs);
	batchBiasGradient = 0;
}


//using Xavier to init weight
void Neuron::resetWeights(int layer, int index)
{
	std::normal_distribution<double> normalDistribution(0, 1 / double(weights.size()));
	std::default_random_engine de((unsigned int)time(0) + layer * 10000 + index * 5000);
	for (size_t i = 0; i < weights.size(); i++) 
	{
		weights[i] = normalDistribution(de);
	}
	biasWeight = normalDistribution(de);
}


//Sigmoid Activation Function
double Neuron::sigmoid(double val) 
{
	return 1 / (1 + exp(-1 * val));
}

double Neuron::softmax(double val)
{
	return exp(val);
}


void Neuron::adjustForError(bool isDescent)
{
	//update the weights
	//using RMSprop adaptive learning rate
	for (size_t wt = 0; wt < weights.size(); wt++) 
	{
		//L2 regulation
		if (isDescent)
			batchGradient[wt] = batchGradient[wt] + 0.0025 * weights[wt] ;
		else
			batchGradient[wt] = batchGradient[wt] - 0.01 * weights[wt];

		if (accumulatedGradient[wt] == 0)
		{
			accumulatedGradient[wt] = batchGradient[wt] * batchGradient[wt];
		}
		else
		{
			accumulatedGradient[wt] = 0.9 * accumulatedGradient[wt] + 0.1 * batchGradient[wt] * batchGradient[wt];
		}

		if (isDescent)
			weights[wt] -= (learningRate / sqrt(accumulatedGradient[wt] + 0.0000001)) * batchGradient[wt];
		else
			weights[wt] += (learningRate / sqrt(accumulatedGradient[wt] + 0.0000001)) * batchGradient[wt];
	}

	if (biasAccumulatedGradient == 0)
	{
		biasAccumulatedGradient = batchBiasGradient * batchBiasGradient;
	}
	else
	{
		biasAccumulatedGradient = 0.9 * biasAccumulatedGradient + 0.1 * batchBiasGradient * batchBiasGradient;
	}
	if (isDescent)
		biasWeight -= (learningRate / sqrt(biasAccumulatedGradient + 0.0000001)) * batchBiasGradient;
	else
		biasWeight += (learningRate / sqrt(biasAccumulatedGradient + 0.0000001)) * batchBiasGradient;

	//reset batch value
	for (auto& v : batchGradient)
	{
		v = 0;
	}
	batchBiasGradient = 0;
	visitCount = 0;
}


void Neuron::debugPrintGradient(int layer, int neuronIndex)
{
	for (size_t wt = 0; wt < weights.size(); wt++)
	{
		printf("layer: %d neuron: %d weight index: %d gradient:%.10f \n", layer, neuronIndex, int(wt), batchGradient[wt] + 0.1 * weights[wt]);
	}
}

//Prints out all the neurons weights
void Neuron::print() 
{
	cout << "Neuron:\n";
	for (size_t i = 0; i < weights.size(); i++) 
	{
		cout << "Weight #" << i << ": " << weights[i] << "\n";
	}
}



string Neuron::serialize()
{
	stringstream serializeString;
	serializeString.precision(15);
	serializeString << std::fixed;
	serializeString << int(functionType) << "|";

	for (size_t i = 0; i < weights.size(); i++)
	{
		if (i != weights.size() - 1)
			serializeString << weights[i] << "&";
		else
			serializeString << weights[i];
	}
	serializeString << "|";

	for (size_t i = 0; i < accumulatedGradient.size(); i++)
	{
		if (i != accumulatedGradient.size() - 1)
			serializeString << accumulatedGradient[i] << "&";
		else
			serializeString << accumulatedGradient[i];
	}
	serializeString << "|";
	serializeString << biasWeight << "|" << biasAccumulatedGradient << "|" << learningRate;
	
	return serializeString.str();
}

void Neuron::deserialize(string input)
{
	std::stringstream ss(input);
	std::vector<string> itemList;
	string item;
	while (getline(ss, item, '|'))
	{
		itemList.push_back(item);
	}
	functionType = static_cast<activeFunctionType>(std::stoi(itemList[0]));

	ss.str("");
	ss.clear();
	ss << itemList[1];

	while (getline(ss, item, '&'))
	{
		weights.push_back(std::stod(item));
	}
	batchGradient.resize(weights.size());
	for (auto& r : batchGradient)
	{
		r = 0;
	}
	batchBiasGradient = 0;
	visitCount = 0;

	ss.str("");
	ss.clear();
	ss << itemList[2];

	while (getline(ss, item, '&'))
	{
		accumulatedGradient.push_back(std::stod(item));
	}

	biasWeight = std::stod(itemList[3]);
	biasAccumulatedGradient = std::stod(itemList[4]);
	learningRate = std::stod(itemList[5]);

}
