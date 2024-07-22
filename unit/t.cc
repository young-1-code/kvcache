#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <iostream>
#include <algorithm>
#include <set>
#include <unordered_set>

using namespace std;
class Solution {
public:
    int longestConsecutive(vector<int>& nums) {
        int ret = 0;
        unordered_set<int> us;

        for(auto n : nums){
            us.insert(n);
        }

        for(auto n : us){

            if(us.count(n-1) == 0){ //最小节点
                
                int cnt=0, cur=n;

                while(us.count(cur)){
                    cnt++;
                    cur++;
                }
                ret = max(ret, cnt);
            }
        }

        return ret;
    }
};

int main(void)
{
    Solution s;

    vector<int> v={100,4,200,1,3,2};
    int ret = s.longestConsecutive(v);
    cout << "ret = " << ret << endl;

    return 0;
}