"""
CMPE 478 - Homework 2
Name: Can Atakan Ugur
Student ID: 2017400057
"""

import subprocess
import csv

M_VALUES = [100000, 1000000, 10000000, 100000000, 200000000, 250000000]  # M boundaries to run the program for
NUMBER_OF_PROCESSORS = [1, 2, 4, 8]


# after each run, the results are recorded to this initial csv file created by this function
def create_initial_csv():
    output_file = open("results.csv", 'w')
    csv_writer = csv.writer(output_file)
    # in the first file, number of processors, M, and time (seconds) will be recorded only
    fields = ['P', 'M', 'Time']
    csv_writer.writerow(fields)
    output_file.close()


# calls the .cpp program for given M_VALUES with 1, 2, 4, and 8 processors separately
def execute():
    subprocess.call(["mpic++ prime-generator.cpp -o prime-generator"], shell=True)  # compile
    for current_count in NUMBER_OF_PROCESSORS:
        command_string = f"mpirun -np {current_count} prime-generator"
        for m in M_VALUES:
            command_string = command_string + " " + str(m)  # append all M values
        subprocess.call([command_string], shell=True)  # give all values as arguments to the .cpp program


# The initially created csv file does not match the format in the description. Hence, should be arranged.
def arrange_csv():
    # Group the rows according to M values. This holds a list of lists of rows with the same M values.
    groups = [[] for _ in range(len(M_VALUES))]

    # read the file and group the data
    with open("results.csv", newline='\n') as output_file:
        csv_reader = csv.DictReader(output_file)
        for row in csv_reader:
            current_m = int(row['M'])
            index = -1
            count = 0
            for m in M_VALUES:
                if current_m == m:
                    index = count
                    break
                count = count + 1
            groups[index].append(row)

    # overwrite, following the wanted format
    with open("results.csv", 'w', newline='\n') as output_file:
        csv_writer = csv.writer(output_file)
        fields = ['M', 'T1', 'T2', 'T4', 'T8', 'S2', 'S4', 'S8']
        csv_writer.writerow(fields)

        for group in groups:

            current_data = []
            m = int(group[0]['M'])
            current_data.append(m)

            times = []
            for row in group:  # extract time data
                time = float(row['Time'])
                times.append(time)
                current_data.append(time)  # append time data in order: T1, T2, T4, T8

            t1 = times[0]  # sequential time
            for i in range(1, len(times)):  # calculate speedups
                if times[i] >= t1:  # sequential is faster
                    current_data.append('-')  # then there is no speedup
                else:
                    speedup = t1 / times[i]
                    current_data.append(round(speedup, 5))

            csv_writer.writerow(current_data)


create_initial_csv()
execute()
arrange_csv()
