from typing import Any, Tuple
import matplotlib.ticker as matplttick
import matplotlib.pyplot as plt
import os
import json
import functools
import math
from pathlib import Path
from datetime import datetime

def getPathToData() -> str:
    script_path = os.path.realpath(__file__)
    test_results_path = os.path.join(script_path, "..", "TestResults")

    test_result_folders = sorted(Path(test_results_path).iterdir(), key=os.path.getmtime)

    print("Choose which test result to plot [latest]:")
    print("[latest] - {}".format(test_result_folders[-1].name))

    for i in range(len(test_result_folders) - 1, -1, -1):
        print("[{}] - {}".format(i, test_result_folders[i].name))

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

def readJson(path_to_results_folder: str) -> list[dict[str, Any]]:
    result_data = []

    for platform_folder in os.listdir(path_to_results_folder):
        for json_file_name in os.listdir(os.path.join(path_to_results_folder, platform_folder)):
            encodings = [None, 'utf-16']
            for encoding in encodings:
                try:
                    with open(os.path.join(path_to_results_folder, platform_folder, json_file_name), "r", encoding=encoding) as file:
                        data = json.load(file)

                    for measurement in data["Measurements"]:
                        current = {}
                        current["Plotted"] = [False, False]
                        current["SummaryShown"] = [False, False]
                        current["Timestamp"] = os.path.split(path_to_results_folder)[1]
                        current["Platform"] = platform_folder
                        current["SampleName"] = json_file_name.removesuffix("_data.json")
                        current["MeasurementName"] = measurement["Name"]
                        current["DataPointsCount"] = measurement["DataPointsCount"]
                        current["DataPoints"] = measurement["DataPoints"]
                        current["Summary"] = measurement["Summary"]
                        result_data.append(current)
                    
                    break
                except Exception as e:
                    if encoding == encodings[-1]:
                        raise e
    
    def compare(a, b):
        if (a["MeasurementName"] == b["MeasurementName"]):
            if (a["Platform"] == b["Platform"]):
                if (a["SampleName"] == b["SampleName"]):
                    return 0
                elif (a["SampleName"] > b["SampleName"]):
                    return 1
                else:
                    return -1
            elif (a["Platform"] > b["Platform"]):
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

