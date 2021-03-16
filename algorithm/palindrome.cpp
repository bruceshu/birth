/* 寻找字符串中，最长回文字符串。
 方法：统计每个字符的回文串长度。
 最优算法：利用回文的对称性，记录右边界，减少统计次数.平均算法复杂度：时间O(n/2),空间(n/2);最坏算法复杂度：时间O(n),空间(n)
 author：bruce 2021-3-15
*/

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

	// 初始化一个数组，记录每个字符的回文串长度的一半
	vector<int>palindromeSize(src.size(), 0);

	for(int i=0; i<src.size(); i++) {
		bool needCalc = true;

		//字符在回文右边界的范围内，采用回文的对称性可知后面字符的回文长度。到达边界后，需要重新计算右边界的范围。
		if(i < rightSide) {
			int leftSide = 2 * rightSideCenter - i;
			palindromeSize[i] = palindromeSize[leftSide];

			//当该字符索引加上该字符的回文串长度小于右边界，则不需要计算右边界。当字符索引+回文长度=右边界，则需要重新计算右边界
			if(i + palindromeSize[i] < rightSide) {
				needCalc = false;
			}
		}

		if(needCalc) {
			//记录每个字符的回文串长度
			while((i-1-palindromeSize[i] >= 0) && (i+1+palindromeSize[i] <= src.size())) {
				if(src[i-1-palindromeSize[i]] == src[i+1+palindromeSize[i]]) {
					palindromeSize[i]++;
				} else {
					break;
				}
			}

			//记录右边界和右边界中心
			rightSide = i + palindromeSize[i];
			rightSideCenter = i;

			//获取最长回文并记录中心节点索引
			if(palindromeSize[i] > longestPalindrome) {
				longestPalindrome = palindromeSize[i];
				center = i;
			}

			//当前回文串到达字符串末尾，则表示找到最长回文串，退出循环
			if(i + palindromeSize[i] + 1 == src.size()) {
				break;
			}
		}		
	}

	//去掉特殊字符 ‘#’
	for(int i = center - palindromeSize[center] + 1; i<= center + palindromeSize[center] - 1; i+=2) {
		result.push_back(src[i]);
	}

#if 0 //测试查看回文
	cout << "查看最长回文" << endl;
	for(int i=0; i<result.size(); ++i) {
		cout << result[i];
	}
	cout << endl;
#endif

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
	//测试用例
	vector<vector<char> > v = {
		{'a','b','c','d','c','e','f'},
    	{'a','d','a','e','l','e','l','e'},
    	{'c','a','b','a','d','a','b','a','e'},
        {'a','a','a','a','b','c','d','e','f','g','f','e','d','c','b','a','a'},
        {'a','a','b','a'},
        {'a','a','a','a','a','a','a','a','a'}
    };

	//测试用例对应的结果
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
    	cout << "通过 " << count * 100 / v.size() << "\% 用例" << endl;
    }

    return 0;
}

