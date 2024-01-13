#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
using namespace std;
long long int combination(int n, int k);
long long int factorial(int n);
// Function to read labels from a CSV file
vector<int> readLabelsFromCSV(const string& filename) {
    vector<int> labels;
    ifstream file(filename);

    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            int label;
            while (ss >> label) {
                labels.push_back(label);
                if (ss.peek() == ',' || ss.peek() == ' ') {
                    ss.ignore();
                }
            }
        }
        file.close();
    } else {
        cerr << "Error opening file: " << filename << endl;
    }

    return labels;
}


// Function to calculate Rand Index (RI)
double calculateRI(const vector<int>& trueLabels, const vector<int>& predictedLabels) {
    int agree = 0, total = 0;

    for (size_t i = 0; i < trueLabels.size(); ++i) {
        for (size_t j = i + 1; j < trueLabels.size(); ++j) {
            if ((trueLabels[i] == trueLabels[j] && predictedLabels[i] == predictedLabels[j]) ||
                (trueLabels[i] != trueLabels[j] && predictedLabels[i] != predictedLabels[j])) {
                agree++;
            }
            total++;
        }
    }

    return static_cast<double>(agree) / total;
}

// Function to calculate Hamming Index (HI)
double calculateHamingI(const vector<int>& trueLabels, const vector<int>& predictedLabels) {
    int disagree = 0;

    for (size_t i = 0; i < trueLabels.size(); ++i) {
        if (trueLabels[i] != predictedLabels[i]) {
            disagree++;
        }
    }

    return static_cast<double>(disagree) / trueLabels.size();
}

// Function to calculate Adjusted Rand Index (ARI)
vector<vector<int>> Contingency(const vector<int>& Mem1, const vector<int>& Mem2) {
    int maxMem1 = *max_element(Mem1.begin(), Mem1.end());
    int maxMem2 = *max_element(Mem2.begin(), Mem2.end());

    vector<vector<int>> Cont(maxMem1 + 1, vector<int>(maxMem2 + 1, 0));

    for (size_t i = 0; i < Mem1.size(); i++) {
        Cont[Mem1[i]][Mem2[i]]++;
    }

    return Cont;
}

double RandIndex(const vector<int>& c1, const vector<int>& c2, double& AR, double& RI, double& MI, double& HI) {
    // Form contingency matrix
    vector<vector<int>> C = Contingency(c1, c2);

    int n = c1.size();
    long int nis = 0, njs = 0;
    long int sum_n = 0;
    long int row_sum[C.size()];
    long int col_sum[C[0].size()];
    long int sum_row_sum =0, sum_col_sum =0;
    int contingency_power2 = 0;
    for (int j = 0; j < C[0].size(); j++) col_sum[j] =0;
    for (int j = 0; j < C.size(); j++) row_sum[j]=0;
    
    for (int i = 0; i < C.size(); i++) {
        //int rowSum = 0;
        //int colSum = 0;
        for (int j = 0; j < C[0].size(); j++) {
            sum_n += C[i][j];
            row_sum[i]+= C[i][j];
            col_sum[i]+= C[j][i];
          //  std::cout << "C[i][j]: " << C[i][j] << " i: " << i << " j: " <<j <<endl;
        }
    }
    for (int i = 0; i < C.size(); i++) {
    	row_sum[i] *= row_sum[i];
    	sum_row_sum += row_sum[i];
    }
    for (int i = 0; i < C[0].size(); i++) {
   // std::cout << "col_sum[i]: " << col_sum[i]  <<endl;
    	col_sum[i] *= col_sum[i];
    	sum_col_sum += col_sum[i];
    }
    nis = sum_row_sum;
    njs = sum_col_sum;
    
   for (int i = 0; i < C.size(); i++) {
        for (int j = 0; j < C[0].size(); j++) {
            contingency_power2 += C[i][j] * C[i][j];
        }
    }
    
//	std::cout << "nis: " << nis  <<endl;
//	std::cout << "njs: " << njs <<endl;
//	std::cout << "sum_n: " << sum_n <<endl;
    double t1 = combination(sum_n, 2);
 //   std::cout << "t1: " << t1 <<endl;
    int t2 = contingency_power2;
//    std::cout << "t2: " << t2 <<endl;
    int t3 = 0.5 * (nis + njs);
//std::cout << "t3: " << t3 <<endl;

	
    double nc = (sum_n * (std::pow(sum_n, 2) + 1) - ((sum_n + 1) * nis) - ((sum_n + 1) * njs) + (2 * (nis * njs) / sum_n)) / (2 * (sum_n - 1));
//	std::cout << "nc: " << nc <<endl;
	//nc=(n*(n**2+1)-(n+1)*nis-(n+1)*njs+2*(nis*njs)/n)/(2*(n-1))
    int A = t1 + t2 - t3;  // no. agreements
    int D = -t2 + t3;       // no. disagreements

    if (t1 == nc) {
        AR = 0;
    } else {
        AR = static_cast<double>(A - nc) / (t1 - nc);  // adjusted Rand - Hubert & Arabie 1985
    }

    RI = static_cast<double>(A) / t1;       // Rand 1971 - Probability of agreement
    MI = static_cast<double>(D) / t1;       // Mirkin 1970 - p(disagreement)
    HI = static_cast<double>(A - D) / t1;   // Hubert 1977 - p(agree) - p(disagree)

    return AR;
}

long long int combination(int n, int k) {
    if (k > n) {
        return 0;  // Invalid combination
    }

    // Ensure k is no more than half of n for optimization
    if (k > n / 2) {
        k = n - k;
    }

    long long int result = 1;

    for (int i = 1; i <= k; ++i) {
        result *= n--;
        result /= i;
    }

    return result;
}

void printVector(const vector<int>& vec, const string& name) {
    cout << name << ": ";
    for (const int& value : vec) {
        cout << value << " ";
    }
    cout << endl;
}

void adjustRanges(vector<int>& c1, vector<int>& c2) {
    // Find the minimum value in c2
    int minC2 = *min_element(c2.begin(), c2.end());

    // Adjust the values in c2 to align with c1
    for (int& value : c2) {
        value -= minC2;
    }
}
