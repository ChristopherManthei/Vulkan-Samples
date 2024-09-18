from typing import Any, Tuple
import matplotlib.pyplot as plt
import os
import json
import functools

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
            return os.path.join(script_path, "..", "TestResults", test_result_folders[-1])
        else:
            try:
                idx = int(chosen)
                if idx >= 0 and idx < len(test_result_folders):
                    return os.path.join(script_path, "..", "TestResults", test_result_folders[idx])
            except:
                pass # If we fall through the if we retry, so no error handling is needed here

        print("Input not correct. Please retry.")

def readJson(path_to_results_folder: str) -> list[dict[str, Any]]:
    result_data = []

    for platform_folder in os.listdir(path_to_results_folder):
        for json_file_name in os.listdir(os.path.join(path_to_results_folder, platform_folder)):
            with open(os.path.join(path_to_results_folder, platform_folder, json_file_name), "r") as file:
                data = json.load(file)

                for measurement in data["Measurements"]:
                    current = {}
                    current["Plotted"] = False
                    current["SummaryShown"] = False
                    current["Timestamp"] = os.path.split(path_to_results_folder)[1]
                    current["Platform"] = platform_folder
                    current["SampleName"] = json_file_name.removesuffix("_data.json")
                    current["MeasurementName"] = measurement["Name"]
                    current["DataPointsCount"] = measurement["DataPointsCount"]
                    current["DataPoints"] = measurement["DataPoints"]
                    current["Summary"] = measurement["Summary"]
                    result_data.append(current)
    
    def compare(a, b):
        if (a["MeasurementName"] == b["MeasurementName"]):
            if (a["SampleName"] == b["SampleName"]):
                if (a["Platform"] == b["Platform"]):
                    return 0
                elif (a["Platform"] > b["Platform"]):
                    return 1
                else:
                    return -1
            elif (a["SampleName"] > b["SampleName"]):
                return 1
            else:
                return -1
        elif (a["MeasurementName"] > b["MeasurementName"]):
            return 1
        else:
            return -1

    return sorted(result_data, key=functools.cmp_to_key(compare))

def calculatePrefixAndSuffixWhitespaces(headerLen: int, valueLen: int) -> Tuple[str, str, str, str]:
    def calcPrefixSuffix(bigger: int, smaller: int) -> Tuple[int, int]:
        number_whitespaces_to_place = bigger - smaller
        half_rounded_down = int(number_whitespaces_to_place / 2)
        return (half_rounded_down + number_whitespaces_to_place % 2, half_rounded_down)
    
    if (headerLen > valueLen):
        value_prefix, value_suffix = calcPrefixSuffix(headerLen, valueLen)
        return ("", "", " "*value_prefix, " "*value_suffix)
    elif (valueLen > headerLen):
        value_prefix, value_suffix = calcPrefixSuffix(valueLen, headerLen)
        return (" "*value_prefix, " "*value_suffix, "", "")
    else:
        return ("", "", "", "")

