#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 2000
#define MAX_NODES 2000

char treeStr[MAX_LEN];
int treeLen, parsePos;
int parseOk;
char parseError[100];

typedef struct Node {
    char data;
    struct Node *left, *right;
} Node;

Node* newNode(char c) {
    Node *n = (Node *)malloc(sizeof(Node));
    if (n == NULL) {
        printf("메모리 할당 실패\n");
        exit(1);
    }
    n->data = c;
    n->left = n->right = NULL;
    return n;
}

Node* parseTree(int isRoot) {
    if (parsePos >= treeLen) {
        parseOk = 0; strcpy(parseError, "입력이 예상보다 일찍 끝났습니다");
        return NULL;
    }
    char c = treeStr[parsePos];
    if (c < 'A' || c > 'Z') {
        parseOk = 0; sprintf(parseError, "잘못된 문자 '%c'", c);
        return NULL;
    }
    if (isRoot && c != 'A') {
        parseOk = 0; strcpy(parseError, "루트 노드는 A여야 합니다");
        return NULL;
    }
    parsePos++;

    Node *node = newNode(c);

    if (parsePos < treeLen && treeStr[parsePos] == '(') {
        parsePos++;
        if (parsePos < treeLen && treeStr[parsePos] != ',' && treeStr[parsePos] != ')') {
            node->left = parseTree(0);
            if (!parseOk) return NULL;
        }
        if (parsePos < treeLen && treeStr[parsePos] == ',') {
            parsePos++;
            if (parsePos < treeLen && treeStr[parsePos] != ')') {
                node->right = parseTree(0);
                if (!parseOk) return NULL;
            }
        }
        if (parsePos < treeLen && treeStr[parsePos] == ')') {
            parsePos++;
        } else {
            parseOk = 0; strcpy(parseError, "괄호가 맞지 않습니다");
            return NULL;
        }
    }
    return node;
}

void printTree(Node *n, char *prefix, int isLast, int isRoot) {
    if (n == NULL) return;

    if (isRoot) printf("%c\n", n->data);
    else printf("%s+---%c\n", prefix, n->data);

    char nextPrefix[MAX_LEN];
    snprintf(nextPrefix, MAX_LEN, "%s%s", prefix, isRoot ? "" : (isLast ? "    " : "|   "));

    if (n->left) printTree(n->left, nextPrefix, n->right == NULL, 0);
    if (n->right) printTree(n->right, nextPrefix, 1, 0);
}

/* ---------------- 반복적(iterative) 순회 ---------------- */

/* 전위 순회: 스택 1개, 오른쪽 먼저 push -> 왼쪽이 먼저 pop됨 */
void preorderIterative(Node *root) {
    if (root == NULL) return;

    Node *stack[MAX_NODES];
    int top = -1;
    stack[++top] = root;

    printf("Preorder  : ");
    while (top >= 0) {
        Node *node = stack[top--];
        printf("%c ", node->data);

        if (node->right != NULL) stack[++top] = node->right;
        if (node->left != NULL) stack[++top] = node->left;
    }
    printf("\n");
}

/* 중위 순회: 왼쪽으로 계속 내려가며 push, 더 못 가면 pop해서 방문 후 오른쪽으로 */
void inorderIterative(Node *root) {
    Node *stack[MAX_NODES];
    int top = -1;
    Node *current = root;

    printf("Inorder   : ");
    while (current != NULL || top >= 0) {
        while (current != NULL) {
            stack[++top] = current;
            current = current->left;
        }
        current = stack[top--];
        printf("%c ", current->data);
        current = current->right;
    }
    printf("\n");
}

/* 후위 순회: 스택 2개 사용. 스택1에서 뽑은 걸 스택2에 쌓고,
   왼쪽을 나중에 push해서 스택1에서 먼저 나오게 만든 뒤,
   스택2를 통째로 pop하면 Left->Right->Root 순서가 된다 */
void postorderIterative(Node *root) {
    if (root == NULL) return;

    Node *stack1[MAX_NODES], *stack2[MAX_NODES];
    int top1 = -1, top2 = -1;

    stack1[++top1] = root;

    while (top1 >= 0) {
        Node *node = stack1[top1--];
        stack2[++top2] = node;

        if (node->left != NULL) stack1[++top1] = node->left;
        if (node->right != NULL) stack1[++top1] = node->right;
    }

    printf("Postorder : ");
    while (top2 >= 0) {
        printf("%c ", stack2[top2--]->data);
    }
    printf("\n");
}

void freeTree(Node *n) {
    if (n == NULL) return;
    freeTree(n->left);
    freeTree(n->right);
    free(n);
}

void loadTreeString(const char *input) {
    int j = 0;
    for (int i = 0; input[i]; i++) {
        if (!isspace((unsigned char)input[i])) treeStr[j++] = input[i];
    }
    treeStr[j] = '\0';
    treeLen = j;
}

int main(void) {
    char inputBuffer[MAX_LEN];
    printf("괄호형 이진트리를 입력하세요 \n (예: A(B(D,E),C(,F))): ");
    if (fgets(inputBuffer, MAX_LEN, stdin) == NULL) {
        printf("오류: 입력이 없습니다\n");
        return 1;
    }

    loadTreeString(inputBuffer);

    if (treeLen == 0) {
        printf("오류: 트리가 비어 있습니다\n");
        return 1;
    }

    parsePos = 0;
    parseOk = 1;
    Node *root = parseTree(1);

    if (parseOk && parsePos != treeLen) {
        parseOk = 0;
        strcpy(parseError, "구조 오류: 입력 뒤에 남는 문자가 있습니다");
    }
    if (!parseOk) {
        printf("오류: %s\n", parseError);
        return 1;
    }

    printf("\n입력된 이진트리 구조\n\n");
    printTree(root, "", 1, 1);

    printf("\n순회 결과\n\n");
    preorderIterative(root);
    inorderIterative(root);
    postorderIterative(root);

    freeTree(root);
    return 0;
}