def selectDataToPlot(result_data: list[dict[str, Any]], graph_data: dict[str, Any]) -> Tuple[list[dict[str, Any]], dict[str, Any]]:
    max_length = {"Timestamp": 0, "MeasurementName": 0, "SampleName": 0, "Platform": 0}
    max_length["Index"] = len(str(len(result_data) - 1)) if len(str(len(result_data) - 1)) > len("Index") else len("Index")

    for entry in result_data:
        max_length["Timestamp"] = max(max_length["Timestamp"], len(entry["Timestamp"]))
        max_length["MeasurementName"] = max(max_length["MeasurementName"], len(entry["MeasurementName"]))
        max_length["Platform"] = max(max_length["Platform"], len(entry["Platform"]))
        max_length["SampleName"] = max(max_length["SampleName"], len(entry["SampleName"]))


    header_boundaries = {}
    header_boundaries["Timestamp"] = {}
    (header_boundaries["Timestamp"]["Prefix"], header_boundaries["Timestamp"]["Suffix"], _, _) = calculatePrefixAndSuffixWhitespaces(len("Timestamp"), max_length["Timestamp"])

    header_boundaries["MeasurementName"] = {}
    (header_boundaries["MeasurementName"]["Prefix"], header_boundaries["MeasurementName"]["Suffix"], _, _) = calculatePrefixAndSuffixWhitespaces(len("MeasurementName"), max_length["MeasurementName"])

    header_boundaries["Platform"] = {}
    (header_boundaries["Platform"]["Prefix"], header_boundaries["Platform"]["Suffix"], _, _) = calculatePrefixAndSuffixWhitespaces(len("Platform"), max_length["Platform"])

    header_boundaries["SampleName"] = {}
    (header_boundaries["SampleName"]["Prefix"], header_boundaries["SampleName"]["Suffix"], _, _) = calculatePrefixAndSuffixWhitespaces(len("SampleName"), max_length["SampleName"])

    header_str = "{}Index | Plotted | SummaryShown | {}Timestamp{} | {}MeasurementName{} | {}Platform{} | {}SampleName{}".format(
          " "*(len(str(len(result_data) - 1)) - len("Index")) if len(str(len(result_data) - 1)) > len("Index") else "",
          header_boundaries["Timestamp"]["Prefix"], header_boundaries["Timestamp"]["Suffix"], 
          header_boundaries["MeasurementName"]["Prefix"], header_boundaries["MeasurementName"]["Suffix"], 
          header_boundaries["Platform"]["Prefix"], header_boundaries["Platform"]["Suffix"],
          header_boundaries["SampleName"]["Prefix"], header_boundaries["SampleName"]["Suffix"])

    print(header_str)
    
    current_index = 0
    for entry in result_data:
        vb = {}
        vb["Timestamp"] = {}
        (_, _, vb["Timestamp"]["Prefix"], vb["Timestamp"]["Suffix"]) = calculatePrefixAndSuffixWhitespaces(max_length["Timestamp"], len(entry["Timestamp"]))
        
        vb["MeasurementName"] = {}
        (_, _, vb["MeasurementName"]["Prefix"], vb["MeasurementName"]["Suffix"]) = calculatePrefixAndSuffixWhitespaces(max_length["MeasurementName"], len(entry["MeasurementName"]))
        
        vb["Platform"] = {}
        (_, _, vb["Platform"]["Prefix"], vb["Platform"]["Suffix"]) = calculatePrefixAndSuffixWhitespaces(max_length["Platform"], len(entry["Platform"]))
        
        vb["SampleName"] = {}
        (_, _, vb["SampleName"]["Prefix"], vb["SampleName"]["Suffix"]) = calculatePrefixAndSuffixWhitespaces(max_length["SampleName"], len(entry["SampleName"]))

        print("{}{} |   {} {}   |    {}    {}    | {}{}{} | {}{}{} | {}{}{} | {}{}{} ".format(
            " "*(max_length["Index"] - len(str(current_index))), str(current_index),
            "1" if entry["Plotted"][0] else " ", "2" if entry["Plotted"][1] else " ",
            "1" if entry["SummaryShown"][0] else " ", "2" if entry["SummaryShown"][1] else " ",
            vb["Timestamp"]["Prefix"], entry["Timestamp"], vb["Timestamp"]["Suffix"],
            vb["MeasurementName"]["Prefix"], entry["MeasurementName"], vb["MeasurementName"]["Suffix"],
            vb["Platform"]["Prefix"], entry["Platform"], vb["Platform"]["Suffix"],
            vb["SampleName"]["Prefix"], entry["SampleName"], vb["SampleName"]["Suffix"]
        ))

        current_index += 1
    

    print("")
    print("-"*len(header_str))
    print("")
    print("plot/p [0, {}]        - Plot these data points on currently active graph (currently active graph #{}).".format(len(result_data) - 1, graph_data["ActiveGraph"] + 1))
    print("summary/s [0, {}]     - Show summary of row in table.".format(len(result_data) - 1))
    print("measurement/m [0, {}] - Toggle all plots and summaries for all samples in the measurement and platform combinations.".format(len(result_data) - 1))
    print("logarithmic/l        {}- Toggle logarithmic scale for active graph (currently active graph #{})".format(" "*(len(str(len(result_data) - 1))-1), graph_data["ActiveGraph"] + 1))
    print("clear/c              {}- Clear all data from the current active graph (currently active graph #{})".format(" "*(len(str(len(result_data) - 1))-1), graph_data["ActiveGraph"] + 1))
    print("graph/g [1, 3]       {}- Switch active graph (currently active graph #{})".format(" "*(len(str(len(result_data) - 1))-1), graph_data["ActiveGraph"] + 1))
    print("save [file_name]     {}- Save current set of graphs into svg (file_name is optional, if empty saved to TestScripts/Graphs/<timestamp>.png otherwise to TestScripts/Graphs/<file_name>.png)".format(" "*(len(str(len(result_data) - 1))-1)))
    print("quit/q               {}- Quit program".format(" "*(len(str(len(result_data) - 1))-1)))

    while True:
        error_msg = ""
        command = input("Please select a command: ")

        def getNumberFromInput(input: str, short_command: str, long_command: str, min: int, max: int) -> Tuple[int, str]:
            try:
                idx = int(input.removeprefix(long_command).removeprefix(short_command).strip())
                if idx >= min and idx < max:
                    return (idx, "")
                else:
                    return (None, "Index out of range")
            except:
                return (None, "Index could not be parsed")

        if command.startswith("plot ") or command.startswith("p "):
            (idx, error_msg) = getNumberFromInput(command, "p", "plot", 0, len(result_data))
            if idx != None:
                result_data[idx]["Plotted"][graph_data["ActiveGraph"]] = not result_data[idx]["Plotted"][graph_data["ActiveGraph"]]

        elif command.startswith("summary") or command.startswith("s "):
            (idx, error_msg) = getNumberFromInput(command, "s", "summary", 0, len(result_data))
            if idx != None:
                result_data[idx]["SummaryShown"][graph_data["ActiveGraph"]] = not result_data[idx]["SummaryShown"][graph_data["ActiveGraph"]]
                
        elif command.startswith("measurement") or command.startswith("m "):
            (idx, error_msg) = getNumberFromInput(command, "m", "measurement", 0, len(result_data))
            if idx != None:
                all_on = True
                for entry in result_data:
                    if entry["MeasurementName"] == result_data[idx]["MeasurementName"] and entry["Platform"] == result_data[idx]["Platform"]:
                        if (not entry["SummaryShown"][graph_data["ActiveGraph"]] or not entry["Plotted"][graph_data["ActiveGraph"]]):
                            all_on = False
                            break
                
                toggle_to_state = not all_on
                
                for entry in result_data:
                    if entry["MeasurementName"] == result_data[idx]["MeasurementName"] and entry["Platform"] == result_data[idx]["Platform"]:
                        entry["Plotted"][graph_data["ActiveGraph"]] = toggle_to_state
                        entry["SummaryShown"][graph_data["ActiveGraph"]] = toggle_to_state

        elif command.startswith("logarithmic") or command.startswith("l"):
            graph_data["LogarithmicScales"][graph_data["ActiveGraph"]] = not graph_data["LogarithmicScales"][graph_data["ActiveGraph"]]

        elif command.startswith("clear") or command.startswith("c"):
            for entry in result_data:
                entry["Plotted"][graph_data["ActiveGraph"]] = False
                entry["SummaryShown"][graph_data["ActiveGraph"]] = False

        elif command.startswith("graph") or command.startswith("g "):
            (idx, error_msg) = getNumberFromInput(command, "g", "graph", 1, 3)
            if idx != None:
                graph_data["ActiveGraph"] = idx - 1

        elif command.startswith("save"):
            graph_data["SaveStatus"]["Queued"] = True
            file_name = command.removeprefix("save").strip()
            if file_name != "":
                graph_data["SaveStatus"]["FileName"] = file_name if file_name.endswith(".png") else "{}.png".format(file_name)
            else:
                graph_data["SaveStatus"]["FileName"] = datetime.today().strftime('%Y-%m-%d_%H-%M-%S.png')

        elif command.startswith("quit") or command.startswith("q"):
            exit(0)

        else:
            error_msg = "Unknown command"



        if error_msg != "":
            print("Input not correct ({}). Please choose a correct command.".format(error_msg))
        else:
            break
    
    return (result_data, graph_data)

