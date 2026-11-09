#include <stdio.h>
#include <string.h>

int main(void) {
    char s[2000];
    scanf("%s", s);
    int len = strlen(s);

    char parentStack[2000];   // 스택: 부모 노드 문자
    int  childCount[2000];    // 스택: 각 부모의 자식 수
    int  top = -1;

    int nodes = 0, leaves = 0, height = 0, degree = 0;
    int  cFound = 0;
    char cParent = 0;          // 0이면 부모 없음(루트)
    char cChildren[100];
    int  cChildN = 0;

    int expectingNode = 1;     // 1: 노드가 와야 함, 0: ',' 또는 ')' 가 와야 함
    int i = 0;

    while (i < len) {
        char c = s[i];

        if (expectingNode) {
            if (c < 'A' || c > 'Z') {
                printf("오류: 잘못된 입력입니다\n");
                return 1;
            }
            nodes++;

            /* ??? (1) 부모 문자: 스택이 비었으면 0, 아니면 스택 맨 위 문자 */
            char parent = ;

            if (top >= 0) childCount[top]++;

            int depth = top + 2;
            if (depth > height) height = depth;

            if (c == 'C') {
                cFound = 1;
                cParent = parent;
            }
            if (parent == 'C') {
                cChildren[cChildN++] = c;
            }

            i++;
            if (i < len && s[i] == '(') {
                /* ??? (2) 방금 읽은 노드 c를 부모로 push (카운터 0) */

                i++;
                expectingNode = 1;
            } else {
                /* ??? (3) 단말 노드 처리 + 다음엔 ',' 또는 ')' 가 와야 함 */

            }
        }
        else {
            if (c == ',') {
                if (top < 0) {
                    printf("오류: 구조 오류입니다\n");
                    return 1;
                }
                i++;
                expectingNode = 1;
            }
            else if (c == ')') {
                if (/* ??? (4) 오류 조건: 닫을 괄호가 짝이 없는 경우 */) {
                    printf("오류: 괄호가 맞지 않습니다\n");
                    return 1;
                }
                if (childCount[top] > degree) degree = childCount[top];
                top--;
                i++;
                expectingNode = 0;
            }
            else {
                printf("오류: 잘못된 입력입니다\n");
                return 1;
            }
        }
    }

    /* ??? (5) 끝까지 읽은 뒤 오류 조건: 닫히지 않은 괄호가 남았거나 노드가 하나도 없음 */
    if (/* ... */) {
        printf("오류: 올바른 트리가 아닙니다\n");
        return 1;
    }

    printf("전체 노드 수: %d\n", nodes);
    printf("단말 노드 수: %d\n", leaves);
    printf("비단말 노드 수: %d\n", nodes - leaves);
    printf("트리의 높이: %d\n", height);
    printf("트리의 차수: %d\n", degree);

    printf("C의 부모: ");
    if (!cFound) printf("C 없음\n");
    else if (cParent == 0) printf("없음(루트)\n");
    else printf("%c\n", cParent);

    printf("C의 자식: ");
    if (!cFound) printf("C 없음\n");
    else if (cChildN == 0) printf("없음(단말노드)\n");
    else {
        for (int k = 0; k < cChildN; k++) printf("%c ", cChildren[k]);
        printf("\n");
    }
    return 0;
}