/*********************************************************************
** Filename: 	main.cpp
** Author: 		Group 8 - Niza Volair, Sara Hashem, Matt Palmer
** Date: 		08/06/16
** Description: Main function for Project 4: Traveling Salesperson 
** Input: 		From file
** Output: 		Test results from functions to standard output and file
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <sstream>
#include <deque>
#include <string>
#include <math.h>

// Set up struct for city info- name and axes 
struct City {
	int name;
	int x;
	int y;
	bool visited;
};
	
struct Node {
	int city;
	int lower_bound;
	struct Node *parent;
	struct std::deque<Node *> children;
	int **savedAdjacenyMatrix;
	std::deque<int> savedPath;
	std::deque<City> savedCities;
};
	
// Function Prototypes
int distance(struct City city1, struct City city2);
void tourGreedy(std::deque<City> Cities, std::ofstream &file);
void tourBranchBound(std::deque<City> Cities, std::ofstream &file);
void opt(std::deque<City>& Cities, int &d);
int reduceMatrix(int **adjacencyMatrix, int matrixLength);
int minDistance(int array[], int arrayLength);
struct Node *createNode(int newCity, struct Node *newParent);
int findSmallestLowerBound(struct Node *currentNode, 
	int **childAdjacencyMatrix, 
	int **adjacencyMatrix, 
	int matrixLength,
	std::deque<struct Node*> duplicates,
	std::deque<City> cities,
	std::deque<int> path);
void chooseDestination(int **childAdjacencyMatrix, 
	int destination, 
	int origin, 
	int matrixLength);
void deleteChildren(struct Node *currentNode, int matrixLength);
long unsigned int calculatePathLength(int **originalMatrix, std::deque<int> path);	
	



int main(int argc, char *argv[]){
    // Check number of arguments 
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "WELCOME TO GROUP 8 PROJECT 4" << std::endl; 
	std::cout << std::endl;
	std::cout << "Results for Small Files Displayed for" << std::endl;
	std::cout << "Greedy + Optimzation and Branch & Bound Algorithms" << std::endl;
	std::cout << std::endl;
	std::cout << "Results for Large Files Displayed for" << std::endl;
	std::cout << "Greedy + Optimzation Algorithm" << std::endl;
	std::cout << std::endl;
	std::cout << "Results for Greedy + Optimzation Algorithm" << std::endl;
	std::cout << "Written to Output File (InputFileName+tour)" << std::endl;
	std::cout << std::endl;
	
    if(argc < 2){
		std::cout << "Usage: prog [input_file]" << std::endl;
		exit(1);
    }

    // Create clock variable and double to track time
    std::clock_t start;
    double duration;

	// Initialize random number generator with time seed
	time_t seed;
	srand((unsigned) time(&seed));


	// Set up vector to contain city structs
	std::deque<City> Cities;
	
	
	// Set up file objects and parsers
	std::ifstream infile;
	std::ofstream outfile;
	std::string line, tempString, inFileName, outFileName, fileExtension;
	int tempInt;


	// Open in files for reading
	inFileName = argv[1];
	infile.open( inFileName.c_str() );
	
	// Check for file opening error
	if(!infile.is_open()){
		std::cout << "Error opening file" << std::endl;
		exit(1);
		
	}	
	
	// Get outfile extension
	unsigned int extensionIndex = inFileName.find_last_of('.');
	outFileName = inFileName.substr(0, extensionIndex) + "tour" + inFileName.substr(extensionIndex);
	
	// Open and check outfile
	outfile.open( outFileName.c_str() );
	if(!outfile.is_open()){
		std::cout << "Error opening file" << std::endl;
		exit(1);
		
	}		

	



	// Temp struct to parse file
	City tempCity;
	
	// Parse each line from file
	while(getline(infile, line)){
		std::stringstream linestream(line);
		
		//For each line extrack the name, x, and y axes
		linestream >> tempString;
		// Convert strings to integers (must compile with -std=c++11)
		tempCity.name = stoi(tempString);
		
		linestream >> tempString;
		tempCity.x = stoi(tempString);
		
		linestream >> tempString;
		tempCity.y = stoi(tempString);		

		// Add tempCity struct to Cities vector
		tempCity.visited = false;
		Cities.push_back(tempCity);
	}

	
	/* Test print of parsing 
	std::cout << "~~~Test Print of Cities Info in Vector~~~" << std::endl;
	for(int i = 0; i < Cities.size(); i++){
		std::cout << Cities[i].name << " " << Cities[i].x << " " << Cities[i].y << std::endl;
	}*/
		

	// Call algorithm and track run time 
	std::cout << "~~~Greedy Algorithm Call and Time Tracking~~~" << std::endl;
	
	start = std::clock();
	
	tourGreedy(Cities, outfile);
	
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

	std::cout << "Greedy Time: " << duration << std::endl;
	std::cout << std::endl;

	// Call algorithm and track run time 
	if(Cities.size() > 300) {
		std::cout << "~~~Skipping Branch & Bound Algorithm: Input Too Large~~~" << std::endl;
	}
	else {
		std::cout << "~~~Branch & Bound Algorithm Call and Time Tracking~~~" << std::endl;
	
		start = std::clock();
	
		tourBranchBound(Cities, outfile);
		
		duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

		std::cout << "Branch & Bound Time: " << duration << std::endl;
		std::cout << std::endl;

	}
	// Close files
	outfile.close();
	infile.close();

	return 0;
}

