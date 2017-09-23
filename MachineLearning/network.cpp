#include "network.h"

//Initialize network with one layer and 0 input/outputs
Network::Network() 
{
	setLearningRate(0.2);
}

void Network::copy(Network n)
{
	network = n.network;
}


void Network::targetNetworkUpdate(Network& n)
{
	for (size_t layer = 0; layer < n.network.size(); layer++)
	{
		for (size_t neuron = 0; neuron < n.network[layer].size(); neuron++)
		{
			for (size_t weightIndex = 0; weightIndex < n.network[layer][neuron].weights.size(); weightIndex++)
			{
				network[layer][neuron].weights[weightIndex] = 0.001 * n.network[layer][neuron].weights[weightIndex] + 0.999 * network[layer][neuron].weights[weightIndex];
			}
			network[layer][neuron].biasWeight = 0.001 * n.network[layer][neuron].biasWeight + 0.999 * network[layer][neuron].biasWeight;
		}
	}
}


//Set the networks learning rate
void Network::setLearningRate(double rate) 
{
	for (size_t i = 0; i < network.size(); i++) 
	{
		for (size_t j = 0; j < network[i].size(); j++) 
		{
			network[i][j].setLearningRate(rate);
		}
	}
}


//input layer is not shown in the network
void Network::setNumInputs(int ninp) 
{
	for (size_t i = 0; i < network[0].size(); i++) 
	{
		network[0][i].setNumInputs(ninp);
	}
}

//Set the depth of the network
void Network::setDepth(int depth) 
{
	network.resize(depth);
}

//Set the height of the network at a given layer
void Network::setNodeCountAtLevel(int count, int level, activeFunctionType af)
{
	network[level].resize(count);
	for (auto& n : network[level])
	{
		n.setActiveFunction(af);
	}
}

//This function sets the number of inputs in each layer to the number of neuron in the previous layer
void Network::linkNeurons() 
{
	for (size_t i = 1; i < network.size(); i++) 
	{
		for (size_t j = 0; j < network[i].size(); j++) 
		{
			network[i][j].setNumInputs(network[i-1].size());
		}
	}
}

//Prints each neuron in each layer and the weights of each
void Network::printNetwork() 
{
	for (size_t i = 0; i < network.size(); i++) 
	{
		cout << "Layer " << i << "\n";
		for (size_t j = 0; j < network[i].size(); j++) 
		{
			network[i][j].print();
		}
	}
}

//Randomly sets the weights for each connection
void Network::initNetwork()
{
	for (size_t i = 0; i < network.size(); i++) 
	{
		for (size_t j = 0; j < network[i].size(); j++) 
		{
			network[i][j].resetWeights(i, j);
		}
	}
}

//Applies the inputs to the network and returns the output
vector<double> Network::feedForward(vector<double> inputs)
{
	vector<int> noneZeroIndex;
	for (size_t i = 0; i < inputs.size(); i++)
	{
		if (inputs[i] != 0.0)
		{
			noneZeroIndex.push_back(i);
		}
	}

	for (size_t i = 0; i < network.size(); i++) 
	{
		vector<double> lInp(network[i].size());
		for (size_t j = 0; j < network[i].size(); j++)
		{
			//input is sparse
			if (i == 0)
			{
				lInp[j] = network[i][j].activation(inputs, noneZeroIndex);
			}
			else
			{
				lInp[j] = network[i][j].activation(inputs);
			}
		}
		inputs = lInp;
	}
	return inputs;
}


