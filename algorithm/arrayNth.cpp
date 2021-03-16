/* 寻找两个数组中第N大的数。
 最优算法：采用类二分查找算法，以递归的形式实现。时间复杂度O(log2(m+n)),空间复杂度O(1)
 author：bruce 2021-3-15
*/

#include <vector>
#include <iostream>
using namespace std;

#define MIN(a,b) (a) < (b) ? (a) : (b)
#define MAX(a,b) (a) > (b) ? (a) : (b)
#define LEN 20   //每一个数组最大的长度
#define TOTAL 6 //用例个数
#define K_NUM 3 //最大值的个数，每一组数据分别测试3个最大值

int findKth(vector<int>a, int beginA, int endA, vector<int>b, int beginB, int endB, int k) {
    if(a.size() == 0 && b.size() == 0) {
        cout << "两个数组都为空" << endl;
        return -1;
    }

    //默认情况A数组的长度小于等于B数组长度。
    if(endA - beginA > endB - beginB) {
        return findKth(b, beginB, endB, a, beginA, endA, k);
    }

    //A数组偏移到头后，B数组第k-1个数为第k个最大值
    if(beginA > endA) {
        return b[beginB+k-1];
    }
    
    if(k == 1) {
        return MAX(a[beginA], b[beginB]);
    }

    int k1 = 0;
    int k2 = 0;
    k1 = MIN(k/2, endA-beginA+1);
    k2 = k-k1;

    if(a[beginA+k1-1] > b[beginB+k2-1]) {
        return findKth(a, beginA+k1, endA, b, beginB, beginB+k2-1, k-k1);
    } else if(a[beginA+k1-1] < b[beginB+k2-1]) {
        return findKth(a, beginA, beginA+k1-1, b, beginB+k2, endB, k-k2);
    } else {
        return a[beginA+k1-1];
    }
}


int main(int argc, char* args[]) {
    //测试用例
    int A[TOTAL][LEN] = {
        {5,4,3},
        {8},
        {9,7,5,3},
        {3,2,1},
        {10,9,8,7,6,5,3,2},
        {10,8,6,4,2}
    };

    int B[TOTAL][LEN] = {
        {5,4,3},
        {9,7,5,3},
        {8},
        {6,5,4},
        {},
        {11,9,7,5,3}
    };

    int k[K_NUM] = {1,2,3};
    int result[TOTAL][K_NUM] = {
        {5,5,4},
        {9,8,7},
        {9,8,7},
        {6,5,4},
        {10,9,8},
        {11,10,9}
    };

    int k2[K_NUM] = {3,4,5};
    int result2[TOTAL][K_NUM] = {
        {4,4,3},{7,5,3},{7,5,3},{4,3,2},{8,7,6},{9,8,7}
    };


    vector<vector<int> >arrayA;
    vector<vector<int> >arrayB;
    int count = 0;

    vector<int> tmp;
    int i = 0;
    int j = 0;

    for(i=0; i<TOTAL; ++i) {
        j = 0;
        while(A[i][j] != 0) {
            tmp.push_back(A[i][j]);
            ++j;
        }
        arrayA.push_back(tmp);
        tmp.clear();

        //重置索引,开始B数组数据写入vector B
        j = 0;
        while(B[i][j] != 0) {
            tmp.push_back(B[i][j]);
            ++j;
        }
        arrayB.push_back(tmp);
        tmp.clear();
    }
 
    for(int i=0; i<arrayA.size(); ++i) {
        for(int j=0; j<K_NUM; ++j) {
            if(result2[i][j] == findKth(arrayA[i], 0, arrayA[i].size()-1, arrayB[i], 0, arrayB[i].size()-1, k2[j])) {
                ++count;
            }
        }
    }

    for(int i=0; i<arrayA.size(); ++i) {
        for(int j=0; j<K_NUM; ++j) {
            if(result[i][j] == findKth(arrayA[i], 0, arrayA[i].size()-1, arrayB[i], 0, arrayB[i].size()-1, k[j])) {
                ++count;
            }
        }
    }

    if(count == TOTAL * K_NUM * 2) {
        cout << "success!" << endl;
    } else {
        cout << "成功通过:" << count *100 / (TOTAL*K_NUM) / 2 << "\% 用例" << endl; 
    }

    return 0;
}