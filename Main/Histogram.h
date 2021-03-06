#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <filesystem>
#include <array>
#include <iterator>
#include <map>
#include <algorithm>
#include <iomanip>
#include <cstdlib>

#include <jsoncons/json.hpp>

//Create a map containing all occurrences of data
template <typename T>
std::map <T, size_t> MakeHistogram (const std::vector<T> & Data) {
	std::map <T, size_t> Histogram;
	for (const auto & Datum : Data) {
		Histogram[Datum]++;
	}
	return Histogram;
}

//Divide a std::map <T, size_t> in NBins intervals
//NOTE: If a constant function is given, histogram will collapse to single bin
template <typename T>
std::vector<size_t> MakeBins (const std::map <T, size_t> & Data, const size_t NBins) {
	std::vector<size_t> Bins;
	Bins.reserve(NBins);
	size_t NElem = 0;
	//Get Max element
	auto Max = std::end(Data);
	std::advance(Max, -1);
	//Width of a bin and limit of interval
	T Width = std::abs(Max->first-std::begin(Data)->first)/NBins;
	T Limit = std::begin(Data)->first+Width;
	//Range starts at first element and finishes at limit
	auto RangeBegin = std::begin(Data);
	auto RangeEnd = Data.upper_bound(Limit);
	while (RangeEnd != std::end(Data)) {
		while (RangeBegin != RangeEnd) {		
			NElem += RangeBegin -> second;
			++RangeBegin;
		}
		//Now RangeBegin == RangeEnd
		Limit += Width;
		RangeEnd = Data.upper_bound(Limit);
		Bins.push_back(NElem);
		NElem = 0;
	}
	//Last elements are not pushed back
	//Check to avoid problems
	if (Bins.size () == NBins) {
		Bins[NBins-1]++;
	}
	else {
		while (RangeBegin != RangeEnd) {		
			NElem += RangeBegin -> second;
			++RangeBegin;
			}
		Bins.push_back (NElem);
	}
	return Bins;
}

template <typename T>
std::vector<T> MakeProbabilities (std::vector<size_t> Bins) {
	std::vector<T> Probabilities;
	Probabilities.reserve(Bins.size());
	T TotalOccurrences = 0;
	for (auto & Bin : Bins) {
		TotalOccurrences += static_cast<T> (Bin);
	}
	for (long unsigned int I = 0; I != Bins.size(); ++I) {
		Probabilities.push_back(static_cast<T> (Bins[I])/TotalOccurrences);
	}
	return Probabilities;
}

template <typename T>
jsoncons::wojson Histograms (const std::array<std::vector<T>, 3>& Data, const size_t NBins) {
    jsoncons::wojson ProcessedData;
    std::map <T, size_t> Histogram;
    std::vector <T> Probabilities;
    std::vector <size_t> Bins;
    const std::array<std::wstring, 3> XYZ = {L"X", L"Y", L"Z"};
    for (short int I = 0; I != 3; ++I) {
        Histogram = MakeHistogram(Data[I]);
        ProcessedData.insert_or_assign(XYZ[I], jsoncons::json_object_arg);
        ProcessedData.insert_or_assign(L"NBins", NBins);
        ProcessedData[XYZ[I]].insert_or_assign(L"Min", std::begin(Histogram)->first);
        ProcessedData[XYZ[I]].insert_or_assign(L"Max", std::rbegin(Histogram)->first);
        Bins = MakeBins<T>(Histogram, NBins);
        Probabilities = MakeProbabilities<T>(Bins);
        ProcessedData[XYZ[I]].insert_or_assign(L"FrequencyHistogram", jsoncons::json_array_arg);
        ProcessedData[XYZ[I]].insert_or_assign(L"ProbabilityHistogram", jsoncons::json_array_arg);
        for (unsigned long int J = 0; J != Bins.size(); ++J) {
            ProcessedData[XYZ[I]][L"FrequencyHistogram"].push_back(Bins[J]);
            ProcessedData[XYZ[I]][L"ProbabilityHistogram"].push_back( Probabilities[J]);
        }
    }
    return ProcessedData;
}

template <typename T>
jsoncons::wojson Histograms (std::vector<T>& Data, const size_t NBins) {
    jsoncons::wojson ProcessedData;
    std::map <T, size_t> Histogram;
    std::vector <T> Probabilities;
    std::vector <size_t> Bins;
	Histogram = MakeHistogram(Data);
	ProcessedData.insert_or_assign(L"Data", jsoncons::json_object_arg);
	ProcessedData.insert_or_assign(L"NBins", NBins);
	ProcessedData[L"Data"].insert_or_assign(L"Min", std::begin(Histogram)->first);
	ProcessedData[L"Data"].insert_or_assign(L"Max", std::rbegin(Histogram)->first);
	Bins = MakeBins<T>(Histogram, NBins);
	Probabilities = MakeProbabilities<T>(Bins);
	ProcessedData[L"Data"].insert_or_assign(L"FrequencyHistogram", jsoncons::json_array_arg);
	ProcessedData[L"Data"].insert_or_assign(L"ProbabilityHistogram", jsoncons::json_array_arg);
	for (unsigned long int J = 0; J != Bins.size(); ++J) {
		ProcessedData[L"Data"][L"FrequencyHistogram"].push_back(Bins[J]);
		ProcessedData[L"Data"][L"ProbabilityHistogram"].push_back( Probabilities[J]);
	}
    return ProcessedData;
}

#endif