// Function Implementation
/*********************************************************************
** Function: 		distance
** Description:		This function calculates the distance between cities
** Parameters: 		Two city structs
** Pre-Conditions:	Structs initialized
** Post-Conditions:	Correctly calculates and returns distance between
						two cities
*********************************************************************/
int distance(struct City city1, struct City city2){
	int distance = 0;

	// Use formula from project desciption to find distance
	distance = round(sqrt(pow((city1.x - city2.x), 2) + 
						(pow((city1.y - city2.y), 2))));
	return distance;
}


/*********************************************************************
** Function: 		opt
** Description:		This function optimizes results
** Parameters: 		Deque of city structs, total distance 
** Pre-Conditions:	Deque and distance initialized
** Post-Conditions:	Minimizes total distance by iterating trough
						deque and making changes  
*********************************************************************/
void opt(std::deque<City>& Cities, int &length){
	int swapDistance = 0;
	int swap1Distance = 0;
	int swap2Distance = 0;
	int swap3Distance = 0;
	int swap4Distance = 0;
	int oldDistance = 0;
	int select = 0;
	bool swap = false;
	struct City tempCity;
	
	//Simple Optimization for 4 cities
	while(swap){
			bool swap = false;
			//iterate through deque looking for shorter routes
			for(int i = 0; i < Cities.size() - 5; i++){
			//distance 1->2->3->4
			oldDistance = (distance(Cities[i],Cities[i+1]) + 
				distance(Cities[i+2],Cities[i+3]));
			//distance 1->3->2->4
			swapDistance = (distance(Cities[i],Cities[i+2]) + 
				distance(Cities[i+1],Cities[i+3]));
			//if there is a shorter route from 1->3->2->4 than 1->2->3->4, swap			
			if(swapDistance < oldDistance) {
				//swap the cities and decrease the distance
				tempCity = Cities[i+1];
				Cities[i+1] = Cities[i+2];
				Cities[i+2] = tempCity;
				length -= oldDistance;
				length += swapDistance;
				swap = true;
			}		
		}
	}
	
	swap = true;
	//More complex Optimization for 6 cities
	while(swap) {
		//iterate through deque looking for shorter routes
		for(int i = 0; i < Cities.size() - 7; i++){
		select = 0;
		//distance 1->2->3->4->5->6
		oldDistance = (distance(Cities[i],Cities[i+1]) + 
			distance(Cities[i+1],Cities[i+2]) +
			distance(Cities[i+2],Cities[i+3]) +
			distance(Cities[i+3],Cities[i+4]) +
			distance(Cities[i+4],Cities[i+5]));		
		//distance 1->5->3->4->2->6
		swap1Distance = (distance(Cities[i],Cities[i+4]) + 
			distance(Cities[i+4],Cities[i+2]) +
			distance(Cities[i+2],Cities[i+3]) +
			distance(Cities[i+3],Cities[i+1]) +
			distance(Cities[i+1],Cities[i+5]));
		//distance 1->2->5->4->3->6
		swap2Distance = (distance(Cities[i],Cities[i+1]) + 
			distance(Cities[i+1],Cities[i+4]) +
			distance(Cities[i+4],Cities[i+3]) +
			distance(Cities[i+3],Cities[i+2]) +
			distance(Cities[i+2],Cities[i+5]));
		//distance 1->4->3->2->5->6
		swap3Distance = (distance(Cities[i],Cities[i+3]) + 
			distance(Cities[i+3],Cities[i+2]) +
			distance(Cities[i+2],Cities[i+1]) +
			distance(Cities[i+1],Cities[i+4]) +
			distance(Cities[i+4],Cities[i+5]));
		//distance 1->5->4->3->2->6
		swap4Distance = (distance(Cities[i],Cities[i+4]) + 
			distance(Cities[i+4],Cities[i+3]) +
			distance(Cities[i+3],Cities[i+2]) +
			distance(Cities[i+2],Cities[i+1]) +
			distance(Cities[i+1],Cities[i+5])); 
		if(swap1Distance < oldDistance){ select = 1; }
		if(swap2Distance < swap1Distance && swap2Distance < oldDistance){ select = 2; }
		if(swap3Distance < swap2Distance && swap3Distance < oldDistance){ select = 3; }
		if(swap4Distance < swap3Distance && swap4Distance < oldDistance){ select = 4; }
		switch(select){
			case 0: //no change
			swap = false;
				break;
			case 1: //1->5->3->4->2->6
				tempCity = Cities[i+4];
				Cities[i+4] = Cities[i+1];
				Cities[i+1] = tempCity;
				length -= oldDistance;
				length += swap1Distance;
				break;
			case 2: //1->2->5->4->3->6
				tempCity = Cities[i+4];
				Cities[i+4] = Cities[i+2];
				Cities[i+2] = tempCity;
				length -= oldDistance;
				length += swap2Distance;
				break;
			case 3: //1->4->3->2->5->6
				tempCity = Cities[i+3];
				Cities[i+3] = Cities[i+1];
				Cities[i+1] = tempCity;
				length -= oldDistance;
				length += swap3Distance;
				break;
			case 4: //1->5->4->3->2->6
				tempCity = Cities[i+4];
				Cities[i+4] = Cities[i+1];
				Cities[i+1] = tempCity;
				tempCity = Cities[i+2];
				Cities[i+2] = Cities[i+3];
				Cities[i+3] = tempCity;
				length -= oldDistance;
				length += swap4Distance;
				break;
			}
		}
	}
	return;
}


