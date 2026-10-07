#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATA_COUNT 100
#define SEARCH_COUNT 50
#define MAX_VALUE 1000

/* ---------------- 배열 ---------------- */
int arrayData[DATA_COUNT];
int arrayLen = 0;

int arrayContains(int *arr, int n, int value, long *comparisons) {
    for (int i = 0; i < n; i++) {
        (*comparisons)++;
        if (arr[i] == value) return 1;
    }
    return 0;
}

/* ---------------- BST ---------------- */
typedef struct Node {
    int value;
    struct Node *left, *right;
} Node;

Node* newNode(int value) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->value = value;
    n->left = n->right = NULL;
    return n;
}

/* 삽입: 중복이면 inserted=0, 신규면 inserted=1 */
Node* bstInsert(Node *root, int value, long *comparisons, int *inserted) {
    if (root == NULL) { *inserted = 1; return newNode(value); }

    Node *current = root;
    while (1) {
        (*comparisons)++;
        if (value == current->value) { *inserted = 0; return root; }
        else if (value < current->value) {
            if (current->left == NULL) { current->left = newNode(value); *inserted = 1; return root; }
            current = current->left;
        } else {
            if (current->right == NULL) { current->right = newNode(value); *inserted = 1; return root; }
            current = current->right;
        }
    }
}

int bstSearch(Node *root, int key, long *comparisons) {
    Node *current = root;
    while (current != NULL) {
        (*comparisons)++;
        if (key == current->value) return 1;
        else if (key < current->value) current = current->left;
        else current = current->right;
    }
    return 0;
}

/* 노드 수 기준 높이: 빈 트리 0, 루트만 1 */
int bstHeight(Node *n) {
    if (n == NULL) return 0;
    int l = bstHeight(n->left);
    int r = bstHeight(n->right);
    return 1 + (l > r ? l : r);
}

void freeBst(Node *n) {
    if (n == NULL) return;
    freeBst(n->left);
    freeBst(n->right);
    free(n);
}

/* ---------------- AVL 트리 ---------------- */
typedef struct AVLNode {
    int value;
    int height;   
    struct AVLNode *left, *right;
} AVLNode;

AVLNode* newAVLNode(int value) {
    AVLNode *n = (AVLNode *)malloc(sizeof(AVLNode));
    n->value = value;
    n->height = 1;
    n->left = n->right = NULL;
    return n;
}

int heightOf(AVLNode *n) { return n ? n->height : 0; }
int maxInt(int a, int b) { return a > b ? a : b; }

AVLNode* rotateRight(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *t2 = x->right;

    x->right = y;
    y->left = t2;

    y->height = 1 + maxInt(heightOf(y->left), heightOf(y->right));
    x->height = 1 + maxInt(heightOf(x->left), heightOf(x->right));

    return x;
}

AVLNode* rotateLeft(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *t2 = y->left;

    y->left = x;
    x->right = t2;

    x->height = 1 + maxInt(heightOf(x->left), heightOf(x->right));
    y->height = 1 + maxInt(heightOf(y->left), heightOf(y->right));

    return y;
}

/* 삽입: 값 비교는 현재 노드와 value를 비교할 때만 증가
   높이 계산/균형 확인/회전 판단은 비교 횟수에 포함하지 않는다 */