def selectDataToPlot(result_data: list[dict[str, Any]]):
    max_length = {"Timestamp": 0, "MeasurementName": 0, "SampleName": 0, "Platform": 0}
    max_length["Index"] = len(str(len(result_data) - 1)) if len(str(len(result_data) - 1)) > len("Index") else len("Index")

    for entry in result_data:
        max_length["Timestamp"] = max(max_length["Timestamp"], len(entry["Timestamp"]))
        max_length["MeasurementName"] = max(max_length["MeasurementName"], len(entry["MeasurementName"]))
        max_length["SampleName"] = max(max_length["SampleName"], len(entry["SampleName"]))
        max_length["Platform"] = max(max_length["Platform"], len(entry["Platform"]))

    header_boundaries = {}
    header_boundaries["Timestamp"] = {}
    (header_boundaries["Timestamp"]["Prefix"], header_boundaries["Timestamp"]["Suffix"], _, _) = calculatePrefixAndSuffixWhitespaces(len("Timestamp"), max_length["Timestamp"])

    header_boundaries["MeasurementName"] = {}
    (header_boundaries["MeasurementName"]["Prefix"], header_boundaries["MeasurementName"]["Suffix"], _, _) = calculatePrefixAndSuffixWhitespaces(len("MeasurementName"), max_length["MeasurementName"])

    header_boundaries["SampleName"] = {}
    (header_boundaries["SampleName"]["Prefix"], header_boundaries["SampleName"]["Suffix"], _, _) = calculatePrefixAndSuffixWhitespaces(len("SampleName"), max_length["SampleName"])
    
    header_boundaries["Platform"] = {}
    (header_boundaries["Platform"]["Prefix"], header_boundaries["Platform"]["Suffix"], _, _) = calculatePrefixAndSuffixWhitespaces(len("Platform"), max_length["Platform"])

    print("{}Index | Plotted | SummaryShown | {}Timestamp{} | {}MeasurementName{} | {}SampleName{} | {}Platform{}".format(
          " "*(len(str(len(result_data) - 1)) - len("Index")) if len(str(len(result_data) - 1)) > len("Index") else "",
          header_boundaries["Timestamp"]["Prefix"], header_boundaries["Timestamp"]["Suffix"], 
          header_boundaries["MeasurementName"]["Prefix"], header_boundaries["MeasurementName"]["Suffix"], 
          header_boundaries["SampleName"]["Prefix"], header_boundaries["SampleName"]["Suffix"], 
          header_boundaries["Platform"]["Prefix"], header_boundaries["Platform"]["Suffix"]))
    
    current_index = 0
    for entry in result_data:
        vb = {}
        vb["Timestamp"] = {}
        (_, _, vb["Timestamp"]["Prefix"], vb["Timestamp"]["Suffix"]) = calculatePrefixAndSuffixWhitespaces(max_length["Timestamp"], len(entry["Timestamp"]))
        
        vb["MeasurementName"] = {}
        (_, _, vb["MeasurementName"]["Prefix"], vb["MeasurementName"]["Suffix"]) = calculatePrefixAndSuffixWhitespaces(max_length["MeasurementName"], len(entry["MeasurementName"]))
        
        vb["SampleName"] = {}
        (_, _, vb["SampleName"]["Prefix"], vb["SampleName"]["Suffix"]) = calculatePrefixAndSuffixWhitespaces(max_length["SampleName"], len(entry["SampleName"]))
        
        vb["Platform"] = {}
        (_, _, vb["Platform"]["Prefix"], vb["Platform"]["Suffix"]) = calculatePrefixAndSuffixWhitespaces(max_length["Platform"], len(entry["Platform"]))

        print("{}{} |    {}    |       {}      | {}{}{} | {}{}{} | {}{}{} | {}{}{} ".format(
            " "*(max_length["Index"] - len(str(current_index))), str(current_index),
            "X" if entry["Plotted"] else " ",
            "X" if entry["SummaryShown"] else " ",
            vb["Timestamp"]["Prefix"], entry["Timestamp"], vb["Timestamp"]["Suffix"],
            vb["MeasurementName"]["Prefix"], entry["MeasurementName"], vb["MeasurementName"]["Suffix"],
            vb["SampleName"]["Prefix"], entry["SampleName"], vb["SampleName"]["Suffix"],
            vb["Platform"]["Prefix"], entry["Platform"], vb["Platform"]["Suffix"]
        ))

        current_index += 1
        
    while True:
        chosen = input("Please choose a number between [0, {}]: ".format(len(result_data) - 1))

        try:
            if chosen.upper() == "Q":
                exit(0)

            idx = int(chosen)
            if idx >= 0 and idx < len(result_data):
                result_data[idx]["Plotted"] = not result_data[idx]["Plotted"]
                result_data[idx]["SummaryShown"] = not result_data[idx]["SummaryShown"]
                break
        except Exception as e:
            pass

        print("Input not correct. Please retry and choose a number between [0, {}].".format(len(result_data) - 1))
    
    return result_data

def plot(data: list[dict[str, Any]]) -> None:
    plt.close()
    for entry in data:
        # Sample JSON data
        # json_data = {
        #     "categories": ["A", "B", "C"],
        #     "values": [10, 20, 15]
        # }
        # Plotting
        if entry["Plotted"]:
            plt.plot(entry["DataPoints"], label="{}_{}_{}".format(entry["SampleName"], entry["MeasurementName"], entry["Platform"]))
        # plt.bar(json_data["categories"], json_data["values"])
        # plt.xlabel('Categories')
        # plt.ylabel('Values')
    plt.title('How much can we upset Vulkan drivers?')
    plt.show(block=False)

if __name__ == "__main__":
    path_to_results_folder = getPathToData()
    json_data = readJson(path_to_results_folder)

    while True:
        json_data = selectDataToPlot(json_data)
        plot(json_data)

    # import ipdb; ipdb.set_trace()

