import os
import pathlib
import shutil
import csv

EXE_PATH = "..\\out\\build\\x64-Release"
OUTPUT_DIR = "output"

periodic = ["yes"]
methods = ["Moore"]
sizes = [20, 30, 40, 50, 100]
no_seeds = [10, 100, 1000]
mc_count = [1]
mc_kts = [0.6]

def create_config(periodic, method, size, seeds, mc_iterations, mc_kt):
    with open("config.txt", "w") as f:
        f.write("periodic={}\n".format(periodic))
        f.write("method={}\n".format(method))
        f.write("x_size={}\n".format(size))
        f.write("y_size={}\n".format(size))
        f.write("z_size={}\n".format(size))
        f.write("random_seeds={}\n".format(seeds))
        f.write("MC_iterations={}\n".format(mc_iterations))
        f.write("MC_kt={}\n".format(mc_kt))

def perform_test(periodic, method, size, seeds, mc_iterations, mc_kt):
    current_path = pathlib.Path().absolute()
    os.chdir(EXE_PATH)
    create_config(periodic, method, size, seeds, mc_iterations, mc_kt)
    os.system("mpiexec.exe -n 6 ./CellularAutomaton.exe > time.txt")
    os.chdir(current_path)
    file_suffix = "_{}_{}_{}_{}_{}_{}".format(periodic, method, size, seeds, mc_iterations, mc_kt)
    output_file = "{}\\time{}.txt".format(OUTPUT_DIR, file_suffix)
    shutil.copyfile("{}\\time.txt".format(EXE_PATH), output_file)
    shutil.copyfile("{}\\board.txt".format(EXE_PATH), "{}\\board{}.txt".format(OUTPUT_DIR, file_suffix))
    return output_file

def perform_all_tests():
    try:
        shutil.rmtree(OUTPUT_DIR)
    except:
        pass
    os.mkdir(OUTPUT_DIR)

    for period in periodic:
        for method in methods:
            for size in sizes:
                for seeds in no_seeds:
                    for mc_iterations in mc_count:
                        for mc_kt in mc_kts:
                            print("Performing test for: {}, {}, {}, {}, {}, {}".format(period, method, size, seeds, mc_iterations, mc_kt))
                            filename = perform_test(period, method, size, seeds, mc_iterations, mc_kt)
                            print("Output saved in {}/{}".format(OUTPUT_DIR, filename))

def compile_results():
    output_files = [i for i in os.listdir(OUTPUT_DIR) if os.path.isfile(os.path.join(OUTPUT_DIR, i)) and i.startswith("time")]

    data = {}
    for output_file in output_files:
        header = output_file[5:-4]
        data[header] = {}
        with open(os.path.join(OUTPUT_DIR, output_file), 'r') as f:
            for line in f.readlines():
                splitLine = line.strip().split('=')
                values = [int(val) for val in splitLine[1].split(",") if val.isnumeric()]
                data[header][splitLine[0]] = values
    max_iterations = 0
    max_mc_iterations = 0
    for _, entry in data.items():
        if len(entry['Iterations']) > max_iterations:
            max_iterations = len(entry['Iterations'])
        if len(entry['MCiterations']) > max_mc_iterations:
            max_mc_iterations = len(entry['MCiterations'])
    
    with open('results.csv', 'w', newline='') as csvfile:
        fieldnames = ['Options', 'Read config', 'Generating structure', 'MonteCarlo', 'Save to file'] + ['Iteration {}'.format(i) for i in range(max_iterations)] + ['MC iteration {}'.format(i) for i in range(max_mc_iterations)]
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        
        for key, entry in data.items():
            data_to_save = {
                'Options': key,
                'Read config': entry['ReadConfig'][0],
                'Generating structure': entry['Structure_generation'][0],
                'MonteCarlo': entry['MonteCarlo'][0],
                'Save to file': entry['WriteToFile'][0]
            }
            i = 0
            for val in entry['Iterations']:
                data_to_save['Iteration {}'.format(i)] = val
                i = i + 1
            i = 0
            for val in entry['MCiterations']:
                data_to_save['MC iteration {}'.format(i)] = val
                i = i + 1
            writer.writerow(data_to_save)
        


if __name__ == "__main__":
    perform_all_tests()
    compile_results()
    