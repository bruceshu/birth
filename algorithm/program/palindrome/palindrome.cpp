

#include <vector>
#include <iostream>
using namespace std;

vector<char> findLongestPalindrome(vector<char> v) {
	vector<char> src;
	vector<char> result;
	int center = 0;
	int longestPalindrome = 0;
	int rightSide = 0;
	int rightSideCenter = 0;

	//在字符串每个字符前后增加 ‘#’ 特殊符号，让字符串的长度成为奇数
	src.push_back('#');
	for(int i=0; i<v.size(); i++) {
		src.push_back(v[i]);
		src.push_back('#');
	}

	vector<int>palindromeSize(src.size(), 0);
	for(int i=0; i<src.size(); i++) {
		bool needCalc = true;

		if(i < rightSide) {
			int leftSide = 2 * rightSideCenter - i;
			palindromeSize[i] = palindromeSize[leftSide];
			if(i + palindromeSize[i] < rightSide) {
				needCalc = false;
			}
		}

		if(needCalc) {
			while((i-1-palindromeSize[i] >= 0) && (i+1+palindromeSize[i] <= src.size())) {
				if(src[i-1-palindromeSize[i]] == src[i+1+palindromeSize[i]]) {
					palindromeSize[i]++;
				} else {
					break;
				}
			}

			rightSide = i + palindromeSize[i];
			rightSideCenter = i;

			//获取最长回文并记录中心节点索引
			if(palindromeSize[i] > longestPalindrome) {
				longestPalindrome = palindromeSize[i];
				center = i;
			}

			if(i + palindromeSize[i] + 1 == src.size()) {
				break;
			}
		}		
	}

	cout << "输出中心节点,索引:" << center << ",长度:" << longestPalindrome << endl;

	//去掉特殊字符 ‘#’
	for(int i = center - palindromeSize[center] + 1; i<= center + palindromeSize[center] - 1; i+=2) {
		result.push_back(src[i]);
	}

	cout << "查看最长回文" << endl;
	for(int i=0; i<result.size(); ++i) {
		cout << result[i];
	}
	cout << endl;

	return result;
}

bool compareVector(vector<char> v1, vector<char> v2) {
	vector<char>::iterator itr1, itr2;

	if(v1.size() == 0 || v2.size() == 0) {
		return false;
	}

	if(v1.size() != v2.size()) {
		return false;
	}

	itr1 = v1.begin();
	itr2 = v2.begin();
	while(itr1 != v1.end() && *itr1 == *itr2) {
		++itr1;
		++itr2;
	}

	if(itr1 == v1.end()) {
		return true;
	} else {
		return false;
	}
}


int main(int argc, char *argv[]) {
	vector<vector<char> > v = {
		{'a','b','c','d','c','e','f'},
    	{'a','d','a','e','l','e','l','e'},
    	{'c','a','b','a','d','a','b','a','e'},
        {'a','a','a','a','b','c','d','e','f','g','f','e','d','c','b','a','a'},
        {'a','a','b','a'},
        {'a','a','a','a','a','a','a','a','a'}
    };

    vector<vector<char> > resultV = {
    	{'c','d','c'},
    	{'e','l','e','l','e'},
    	{'a','b','a','d','a','b','a'},
    	{'a','a','b','c','d','e','f','g','f','e','d','c','b','a','a'},
    	{'a','b','a'},
    	{'a','a','a','a','a','a','a','a','a'}
    };

    int count = 0;
    for(int i=0; i<v.size(); i++) {
    	if(compareVector(resultV[i], findLongestPalindrome(v[i]))) {
    		count++;
    	}
    }

    if(count / v.size()) {
    	cout << "通过所有用例" << endl;
    } else {
    	cout << "通过 " << (float)count * 100 / v.size() << "% 用例" << endl;
    }

    return 0;
}

