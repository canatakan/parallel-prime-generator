# CMPE 478 - Homework 2

## Files

***

The submission contains two files:

* `prime-generator.cpp` is the main implementation for the project. **Please note that it does not print the
  resulting `primes` vector in order not to slow down the execution time.** You can uncomment line `#235` to get the
  prime numbers printed.
* `execute.py` is used to run the main MPI program multiple times with different number of processors. It also arranges
  the resulting CSV file to provide it as desired by the project description.

## How to run

***

* To generate the well-formed CSV file with multiple run data, you can use the `execute.py` script.
* The script includes a list named `M_VALUES`, you can change that list to try the runs with different M values.
* After making the adjustments, simply run the Python script as below:

```
python3 execute.py
```

* If you would like to try out the print function, simply compile and run the program manually, without using the
  script. The program takes M values as arguments. For example:

```
mpic++ prime-generator.cpp -o  prime-generator
mpirun -np 4 prime-generator 71 10000 33333
```

* Please note that however, the generated CSV file will not be in the wanted format as it is normally arranged by
  the `execute.py` script to fit the CSV file into the deserved form.