AVLNode* avlInsert(AVLNode *node, int value, long *comparisons, int *inserted) {
    if (node == NULL) { *inserted = 1; return newAVLNode(value); }

    (*comparisons)++;
    if (value == node->value) {
        *inserted = 0;
        return node;
    } else if (value < node->value) {
        node->left = avlInsert(node->left, value, comparisons, inserted);
    } else {
        node->right = avlInsert(node->right, value, comparisons, inserted);
    }

    node->height = 1 + maxInt(heightOf(node->left), heightOf(node->right));
    int balance = heightOf(node->left) - heightOf(node->right);

    /* LL */
    if (balance > 1 && node->left != NULL && value < node->left->value)
        return rotateRight(node);
    /* RR */
    if (balance < -1 && node->right != NULL && value > node->right->value)
        return rotateLeft(node);
    /* LR */
    if (balance > 1 && node->left != NULL && value > node->left->value) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    /* RL */
    if (balance < -1 && node->right != NULL && value < node->right->value) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

int avlSearch(AVLNode *root, int key, long *comparisons) {
    AVLNode *current = root;
    while (current != NULL) {
        (*comparisons)++;
        if (key == current->value) return 1;
        else if (key < current->value) current = current->left;
        else current = current->right;
    }
    return 0;
}

void freeAvl(AVLNode *n) {
    if (n == NULL) return;
    freeAvl(n->left);
    freeAvl(n->right);
    free(n);
}

/* ---------------- 메인 ---------------- */
int main(void) {
    srand((unsigned int)time(NULL));

    Node *bstRoot = NULL;
    AVLNode *avlRoot = NULL;

    long arrayConstructionComparisons = 0;
    long bstConstructionComparisons = 0;
    long avlConstructionComparisons = 0;

    int distinctCount = 0;
    int duplicateCount = 0;

    int rawValues[DATA_COUNT];

    printf("===== 생성된 100개의 정수 =====\n");
    for (int i = 0; i < DATA_COUNT; i++) {
        int value = rand() % (MAX_VALUE + 1);
        rawValues[i] = value;
        printf("%d ", value);
        if ((i + 1) % 10 == 0) printf("\n");

        long arrCmp = 0, bstCmp = 0, avlCmp = 0;
        int isNew = !arrayContains(arrayData, arrayLen, value, &arrCmp);
        arrayConstructionComparisons += arrCmp;
        if (isNew) arrayData[arrayLen++] = value;

        int insertedBst = 0, insertedAvl = 0;
        bstRoot = bstInsert(bstRoot, value, &bstCmp, &insertedBst);
        bstConstructionComparisons += bstCmp;

        avlRoot = avlInsert(avlRoot, value, &avlCmp, &insertedAvl);
        avlConstructionComparisons += avlCmp;

        if (isNew) distinctCount++; else duplicateCount++;
    }
    printf("\n");

    printf("===== 삽입 결과 =====\n");
    printf("실제로 저장된 서로 다른 값의 수 : %d\n", distinctCount);
    printf("중복으로 삽입되지 않은 값의 수   : %d\n\n", duplicateCount);

    printf("Construction\n");
    printf("  Array comparisons : %ld\n", arrayConstructionComparisons);
    printf("  BST comparisons   : %ld\n", bstConstructionComparisons);
    printf("  AVL comparisons   : %ld\n\n", avlConstructionComparisons);

    printf("Structure\n");
    printf("  Array length : %d\n", arrayLen);
    printf("  BST height   : %d\n", bstHeight(bstRoot));
    printf("  AVL height   : %d\n\n", heightOf(avlRoot));

    /* 탐색 대상 50개 생성 */
    int searchKeys[SEARCH_COUNT];
    for (int i = 0; i < SEARCH_COUNT; i++) {
        searchKeys[i] = rand() % (MAX_VALUE + 1);
    }

    printf("===== 생성된 50개의 탐색 대상 =====\n");
    for (int i = 0; i < SEARCH_COUNT; i++) {
        printf("%d ", searchKeys[i]);
        if ((i + 1) % 10 == 0) printf("\n");
    }
    printf("\n");

    long totalSeqComparisons = 0;
    long totalBstComparisons = 0;
    long totalAvlComparisons = 0;

    printf("===== 개별 탐색 결과 (50건) =====\n\n");

    for (int i = 0; i < SEARCH_COUNT; i++) {
        int key = searchKeys[i];
        long seqCmp = 0, bstCmp = 0, avlCmp = 0;

        int seqFound = arrayContains(arrayData, arrayLen, key, &seqCmp);
        int bstFound = bstSearch(bstRoot, key, &bstCmp);
        int avlFound = avlSearch(avlRoot, key, &avlCmp);

        totalSeqComparisons += seqCmp;
        totalBstComparisons += bstCmp;
        totalAvlComparisons += avlCmp;

        printf("Search Key : %d\n\n", key);
        printf("Sequential Search\n");
        printf("Result      : %s\n", seqFound ? "Found" : "Not Found");
        printf("Comparisons : %ld\n\n", seqCmp);
        printf("BST Search\n");
        printf("Result      : %s\n", bstFound ? "Found" : "Not Found");
        printf("Comparisons : %ld\n\n", bstCmp);
        printf("AVL Search\n");
        printf("Result      : %s\n", avlFound ? "Found" : "Not Found");
        printf("Comparisons : %ld\n\n", avlCmp);
        printf("--------------------------------------------------\n\n");
    }

    double seqAvg = (double)totalSeqComparisons / SEARCH_COUNT;
    double bstAvg = (double)totalBstComparisons / SEARCH_COUNT;
    double avlAvg = (double)totalAvlComparisons / SEARCH_COUNT;

    printf("===== 통계 =====\n");
    printf("Searches : %d\n\n", SEARCH_COUNT);

    printf("Sequential Search\n");
    printf("  Total comparisons   : %ld\n", totalSeqComparisons);
    printf("  Average comparisons : %.2f\n\n", seqAvg);

    printf("BST Search\n");
    printf("  Total comparisons   : %ld\n", totalBstComparisons);
    printf("  Average comparisons : %.2f\n\n", bstAvg);

    printf("AVL Search\n");
    printf("  Total comparisons   : %ld\n", totalAvlComparisons);
    printf("  Average comparisons : %.2f\n", avlAvg);

    freeBst(bstRoot);
    freeAvl(avlRoot);

    return 0;
}