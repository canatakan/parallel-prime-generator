/*
 * CMPE 478 - Homework 2
 * Name: Can Atakan Ugur
 * Student ID: 2017400057
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

using namespace std;

#include "mpi.h"

// prints the array of prime numbers
void printPrimes(const std::vector<int> &primes) {
	for (int i = 0; i < primes.size(); ++i) {
		cout << primes[i] << " ";
	}
	cout << endl;
}

/*
 * To run the program, give M value(s) to be calculated as
 * arguments.
 *
 * Example:
 * mpirun -np 8 prime-generator 100 1000 10000 100000
 */

int main(int argc, char *argv[]) {

	if (argc < 2) // there should be at least one M value given as an argument
		return -1;

	MPI_Init(&argc, &argv);
	int rank;
	int world;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world);

	for (int runNumber = 0; runNumber < argc - 1; ++runNumber) { // re-run until all M values are calculated

		int M = atoi(argv[runNumber + 1]);
		std::vector<int> primes;

		double startTime, endTime; // to measure the runtime

		// start measuring runtime
		MPI_Barrier (MPI_COMM_WORLD);
		startTime = MPI_Wtime();

		int n;
		int j;
		int k;
		int quo, rem;

		// the size variable that is used by each process to hold the
		// length of their partial prime number arrays
		int currentSize;

		if (rank == 0) { // master

			/*
			 * (1) Master should first calculate the prime numbers up to sqrt(M) since
			 * this is the sequential part of the given problem. (2) After completing the
			 * sequential part, master sends the array of prime numbers up to sqrt(M)
			 * to all slaves. (3) Afterwards, master itself can also continue with the part
			 * from sqrt(M) until the start of the next processor's zone. (4) Finally, master
			 * has to get the results from all the slaves to combine those into the "primes"
			 * array.
			 */

			// 1. First calculate the prime numbers up to sqrt(M)
			primes.push_back(2);
			n = 3;
			j = 0;

			while (n <= sqrt(M)) {
				j += 1;
				primes.insert(primes.begin() + j, n);

				n = n + 2;
				k = 1;
				while (true) {
					quo = n / primes[k];
					rem = n % primes[k];

					if (rem == 0) {
						n = n + 2;
						k = 1;
						continue;
					}

					if (quo <= primes[k])
						break;

					k += 1;
				}
			}

			// 2. Send the vector of prime numbers up to sqrt(M) to slaves.

			currentSize = primes.size();
			for (int i = 0; i < world - 1; ++i) { // send the vector size to all slaves
				MPI_Send(&currentSize, 1, MPI_INT, (i + 1), 0, MPI_COMM_WORLD);
			}

			for (int i = 0; i < world - 1; ++i) { // send the vector to all slaves
				MPI_Send(&primes[0], currentSize, MPI_INT, (i + 1), 0,
						MPI_COMM_WORLD);
			}

			// 3. Keep finding primes up to the start of the next processor's part.

			n += 2;
			while (true) { // keep finding primes on its own interval

				k = 1;
				while (true) {
					quo = n / primes[k];
					rem = n % primes[k];

					if (rem == 0) {
						n = n + 2;
						k = 1;
						continue;
					}

					if (quo <= primes[k])
						break;

					k += 1;
				}

				if (n < (M / world)) {
					j += 1;
					primes.insert(primes.begin() + j, n);
					n = n + 2;
				} else
					break;

			}

			// 4. Get the results from the slaves and combine them.

			for (int i = 0; i < world - 1; ++i) { // receive partial prime vectors

				int slaveSize;
				MPI_Recv(&slaveSize, 1, MPI_INT, (i + 1), 0, MPI_COMM_WORLD,
						MPI_STATUS_IGNORE);

				vector<int> slaveArray;
				slaveArray.resize(slaveSize);
				MPI_Recv(&slaveArray[0], slaveSize, MPI_INT, (i + 1), 0,
						MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				for (int slaveIndex = 0; slaveIndex < slaveSize; ++slaveIndex) {
					primes.push_back(slaveArray[slaveIndex]);
				}
			}

		} else { // slaves

			// receive the vector of prime numbers up to sqrt(M) from the master
			MPI_Recv(&currentSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
					MPI_STATUS_IGNORE);
			primes.resize(currentSize);
			MPI_Recv(&primes[0], currentSize, MPI_INT, 0, 0, MPI_COMM_WORLD,
					MPI_STATUS_IGNORE);

			// store the prime numbers in its own part in this vector
			vector<int> partialPrimes;

			n = (rank * M / world) + 2;
			if (n % 2 == 0)
				n -= 1;
			j = -1;

			while (true) {

				if (n <= sqrt(M)) { // this is already calculated by the master
					n += 2;
					continue;
				}

				k = 1;
				while (true) {

					quo = n / primes[k];
					rem = n % primes[k];

					if (rem == 0) {
						n = n + 2;
						k = 1;
						if (n > (rank + 1) * M / world)
							break;
						else
							continue;
					}

					if (quo <= primes[k])
						break;

					k += 1;
				}

				if (n <= (rank + 1) * M / world) {
					j += 1;
					partialPrimes.insert(partialPrimes.begin() + j, n);
					n = n + 2;
				} else
					break;

			}

			// send the results to the master
			currentSize = partialPrimes.size();
			MPI_Send(&currentSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Send(&partialPrimes[0], currentSize, MPI_INT, 0, 0,
					MPI_COMM_WORLD);

		}

		MPI_Barrier(MPI_COMM_WORLD);
		endTime = MPI_Wtime();
		// measuring time stops

		if (rank == 0) { // check master's execution time
			double execTime = endTime - startTime; // execution time is completed
			ofstream outfile;
			outfile.open("results.csv", ios_base::app); // append, instead of overwriting
			outfile << world << "," << M << "," << execTime << endl;
			// printPrimes(primes); // master can also print all of the prime numbers in [2, M] interval
		}
	}

	MPI_Finalize();
	return 0;
}