/*********************************************************************
** Function: 		tourGreedy
** Description:		This function calculates a near optimal tour time
						for the traveling salesperson problem
** Parameters: 		city struct vector, out file pointer
** Pre-Conditions:	Vector initializaed with city information
** Post-Conditions:	Correctly calculates and saves total tour time to 
						outfile and lists cities in correct order
						in outfile
*********************************************************************/
void tourGreedy(std::deque<City> CitiesIn, std::ofstream &file){
	// Create variables for interator, length, distances, next cit index
	int i = 0;
	int length = 0;
	int checkDistance = 0;
	int minDistance = 0;
	int nextCityIndex = 0;
	
	// Create temp city to use as current city being visited
	struct City curCity;

	// Bool to check if there are unvisited cities
	bool moreCities = true;

	std::deque<City> Cities;

	while(moreCities){
		// Add shortest distance neighbor to path, intially cities[0]
		CitiesIn[nextCityIndex].visited = true;
		Cities.push_back(CitiesIn[nextCityIndex]);
		curCity = CitiesIn[nextCityIndex];
		length += minDistance;
		
		// Largest possible value for an int- hopefully dist is shorter
		minDistance = 2147483647;
		moreCities = false;
		// Check all neighbors of current city (i.e. all cities)
		for(i = 0; i < CitiesIn.size(); i++){
			//if there are any cities that are not visited, we must continue
			if(!CitiesIn[i].visited){
				moreCities = true;
				checkDistance = distance(curCity, CitiesIn[i]);
				// Find the neighbor with the shortest distance
				if(checkDistance < minDistance)	{
					minDistance = checkDistance;
					nextCityIndex = i;
				}
			}
		}
	}	
	
	//Add the distance from the last city to the first
	length += distance(Cities[Cities.size()-1], Cities[0]);
	
	opt(Cities, length);
	
	std::cout << "Greedy + Optimization Path Length: " << length << std::endl;

	file << length << std::endl;
	for(int i = 0; i < Cities.size(); i++){
		file << Cities[i].name <<  std::endl;
	}

	std::cout << "Greedy + Optimization Results & Path Written to File" << std::endl;
	
	return;
}

