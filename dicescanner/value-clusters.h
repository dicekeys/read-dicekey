#include <math.h>
#include <vector>
#include <list>
#include <algorithm>


class ValueCluster {
	public:
	std::vector<double> samples;
	double median;

	ValueCluster(double firstSample) {
		samples.push_back(firstSample);
	}

	void addSample(double sample) {
		samples.push_back(sample);
		// Re-sort list
		for (auto i = samples.size() - 2; i > 0 && samples[i+1] < samples[i]; i--) {
			std::swap(samples[i], samples[i-1]);
		}

		if (samples.size() % 2 == 1) {
			median = samples[samples.size()/2];
		} else {
			auto c = samples.size()/2;
			median = (samples[c] + samples[c-1])/2;
		}

	}
};

class ValueClusters {
	double proximityThreshold;

	public:
	std::list<ValueCluster> clusters;

	ValueClusters(double proximityThreshold) {
		this->proximityThreshold = proximityThreshold;
	}

	void addSample(double sample) {
		for (auto it = clusters.begin(); it != clusters.end(); ++it) {
			if (abs(sample - it->median) < proximityThreshold) {
				// The sample is within close enough proximity to the mean for this
				// cluster to include it in the mean
				it->addSample(sample);
				return;
			} else {
				if (sample < it->median) {
					// The sample comes before this cluster,
					// so we need to create a new cluster for it
					clusters.insert(it, ValueCluster(sample));
					return;
				}
			}
		}
		// The sample comes after all of the existing clusters,
		// and so should in a new cluster at the end of the list.
		if (clusters.empty()) {
			clusters.push_back(ValueCluster(sample));
		}
	}
};

