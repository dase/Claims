/*
 * Requirement.h
 *
 *  Created on: Jan 14, 2014
 *      Author: wangli
 */

#ifndef REQUIREMENT_H_
#define REQUIREMENT_H_
#include <vector>

#include "../ids.h"
#include "../hash.h"
#include "../Catalog/Attribute.h"
#include "../Scheduler/Dataflow.h"
/**
 * This class describes what properties are required to the child data flow.
 */
enum NetworkTransfer{NONE,Shuffle,piped};

class Requirement {
public:
	Requirement();

	void setRequiredPartitionkey(const Attribute partition_key);
	Attribute getPartitionKey()const;
	bool hasReuiredPartitionKey()const;

	void setRequiredPartitionFucntion(PartitionFunction* partition);
	PartitionFunction* getPartitionFunction()const;
	bool hasRequiredPartitionFunction()const;

	void setRequiredLocations(std::vector<NodeID> location_list);
	std::vector<NodeID> getRequiredLocations()const;
	bool hasRequiredLocations()const;

	void setRequiredCost(const unsigned long cost);
	NetworkTransfer requireNetworkTransfer(const Dataflow& dataflow)const;
	bool passLimits(const unsigned long cost)const;
	virtual ~Requirement();

private:
	/**
	 * describe the desirable location for each partition.
	 * empty list if there is no such requirement.
	 */
	std::vector<NodeID> location_list_;

	/**
	 * describe what partition function are required, including partition numbers and
	 * partition fashion(e.g., hash, range, etc.)
	 * null if there is no such requirement
	 */
	PartitionFunction* partition_function_;

	/**
	 * describe the desirable partition attribute.
	 * TODO: a set of desirable partition attributes.
	 */
	Attribute partition_key_;

	/**
	 * specify the acceptable cost for the child sub-plan.
	 * zero if no limit is specified.
	 */
	unsigned long cost_limit_;

};

#endif /* REQUIREMENT_H_ */
