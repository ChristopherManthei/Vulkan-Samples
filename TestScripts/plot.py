import matplotlib.pyplot as plt
import os

def getPathToData() -> str:
    script_path = os.path.realpath(__file__)
    test_results_path = os.path.join(script_path, "..", "TestResults")

    print(test_results_path)

    test_result_folders = os.listdir(test_results_path)

    print("Choose which test result to plot [latest]:")
    print("[latest] - {}".format(test_result_folders[-1]))

    for i in range(len(test_result_folders) - 1, -1, -1):
        print("[{}] - {}".format(i, test_result_folders[i]))

    while True:
        chosen = input("")

        if chosen.upper() == "LATEST" or chosen.isspace() or chosen == "":
            return test_result_folders[-1]
        else:
            try:
                idx = int(chosen)
                if idx >= 0 and idx < len(test_result_folders):
                    return test_result_folders[idx]
            except:
                pass # If we fall through the if we retry, so no error handling is needed here

        print("Input not correct. Please retry.")

def readJson

def plot(path_to_data: str) -> None:
    # Sample JSON data
    json_data = {
        "categories": ["A", "B", "C"],
        "values": [10, 20, 15]
    }
    # Plotting
    plt.bar(json_data["categories"], json_data["values"])
    plt.xlabel('Categories')
    plt.ylabel('Values')
    plt.title('Bar Chart of JSON Data')
    plt.show()

if __name__ == "__main__":
    path_to_data = getPathToData()
    plot(path_to_data)

