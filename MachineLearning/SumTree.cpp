#include "SumTree.h"


SumTree::SumTree(int capacity)
{
	dataCapacity = capacity;
	dataWriteIndex = 0;
	prioritySumTree.resize(capacity * 2, 0);
	rawData.resize(capacity);
}

void SumTree::setCapacity(int capacity)
{
	dataCapacity = capacity;
	dataWriteIndex = 0;
	prioritySumTree.resize(capacity * 2, 0);
	rawData.resize(capacity);
}


vector<treeReturnData> SumTree::getData()
{
	vector<treeReturnData> result;
	for (size_t i = 0; i < rawData.size(); i++)
	{
		if (rawData[i].size() != 0)
		{
			treeReturnData r;
			r.data = rawData[i];
			r.priority = prioritySumTree[i + dataCapacity - 1];
			result.push_back(r);
		}
	}
	return result;
}


void SumTree::add(double priority, vector<string> data)
{
	int treeWriteIndex = dataWriteIndex + dataCapacity - 1;
	rawData[dataWriteIndex] = data;
	dataWriteIndex++;
	if (dataWriteIndex >= dataCapacity)
	{
		dataWriteIndex = 0;
	}

	double priorityChange = priority - prioritySumTree[treeWriteIndex];
	prioritySumTree[treeWriteIndex] = priority;
	propagateChange(treeWriteIndex, priorityChange);
}


void SumTree::propagateChange(int treeIndex, double change)
{
	int parentIndex = (treeIndex - 1) / 2;
	prioritySumTree[parentIndex] += change;
	if (parentIndex != 0)
	{
		propagateChange(parentIndex, change);
	}
}


map<int, vector<string>> SumTree::sample(int miniBatchSize)
{
	map<int, vector<string>> sampleData;
	double segmentInterval = prioritySumTree[0] / miniBatchSize;
	std::random_device rd;  
	std::mt19937 gen(rd());

	for (int i = 0; i < miniBatchSize; i++)
	{
		double startPoint = i * segmentInterval;
		double endPoint = (i + 1) * segmentInterval;
		if (i == miniBatchSize - 1)
		{
			endPoint = prioritySumTree[0];
		}
		std::uniform_real_distribution<double> dis(startPoint, endPoint);
		double randomPriority = dis(gen);

		int treeLeafIndex = retrieve(0, randomPriority);
		int dataIndex = treeLeafIndex - dataCapacity + 1;
		sampleData[treeLeafIndex] = rawData[dataIndex];
	}

	return sampleData;
}


int SumTree::retrieve(int treeIndex, double priority)
{
	int left = 2 * treeIndex + 1;
	int right = 2 * treeIndex + 2;

	//leaf node is start from capacity - 1
	if (left >= prioritySumTree.size() - 1)
		return treeIndex;

	if (priority <= prioritySumTree[left])
	{
		return retrieve(left, priority);
	}
	else
	{
		return retrieve(right, priority - prioritySumTree[left]);
	}
}


void SumTree::updatePriority(int treeIndex, double priority)
{
	double priorityChange = priority - prioritySumTree[treeIndex];
	prioritySumTree[treeIndex] = priority;
	propagateChange(treeIndex, priorityChange);
}