void Network::numericGradientCalculate(vector<vector<double>> multiInputs, vector<int> actualChosen, 
	vector<vector<int>> outputChosen, bool isMSE, vector<double> expectValue)
{
	for (size_t layer = 0; layer < network.size(); layer++)
	{
		for (size_t neuronIndex = 0; neuronIndex < network[layer].size(); neuronIndex++)
		{
			for (int weightIndex = 0; weightIndex < network[layer][neuronIndex].getInputNum(); weightIndex++)
			{



				double difference = 0.0f;
				double addLossFunctionValue = 0;
				double subtrackLoss = 0;
				double regulation_term = 0;
				double regulation_term_sub = 0;
				for (size_t inputIndex = 0; inputIndex < multiInputs.size(); inputIndex++)
				{
					vector<double> testInput = multiInputs[inputIndex];
					int actualChosenIndex = actualChosen[inputIndex];
					regulation_term_sub = 0;
					regulation_term = 0;
					for (size_t i = 0; i < network.size(); i++)
					{
						vector<double> lInp(network[i].size());
						for (size_t j = 0; j < network[i].size(); j++)
						{
							vector<double>& allWeights = network[i][j].getAllWeight();
							for (size_t w_r = 0; w_r < allWeights.size(); w_r++)
							{
								if (w_r == weightIndex && i == layer && j == neuronIndex)
									regulation_term += 0.05 * pow((allWeights[w_r] + 0.0001), 2);
								else
									regulation_term += 0.05 * pow(allWeights[w_r], 2);
							}

							if (i == layer && j == neuronIndex)
							{
								lInp[j] = network[i][j].activationChange(testInput, weightIndex, true);
								
							}
							else
							{
								lInp[j] = network[i][j].activation(testInput);

							}
						}
						testInput = lInp;
					}
					
					if (isMSE)
					{
						addLossFunctionValue += 0.5 * std::pow((testInput[actualChosenIndex] - expectValue[inputIndex]), 2) / multiInputs.size();
					}
					else
					{
						double tmpAdd = 0;
						for (auto o : outputChosen[inputIndex])
						{
							tmpAdd += testInput[o];
						}

						addLossFunctionValue = testInput[actualChosenIndex] / tmpAdd;
						addLossFunctionValue = addLossFunctionValue - 0.1 * regulation_term;
					}
					
					
					testInput = multiInputs[inputIndex];
					for (size_t i = 0; i < network.size(); i++)
					{
						vector<double> lInp(network[i].size());
						for (size_t j = 0; j < network[i].size(); j++)
						{
							vector<double>& allWeights = network[i][j].getAllWeight();
							for (size_t w_r = 0; w_r < allWeights.size(); w_r++)
							{
								if (w_r == weightIndex && i == layer && j == neuronIndex)
									regulation_term_sub += 0.05 * pow((allWeights[w_r] - 0.0001), 2);
								else
									regulation_term_sub += 0.05 * pow(allWeights[w_r], 2);
							}

							if (i == layer && j == neuronIndex)
							{
								lInp[j] = network[i][j].activationChange(testInput, weightIndex, false);
							}
							else
							{
								lInp[j] = network[i][j].activation(testInput);
							}
						}
						testInput = lInp;
					}
					
					if (isMSE)
					{
						subtrackLoss += 0.5 * std::pow((testInput[actualChosenIndex] - expectValue[inputIndex]), 2) / multiInputs.size();
					}
					else
					{
						double tmpAdd = 0;
						for (auto o : outputChosen[inputIndex])
						{
							tmpAdd += testInput[o];
						}
						subtrackLoss = testInput[actualChosenIndex] / tmpAdd;
						subtrackLoss = subtrackLoss - 0.1 * regulation_term;
					}

				}
				difference = (addLossFunctionValue + regulation_term - subtrackLoss - regulation_term_sub) / 0.0002;
				printf("numeric check: layer: %d, neuron: %d, weightIndex: %d gradient: %f \n",
					layer, neuronIndex, weightIndex, difference);

			}
		}
	}
}


