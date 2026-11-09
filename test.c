#include <stdio.h>
#include <string.h>

int main(void) {
    char s[2000];
    scanf("%s", s);

    int childCount[2000];   // 스택: 각 부모의 "지금까지 센 자식 수"
    int top = -1;
    int nodes = 0, height = 0, degree = 0;

    for (int i = 0; s[i]; i++) {
        char c = s[i];

        if (c >= 'A' && c <= 'Z') {
            nodes++;
            if (top >= 0) {
                childCount[top]++;
            }
            int depth = top+1;
            if (depth > height) height = depth;
        }
        else if (c == '(') {
            top++;
            childCount[top] = 0;
        }
        else if (c == ')') {
            if (childCount[top] > degree) degree = childCount[top];
            top--;
        }
       
        /* ',' 는 아무것도 안 해도 됨 */
    }

    printf("노드 %d, 높이 %d, 차수 %d\n", nodes, height, degree);
    return 0;
}
/*
1. A (B(C,D), E)    
2. A -> 그럼 node + 1 해주고   depth = 0  다음 
3. ( -> 그럼 top에다가  +1을 해줘 childcount[1]= 0이 됨 
4. B -> 그럼 node +1 에다가 top이 이제 0 이기 때문에 childcount[0]+1 depth = 1이됨   ---> top = 0 depth = 1 childcount[0] = 1
5. ( -> 그럼top에다가 +1을 해줘 그러면  -> top = 1 chidcount[1] = 0
6. C -> 그럼 node + 1  childcount[1] + 1 -> childcount[1] = 1     ---> depth = 2  top = 1 depth = 2 childcount[1] = 1
7. D -> 그럼 node + 1 childcount[1] + 1 -> childcount[1] = 2     ---> depth = 2  top = 1 depth = 2 childcount[1] = 2
8. ) -> 그럼 childcount[1] = 2 degree 0  , top--            -------------> degree =2 top = 0 
9. E가 -> 그럼 node + 1 childcount [0] +1  depth = 1   -----> top =0 depth = 1 childcount[0] = 2
10. ) -> 그럼 childcount[0] = 2 degree 2 , degree = 2 top = -1
*/