/*********************************************************************
** Function: 		tourBranchBound
** Description:		This function calculates a near optimal tour time
						for the traveling salesperson problem
** Parameters: 		city struct deque, out file pointer
** Pre-Conditions:	Vector initializaed with city information
** Post-Conditions:	Correctly calculates and saves total tour time to 
						outfile and lists cities in correct order
						in outfile
*********************************************************************/
void tourBranchBound(std::deque<City> Cities, std::ofstream &file){
	int adjacencyMatrixLength = Cities.size();
	int **adjacencyMatrix = new int*[adjacencyMatrixLength];
	int **childAdjacencyMatrix = new int*[adjacencyMatrixLength];
	int **originalMatrix = new int*[adjacencyMatrixLength];
	std::deque<int> path;
	std::deque<struct Node*> duplicates;
	
	for(int i = 0; i < Cities.size(); i++){
		adjacencyMatrix[i] = new int[adjacencyMatrixLength];
		childAdjacencyMatrix[i] = new int[adjacencyMatrixLength];
		originalMatrix[i] = new int[adjacencyMatrixLength];
	}
	
	
	// Calculate the distances between every vertex
	for(int i = 0; i < adjacencyMatrixLength; i++){
		for(int j = 0; j < adjacencyMatrixLength; j++){
			if(i == j){
				// Set an infinite distance
				adjacencyMatrix[i][j] = -1;
				childAdjacencyMatrix[i][j] = adjacencyMatrix[i][j];
				originalMatrix[i][j] = adjacencyMatrix[i][j];
			}
			else{
				// Calculate the distance from Cities[i] to Cities[j]
				adjacencyMatrix[i][j] = distance(Cities[i], Cities[j]);
				childAdjacencyMatrix[i][j] = adjacencyMatrix[i][j];
				originalMatrix[i][j] = adjacencyMatrix[i][j];
			}
			
		}
	}	
	
	// Debugging
	// std::cout << "Original Adjacency Matrix" << std::endl;
	// for(int i = 0; i < adjacencyMatrixLength; i++){
		// for(int j = 0; j < adjacencyMatrixLength; j++){
			// std::cout << adjacencyMatrix[i][j] << " ";
		// }
		// std::cout << std::endl;
	// }
	
	// Select a root vertex
	struct Node *root = createNode(Cities[0].name, NULL);
	struct Node *currentNode = root;
	Cities.pop_front();
	
	// Add the root as the first node on the path
	path.push_back(root->city);

	// Reduce the adjacency matrix
	root->lower_bound = reduceMatrix(adjacencyMatrix, adjacencyMatrixLength);
	
	// Loop through the algorithm until a min Hamiltonian cycle is found
	while(Cities.size()){		
		// Create all the children nodes for the current node
		for(int i = 0; i < Cities.size(); i++){
			struct Node *childNode = createNode(Cities[i].name, currentNode);
			currentNode->children.push_back(childNode);
		}
		
		// Loop through the children finding the smallest lower bound
		int minimumLowerBoundElement = 0;
		minimumLowerBoundElement = findSmallestLowerBound(currentNode, 
			childAdjacencyMatrix, 
			adjacencyMatrix,
			adjacencyMatrixLength, 
			duplicates, 
			Cities, 
			path);


		// Travel to the smallest lower bound child node
		chooseDestination(adjacencyMatrix, 
				currentNode->children[minimumLowerBoundElement]->city, 
				currentNode->city, 
				adjacencyMatrixLength);
		
		currentNode = currentNode->children[minimumLowerBoundElement];
		path.push_back(currentNode->city);		
		
		// Remove the city from the set of possible cities to visit next
		int childIndex = -1;
		
		for(int i = 0; i < Cities.size(); i++){
			if(Cities[i].name == currentNode->city){
				childIndex = i;
			}
		}
		
		Cities.erase(Cities.begin() + childIndex);

		
		// If there were previous duplicate lower limit values found
		if(duplicates.size() && duplicates[0]->lower_bound < currentNode->lower_bound){
			if(currentNode->savedAdjacenyMatrix != NULL){
				// Save the current decision's values
				currentNode->savedCities = Cities;
				currentNode->savedPath = path;
				currentNode->savedAdjacenyMatrix = new int*[adjacencyMatrixLength];
				for(int j = 0; j < adjacencyMatrixLength; j++){
					currentNode->savedAdjacenyMatrix[j] = new int[adjacencyMatrixLength];
				}
				
				// Save a copy the adjacency matrix into the node struct
				for(int k = 0; k < adjacencyMatrixLength; k++){
					for(int j = 0; j < adjacencyMatrixLength; j++){
						currentNode->savedAdjacenyMatrix[k][j] = adjacencyMatrix[k][j];
					}
				}
				
				// Add the vertex to the list of duplcates
				duplicates.push_back(currentNode);
			}			
			
			// Go back to the duplicate lowest bound element and search it's children
			Cities = duplicates[0]->savedCities;
			path = duplicates[0]->savedPath;
			
			for(int k = 0; k < adjacencyMatrixLength; k++){
				for(int j = 0; j < adjacencyMatrixLength; j++){
					adjacencyMatrix[k][j] = duplicates[0]->savedAdjacenyMatrix[k][j];
				}
			}
			
			// Remove the duplicate vertex from the list
			currentNode = duplicates[0];
			duplicates.pop_front();
		}
	}
	
	path.push_back(root->city);
	
	// Calculate the Cycle length
	int finalLength = calculatePathLength(originalMatrix, path);
	std::cout << "Branch & Bound Path length: " << finalLength << std::endl;
	
	/*
	std::cout << "Final Path" << std::endl;
	for(int i = 0; i < path.size(); i++){
		std::cout << path[i] << " ";
	}
	std::cout << std::endl;
	*/
	
	// Delete the adjacency matrices
	for(int i = 0; i < adjacencyMatrixLength; i++){
		delete(adjacencyMatrix[i]);
		delete(childAdjacencyMatrix[i]);
		delete(originalMatrix[i]);
	}
	delete(adjacencyMatrix);
	delete(childAdjacencyMatrix);
	delete(originalMatrix);	
	
	// Delete the decision tree
	if(root->children.size()){
		deleteChildren(root, adjacencyMatrixLength);
	}
	delete(root);
		
	/*
		cost(currentNode) = cost(parentNode) + parent's Adjacency matrix[parentNode][currentNode] + running sum of all reductions done to the parent adjacency matrix by selecting the current node
	*/
}