def plot(data: list[dict[str, Any]], graph_data: dict[str, Any]) -> None:

    if graph_data["SaveStatus"]["Queued"]:
        file_path = Path(os.path.join(os.path.realpath(__file__), "..", "Graphs", graph_data["SaveStatus"]["FileName"]))
        file_path = file_path.resolve()
        try:
            plt.savefig(file_path)
        except:
            print("Invalid file name: '{}'. Full file path would have been: '{}".format(graph_data["SaveStatus"]["FileName"], file_path))
            
        graph_data["SaveStatus"]["Queued"] = False
        graph_data["SaveStatus"]["FileName"] = ""
        return

    plt.close()
    fig = plt.figure(figsize=(22, 10)) # Set size in inches for full window (x, y)
    
    number_rows = 2
    number_cols = 4
    plt1 = fig.add_subplot(number_rows, number_cols, 1) # The last digit aka. index represents the position of the subplot
    plt2 = fig.add_subplot(number_rows, number_cols, 3) # in the grid specified by number_rows and number_cols.
    sum1 = fig.add_subplot(number_rows, number_cols, 5) # Index is the top left and is counted towards the right and
    sum2 = fig.add_subplot(number_rows, number_cols, 7) # automatically does line breaks if no further columns are available.
    plots = [plt1, plt2]
    plot_has_entry = [False, False]
    summaries = [sum1, sum2]
    summary_has_entry = [False, False]


    def doPlotsOrBars(data_or_summary: str):
        for i in range(0, len(plots) if data_or_summary == "Plotted" else len(summaries)):
            plots_or_bars = []
            long_labels = []
            short_labels = []
            current_idx = 0
            max_long_label_length = 0
            max_short_label_length = 0
            short_title = ""
            first_suffix = ""
            all_suffixes_are_equal = True
            for entry in data:
                if entry["Plotted" if data_or_summary == "Plotted" else "SummaryShown"][i]:

                    if data_or_summary == "Plotted":
                        plot_has_entry[i] = True
                    else:
                        summary_has_entry[i] = True

                    # If we only have one measurement of one platform in a subplot we can make it a lot prettier
                    # Thus see if we change suffixes, which is the MeasurementName and Platform
                    suffix = "_{}_{}".format(entry["MeasurementName"], entry["Platform"])
                    if first_suffix == "":
                        first_suffix = suffix
                        short_title = "{} - {}".format(entry["MeasurementName"], entry["Platform"])
                    else:
                        if first_suffix != suffix:
                            all_suffixes_are_equal = False
                    
                    # Populate all possible labels already as we don't want to go through all entries twice
                    long_labels.append("{}_{}_{}".format(entry["SampleName"], entry["MeasurementName"], entry["Platform"]))
                    short_labels.append(entry["SampleName"])

                    # Check for the max length of both labels so we can position the legend of the graph correctly
                    max_long_label_length = max(max_long_label_length, len(long_labels[-1]))
                    max_short_label_length = max(max_short_label_length, len(short_labels[-1]))

                    # The switch between plots and summaries is super hard-coded. Might be a good idea to make it more flexible, but for now its fine
                    if data_or_summary == "Plotted":
                        plots_or_bars.append(plots[i].plot(entry["DataPoints"])) # Add the line to the plot
                        plots[i].axes.get_xaxis().set_visible(False) # Hide the x axis, as we do our labeling in the legends
                        plots[i].axes.get_yaxis().set_major_formatter(matplttick.StrMethodFormatter('{x:,.0f} ns')) # Format the y axis so we can actually read it
                        plots[i].set_ylim(ymin=0, ymax=max(plots[i].get_ylim()[1], max(entry["DataPoints"]) * 1.1))
                    else:
                        # See the "Plotted" block for explanations
                        plots_or_bars.append(summaries[i].bar([current_idx], [entry["Summary"]["mean"]]))
                        summaries[i].errorbar([current_idx], [entry["Summary"]["mean"]], yerr=[math.sqrt(entry["Summary"]["variance"])], fmt="o", color="r") # Add the error bars by calculating the standard deviation with the variance
                        summaries[i].axes.get_xaxis().set_visible(False)
                        summaries[i].axes.get_yaxis().set_major_formatter(matplttick.StrMethodFormatter('{x:,.0f} ns'))
                        summaries[i].set_ylim(ymin=0, ymax=max(summaries[i].get_ylim()[1], entry["Summary"]["mean"] * 1.1))
                    current_idx += 1 # The bar chart needs an index as an x label, so we increment one here to have the bars in the correct order

            # Format and setup the legend
            if data_or_summary == "Plotted" and plot_has_entry[i]:
                if all_suffixes_are_equal:
                    plots[i].legend(short_labels, bbox_to_anchor=(1., 0., 1., 1.), loc='center left')
                    plots[i].set_title(short_title)
                else:
                    plots[i].legend(long_labels, bbox_to_anchor=(1., 0., 1., 1.), loc='center left')
                    plots[i].set_title("Various measurements")
            elif summary_has_entry[i]:
                if all_suffixes_are_equal:
                    summaries[i].legend(plots_or_bars, short_labels, bbox_to_anchor=(1., 0., 1., 1.), loc='center left')
                    summaries[i].set_title(short_title)
                else:
                    summaries[i].legend(plots_or_bars, long_labels, bbox_to_anchor=(1., 0., 1., 1.), loc='center left')
                    summaries[i].set_title("Various measurements")

    doPlotsOrBars("Plotted")
    doPlotsOrBars("Summary")

    for i in range(0, len(plots)):
        if graph_data["LogarithmicScales"][i]:
            y_lim_max = plots[i].get_ylim()[1]
            plots[i].set_yscale("log")
            plots[i].set_ylim(ymin=1, ymax=y_lim_max * 1.5)
            plots[i].axes.get_yaxis().set_major_formatter(matplttick.StrMethodFormatter('{x:,.0f} ns')) # Format the y axis so we can actually read it

    for i in range(0, len(summaries)):
        if graph_data["LogarithmicScales"][i]:
            y_lim_max = summaries[i].get_ylim()[1]
            summaries[i].set_yscale("log")
            summaries[i].set_ylim(ymin=1, ymax=y_lim_max * 1.5)
            summaries[i].axes.get_yaxis().set_major_formatter(matplttick.StrMethodFormatter('{x:,.0f} ns'))

    plt.show(block=False)

if __name__ == "__main__":
    path_to_results_folder = getPathToData()
    json_data = readJson(path_to_results_folder)

    graph_data = { "ActiveGraph": 0, "LogarithmicScales": [False, False], "SaveStatus": { "Queued": False, "FileName": "" } }
    while True:
        (json_data, graph_data) = selectDataToPlot(json_data, graph_data)
        plot(json_data, graph_data)

    # import ipdb; ipdb.set_trace()

