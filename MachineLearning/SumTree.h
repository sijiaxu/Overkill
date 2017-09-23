#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <cstdlib>
#include <math.h>
#include <string>
#include <random>


using namespace std;


struct treeReturnData
{
	vector<string> data;
	double priority;
};


class SumTree
{
public:
	SumTree() {}
	SumTree(int capacity);
	void setCapacity(int capacity);
	void add(double priority, vector<string> data);
	map<int, vector<string>> sample(int miniBatchSize);
	void updatePriority(int treeIndex, double priority);
	vector<treeReturnData> getData();

private:
	void propagateChange(int treeIndex, double change);
	int retrieve(int treeIndex, double priority);

	int dataCapacity;
	int dataWriteIndex;
	//tree has capacity -1 non-leaf node and capacity leaf node
	vector<double> prioritySumTree;
	vector<vector<string>> rawData;
};