/*********************************************************************
** Function: 		reduxMatrix
** Description:		This function reduces the matrix for the B&B algoritm 
** Parameters: 		Adjacency matrix, length of the matix
** Pre-Conditions:	The matrix and length are initialized
** Post-Conditions:	Correctly reduces matrix
*********************************************************************/
int reduceMatrix(int **adjacencyMatrix, int matrixLength){
	int totalReduction = 0;
	
	// Find the minimum value in each row and subtract it from all other
	// row elements
	for(int i = 0; i < matrixLength; i++){
		// Find the minimum element
		int minimumDistance = minDistance(adjacencyMatrix[i], matrixLength);
		
		// Check for the irreducable conditions
		if(minimumDistance != -1){
			// Update the total reduction sum for the table
			totalReduction += minimumDistance;
			
			// Reduce the values in the row using the minimum distance
			for(int j = 0; j < matrixLength; j++){
				if(adjacencyMatrix[i][j] != -1){
					if(adjacencyMatrix[i][j] >= minimumDistance){
						adjacencyMatrix[i][j] -= minimumDistance;
					}
					else{
						adjacencyMatrix[i][j] = 0;
					}
				}			
			}
		}		
	}
	
	// Find the minimum value in each column
	for(int i = 0; i < matrixLength; i++){
		// Transpose the column values into row values for the minDistance
		// function
		int transposedColumn[matrixLength];
		for(int j = 0; j < matrixLength; j++){
			transposedColumn[j] = adjacencyMatrix[j][i];	
		}
		
		// Find the minimum element
		int minimumDistance = minDistance(transposedColumn, matrixLength);
		
		// Check for the irreducable conditions
		if(minimumDistance != -1){
			// Update the total reduction sum for the table
			totalReduction += minimumDistance;
			
			// Reduce the values in the row using the minimum distance
			for(int j = 0; j < matrixLength; j++){
				if(adjacencyMatrix[j][i] != -1){
					if(adjacencyMatrix[j][i] >= minimumDistance){
						adjacencyMatrix[j][i] -= minimumDistance;
					}
					else{
						adjacencyMatrix[j][i] = 0;
					}
				}			
			}
		}		
	}
	
	return totalReduction;
}