//Train using back propagation
double Network::train(vector<vector<double>>& multiInputs, lossFunctionType loss,
	const vector<double>& expectValue, const vector<int>& actualChosen, const vector<vector<int>>& outputChosen, const vector<double>& qValues,
	const vector<int>& indexValue, double totalMatchCount)
{
	double totalError = 0.0;
	for (size_t instance = 0; instance < multiInputs.size(); instance++)
	{
		//Feed forward for results
		vector<double> results = feedForward(multiInputs[instance]);
		double multiplyValue = 1.0f;
		
		if (loss == LOGISTIC)
		{
			network[network.size() - 1][0].setErrorSignal((results[0] - expectValue[instance]));
			totalError += results[0] - expectValue[instance];
		}
		else if (loss == POLICY_GRADIENT)
		{
			//get the real softmax output
			//only calculate the outputChosen action's probability

			/*
			double softmaxTotal = 0;
			for (auto o : results)
			{
				softmaxTotal += o;
			}
			for (auto& o : results)
			{
				o = o / softmaxTotal;
			}

			int actualChosenIndex = actualChosen[i];
			multiplyValue = qValues[i] * (1 / results[actualChosenIndex]) * std::pow(0.99, indexValue[i]);

			//set<int> tmp(outputChosen[i].begin(), outputChosen[i].end());
			//Calculate output error.
			for (size_t i = 0; i < results.size(); i++)
			{
				if (i == actualChosenIndex)
				{
					network[network.size() - 1][i].setErrorSignal(results[i] * (1 - results[i]) * multiplyValue);
				}
				else
				{
					network[network.size() - 1][i].setErrorSignal(-1 * results[actualChosenIndex] * results[i] * multiplyValue);
				}
			}
			*/
			
			
			double softmaxTotal = 0;
			for (auto o : outputChosen[instance])
			{
				softmaxTotal += results[o];
			}
			for (auto o : outputChosen[instance])
			{
				results[o] = results[o] / softmaxTotal;
			}

			int actualChosenIndex = actualChosen[instance];
			multiplyValue = qValues[instance] * (1 / results[actualChosenIndex]) * std::pow(0.99, indexValue[instance]) * (1 / totalMatchCount);

			set<int> tmp(outputChosen[instance].begin(), outputChosen[instance].end());
			//Calculate output error.
			for (size_t i = 0; i < results.size(); i++)
			{
				if (i == actualChosenIndex)
				{
					network[network.size() - 1][i].setErrorSignal(results[i] * (1 - results[i]) * multiplyValue);
					totalError += results[i] * (1 - results[i]) * multiplyValue;
				}
				else if (tmp.find(i) != tmp.end())
				{
					network[network.size() - 1][i].setErrorSignal(-1 * results[actualChosenIndex] * results[i] * multiplyValue);
				}
				else
				{
					network[network.size() - 1][i].setErrorSignal(0);
				}
			}
			
		}
		//MSE loss
		else
		{
			int actualChosenIndex = actualChosen[instance];
			double gradient = 0;
			for (size_t resultIndex = 0; resultIndex < results.size(); resultIndex++)
			{
				if (resultIndex == actualChosenIndex)
				{
					//clipping the error into the range [-1, 1]
					double tmp = (results[actualChosenIndex] - expectValue[instance]) / multiInputs.size();
					gradient = tmp; //> 1 ? 1 : (tmp < -1 ? -1 : tmp);
					totalError += std::pow(results[actualChosenIndex] - expectValue[instance], 2);

					network[network.size() - 1][resultIndex].setErrorSignal(gradient);
				}
				else
				{
					network[network.size() - 1][resultIndex].setErrorSignal(0);
				}
			}

			
		}

		//back propagate
		for (int i = (int)network.size() - 2; i >= 0; i--)
		{
			for (size_t hid = 0; hid < network[i].size(); hid++)
			{
				double backPropVal = 0;
				//Summation of: (W_i * Err_i)
				for (size_t bpvI = 0; bpvI < network[i + 1].size(); bpvI++)
				{
					backPropVal += (network[i + 1][bpvI].getWeight(hid) * network[i + 1][bpvI].getErrorSignal());
				}

				//Error = O * (1 - O) * sum(W_i * Err_i) * optional_value
				if (loss == POLICY_GRADIENT)
					network[i][hid].setErrorSignal(network[i][hid].getBpDerivative() * backPropVal * multiplyValue);
				else
					network[i][hid].setErrorSignal(network[i][hid].getBpDerivative() * backPropVal);
			}
		}
	}

	//update weight
 	for (size_t i = 0; i < network.size(); i++)
	{
		for (size_t hid = 0; hid < network[i].size(); hid++)
		{
			//network[i][hid].debugPrintGradient(i, hid);

			if (loss == POLICY_GRADIENT)
				network[i][hid].adjustForError(false);
			else
				network[i][hid].adjustForError(true);

		}
	}

	return totalError;
}


void Network::serialize(string modelName, string pathName)
{
	string filePath;
	filePath = pathName + "NN_model_" + modelName;
	fstream NNModel;

	NNModel << std::fixed << std::setprecision(30);
	NNModel.open(filePath.c_str(), ios::out);

	for (auto row : network)
	{
		for (auto n : row)
		{
			string neuronString = n.serialize();
			NNModel << neuronString << "$";
		}
		NNModel << "#";
	}
	NNModel.close();
}


int Network::deserialize(string modelName, string pathName)
{
	string filePath;
	filePath = pathName + "NN_model_" + modelName;

	fstream NNModel;
	NNModel.open(filePath.c_str(), ios::in);

	if (NNModel.is_open())
	{
		string content;
		while (getline(NNModel, content, '#'))
		{
			if (content == "")
				continue;
			
			std::stringstream ss(content);
			std::vector<Neuron> itemList;
			string item;
			while (getline(ss, item, '$'))
			{
				if (item == "")
					continue;
				Neuron n;
				n.deserialize(item);
				itemList.push_back(n);
			}
			network.push_back(itemList);
		}
		NNModel.close();
		return 0;
	}
	else
	{
		return -1;
	}
}