/*********************************************************************
** Function: 		minDistance
** Description:		This function finds the array with the minimum 
						distance, i.e. the smallest int
** Parameters: 		Array of ints and array length
** Pre-Conditions:	Array and length initialized
** Post-Conditions:	Returns smallest int in the array or a flag 
						value of -1 to indicate there was not
						a smallest value
*********************************************************************/
int minDistance(int array[], int arrayLength){
	int minimumDistance = 65000;
	bool nonNegativeIntegerFound = false;
	
	// Find the minimum value in the array
	for(int i = 0; i < arrayLength; i++){
		if(array[i] != -1){
			if(array[i] < minimumDistance){
				minimumDistance = array[i];
				nonNegativeIntegerFound = true;
			}
		}
	}
	
	// Check to see if a non-negative integer was found
	if(nonNegativeIntegerFound){
		return minimumDistance;
	}
	else{
		// No answer found, return infinity (-1)
		return -1;
	}
}

/*********************************************************************
** Function: 		createNode
** Description:		Creates a node 
** Parameters: 		Int for a city, the parent node pointer
** Pre-Conditions:	Int and pointer are initializaed
** Post-Conditions:	Returns the node pointer 
*********************************************************************/
struct Node *createNode(int newCity, struct Node *newParent){
	struct Node *newNode = new struct Node;
	newNode->city = newCity;
	newNode->parent = newParent;
	newNode->savedAdjacenyMatrix = NULL;
	
	return newNode;
}

/*********************************************************************
** Function: 		findSmallestLowerBound
** Description:		Finds and returns the smallerst lower bound
** Parameters: 		Node, pointers to the child and adjacency martices,
						matirx length, deques for duplicates, cities,
						and the current path
** Pre-Conditions:	Parameters initialized 
** Post-Conditions:	Returns smallest lower bound
*********************************************************************/
int findSmallestLowerBound(struct Node *currentNode, 
	int **childAdjacencyMatrix, 
	int **adjacencyMatrix, 
	int matrixLength,
	std::deque<struct Node*> duplicates,
	std::deque<City> cities,
	std::deque<int> path){
	// Loop through the children finding the smallest lower bound
	int lowerBounds[currentNode->children.size()];
	int minimumLowerBoundElement = 0;
	
	for(int i = 0; i < currentNode->children.size(); i++){
		// Copy the adjacency matrix into the child adjacency matrix
		for(int k = 0; k < matrixLength; k++){
			for(int j = 0; j < matrixLength; j++){
				childAdjacencyMatrix[k][j] = adjacencyMatrix[k][j];
			}
		}
		
		// Select the current child as the destination node and remove
		// both the option to travel from the current node, as well as
		// the option to travel to the current child
		chooseDestination(childAdjacencyMatrix, 
			currentNode->children[i]->city, 
			currentNode->city, 
			matrixLength);
		
		// Reduce the updated child adjacency matrix
		int childReduction = reduceMatrix(childAdjacencyMatrix, matrixLength);
		
		// Calculate the cost to travel from the current node to the 
		// child node under consideration
		lowerBounds[i] = currentNode->lower_bound + 
			adjacencyMatrix[currentNode->city][currentNode->children[i]->city] + 
			childReduction;
			
		currentNode->children[i]->lower_bound = lowerBounds[i];
		
		// Check to see if a smaller lower bound has been identified
		if(lowerBounds[i] < lowerBounds[minimumLowerBoundElement]){
			minimumLowerBoundElement = i;// std::cout << "New lower bound found: " << currentNode->children[minimumLowerBoundElement]->lower_bound << std::endl;
		}
	}
	
	// Search for duplicate lower bounds
	for(int i = 0; i < currentNode->children.size(); i++){
		if(lowerBounds[i] == lowerBounds[minimumLowerBoundElement]){
			duplicates.push_back(currentNode->children[i]);
			
			// Create a new adjacency matrix to save the old state
			currentNode->children[i]->savedAdjacenyMatrix = new int*[matrixLength];
			for(int j = 0; j < matrixLength; j++){
				currentNode->children[i]->savedAdjacenyMatrix[j] = new int[matrixLength];
			}
			
			// Save a copy the adjacency matrix into the node struct
			for(int k = 0; k < matrixLength; k++){
				for(int j = 0; j < matrixLength; j++){
					currentNode->children[i]->savedAdjacenyMatrix[k][j] = adjacencyMatrix[k][j];
				}
			}
			
			// Choose the child as the destination vertex and reduce the adjacency matrix appropriately
			chooseDestination(currentNode->children[i]->savedAdjacenyMatrix, 
				currentNode->children[i]->city, 
				currentNode->city, 
				matrixLength);
		
			// Reduce the updated child adjacency matrix
			reduceMatrix(currentNode->children[i]->savedAdjacenyMatrix, matrixLength);
			
			// Save a copy of the current path
			currentNode->children[i]->savedPath = path;
			currentNode->children[i]->savedPath.push_back(currentNode->children[i]->city);
			
			// Save a copy of the current cities
			currentNode->children[i]->savedCities = cities;
		}
	}
	
	return minimumLowerBoundElement;
}

/*********************************************************************
** Function: 		chooseDestination
** Description:		Chooses the best destination from the child matrix
** Parameters: 		matrix of children, ints for destination, origin, 
						and length
** Pre-Conditions:	Parameters initialized
** Post-Conditions:	Correctly choses the best destination
*********************************************************************/
void chooseDestination(int **childAdjacencyMatrix, 
	int destination, 
	int origin, 
	int matrixLength){
	// Wipe out the row of the origin node
	for(int j = 0; j < matrixLength; j++){
		childAdjacencyMatrix[origin][j] = -1;
	}
	
	// Wipe out the column of the desitination node
	for(int j = 0; j < matrixLength; j++){
		childAdjacencyMatrix[j][destination] = -1;
	}
	
	// Wipe out the complementary edge 
	childAdjacencyMatrix[destination][origin] = -1;	
}

/*********************************************************************
** Function: 		deleteChildren
** Description:		Deletes chilren to properly manage memory
** Parameters: 		Node struct and length
** Pre-Conditions:	None
** Post-Conditions:	Deletes all matrices
*********************************************************************/
void deleteChildren(struct Node *currentNode, int matrixLength){
	// Recursively delete children
	
	if(currentNode->children.size()){
		for(int j = 0; j < currentNode->children.size(); j++){
			deleteChildren(currentNode->children[j], matrixLength);
		}
	}
	else{
		if(currentNode->savedAdjacenyMatrix != NULL){
			for(int j = 0; j < matrixLength; j++){
				delete(currentNode->savedAdjacenyMatrix[j]);
			}
			delete(currentNode->savedAdjacenyMatrix);
		}
		
		delete(currentNode);		
	}
}

/*********************************************************************
** Function: 		calculatePathLength
** Description:		Finds the length of the path
** Parameters: 		Matrix, path deque, and length
** Pre-Conditions:	Parameters properly initialized
** Post-Conditions:	Correctly returns path length
*********************************************************************/
long unsigned int calculatePathLength(int **originalMatrix, std::deque<int> path){
	long unsigned int totalLength = 0;
	
	// Calculate the total distance from start vertex
	// To last vertex in path
	for(int i = 0, j = 1; j < path.size(); i++, j++){
		// Find the distance between the two nodes
		totalLength += originalMatrix[path[i]][path[j]];
	}
	
	return totalLength;
}




