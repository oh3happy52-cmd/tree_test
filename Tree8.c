// 과제 - 07: 인접 행렬과 인접 리스트의 성능 비교


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define V 100
#define SPARSE_EDGES 100
#define DENSE_EDGES 4000

typedef struct {
    int u, v;
} Edge;

/* ---------------- 인접 행렬 ---------------- */
typedef struct {
    int n;
    int matrix[V][V];
} AdjMatrix;

void matrixInit(AdjMatrix *g) {
    g->n = V;
    memset(g->matrix, 0, sizeof(g->matrix));
}

void matrixAddEdge(AdjMatrix *g, int u, int v) {
    g->matrix[u][v] = 1;
    g->matrix[v][u] = 1;
}

/* 연결 여부 확인: matrix[u][v] 하나만 확인 */
int matrixConnected(AdjMatrix *g, int u, int v, int *cmp) {
    *cmp = 1;
    return g->matrix[u][v];
}

/* 인접 정점 찾기: matrix[u][0] ~ matrix[u][99] 전부 확인 */
int matrixAdjacent(AdjMatrix *g, int u, int out[], int *count) {
    int cmp = 0;
    *count = 0;
    for (int j = 0; j < V; j++) {
        cmp++;
        if (g->matrix[u][j] == 1) out[(*count)++] = j;
    }
    return cmp;
}

/* 삽입: 존재 여부 확인 1회 후 양쪽에 표시 */
int matrixInsert(AdjMatrix *g, int u, int v) {
    int cmp = 1;
    if (g->matrix[u][v] == 0) {
        g->matrix[u][v] = 1;
        g->matrix[v][u] = 1;
    }
    return cmp;
}

/* 삭제: 존재 여부 확인 1회 후 양쪽 표시 제거 */
int matrixDelete(AdjMatrix *g, int u, int v) {
    int cmp = 1;
    if (g->matrix[u][v] == 1) {
        g->matrix[u][v] = 0;
        g->matrix[v][u] = 0;
    }
    return cmp;
}

/* ---------------- 인접 리스트 ---------------- */
typedef struct ListNode {
    int vertex;
    struct ListNode *next;
} ListNode;

typedef struct {
    int n;
    ListNode *head[V];
    int nodeCount;      /* 현재 할당된 리스트 노드 수 (메모리 계산용) */
} AdjList;

void listInit(AdjList *g) {
    g->n = V;
    g->nodeCount = 0;
    for (int i = 0; i < V; i++) g->head[i] = NULL;
}

/* 비교 횟수 계산 없이 맨 앞에 노드 추가 (그래프 초기 생성용) */
void listPushFront(AdjList *g, int u, int v) {
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));
    node->vertex = v;
    node->next = g->head[u];
    g->head[u] = node;
    g->nodeCount++;
}

void listAddEdge(AdjList *g, int u, int v) {
    listPushFront(g, u, v);
    listPushFront(g, v, u);
}

/* 연결 여부 확인: u의 리스트를 v를 찾을 때까지 확인 */
int listConnected(AdjList *g, int u, int v, int *cmp) {
    *cmp = 0;
    ListNode *cur = g->head[u];
    while (cur != NULL) {
        (*cmp)++;
        if (cur->vertex == v) return 1;
        cur = cur->next;
    }
    return 0;
}

/* 인접 정점 찾기: u의 리스트 노드를 전부 확인 */
int listAdjacent(AdjList *g, int u, int out[], int *count) {
    int cmp = 0;
    *count = 0;
    ListNode *cur = g->head[u];
    while (cur != NULL) {
        cmp++;
        out[(*count)++] = cur->vertex;
        cur = cur->next;
    }
    return cmp;
}

/* 삽입: 존재 여부 확인을 위해 u의 리스트 전체를 훑고, 없으면 양쪽 맨 앞에 추가 */
int listInsert(AdjList *g, int u, int v) {
    int cmp = 0;
    ListNode *cur = g->head[u];
    while (cur != NULL) {
        cmp++;
        if (cur->vertex == v) return cmp;   /* 이미 있으면 삽입 안 함 */
        cur = cur->next;
    }
    listAddEdge(g, u, v);
    return cmp;
}

/* u의 리스트에서 v 노드를 찾아 삭제 (확인한 노드 수를 cmp에 누적) */
void listRemoveOne(AdjList *g, int u, int v, int *cmp) {
    ListNode *prev = NULL;
    ListNode *cur = g->head[u];
    while (cur != NULL) {
        (*cmp)++;
        if (cur->vertex == v) {
            if (prev != NULL) prev->next = cur->next;
            else g->head[u] = cur->next;
            free(cur);
            g->nodeCount--;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

/* 삭제: 무방향이라 양쪽 리스트에서 각각 찾아서 지움 */
int listDelete(AdjList *g, int u, int v) {
    int cmp = 0;
    listRemoveOne(g, u, v, &cmp);
    listRemoveOne(g, v, u, &cmp);
    return cmp;
}

void listFree(AdjList *g) {
    for (int i = 0; i < V; i++) {
        ListNode *cur = g->head[i];
        while (cur != NULL) {
            ListNode *next = cur->next;
            free(cur);
            cur = next;
        }
        g->head[i] = NULL;
    }
}

/* ---------------- 그래프 생성 ---------------- */
/* 자기 자신 연결 X, 중복 간선 X 인 랜덤 간선 count개 생성
   (중복 검사용 used 배열은 임시 배열이므로 메모리 측정 대상이 아님) */

void generateEdges(Edge edges[], int count) {
    int used[V][V];
    memset(used, 0, sizeof(used));

    int made = 0;
    while (made < count) {
        int u = rand() % V;
        int v = rand() % V;
        if (u == v) continue;
        if (used[u][v]) continue;
        used[u][v] = used[v][u] = 1;
        edges[made].u = u;
        edges[made].v = v;
        made++;
    }
}

/* ---------------- 테스트 대상 선택 ----------------
   같은 그래프(희소 또는 밀집)에서는 행렬/리스트가 같은 정점 쌍을 사용한다 */
   
typedef struct {
    int insU, insV;   /* 삽입할 간선 (현재 그래프에 없는 간선) */
    int delU, delV;   /* 삭제할 간선 (현재 그래프에 있는 간선) */
    int conU, conV;   /* 연결 여부를 확인할 정점 쌍 */
    int adjV;         /* 인접 정점을 출력할 정점 */
} TestPlan;

TestPlan makePlan(Edge edges[], int edgeCount) {
    int exists[V][V];
    memset(exists, 0, sizeof(exists));
    for (int i = 0; i < edgeCount; i++) {
        exists[edges[i].u][edges[i].v] = 1;
        exists[edges[i].v][edges[i].u] = 1;
    }

    TestPlan p;

    do {
        p.insU = rand() % V;
        p.insV = rand() % V;
    } while (p.insU == p.insV || exists[p.insU][p.insV]);

    int k = rand() % edgeCount;
    p.delU = edges[k].u;
    p.delV = edges[k].v;

    do {
        p.conU = rand() % V;
        p.conV = rand() % V;
    } while (p.conU == p.conV);

    p.adjV = rand() % V;
    return p;
}

void printPlan(const char *name, TestPlan p) {
    printf("[%s] 선택된 정점\n", name);
    printf("  삽입할 간선 (없던 간선)   : (%d, %d)\n", p.insU, p.insV);
    printf("  삭제할 간선 (있던 간선)   : (%d, %d)\n", p.delU, p.delV);
    printf("  연결 여부 확인 정점 쌍    : (%d, %d)\n", p.conU, p.conV);
    printf("  인접 정점 출력 대상 정점  : %d\n\n", p.adjV);
}

void printAdjacent(int vertex, int out[], int count) {
    printf("Adjacent vertices of %d (%d개) :", vertex, count);
    for (int i = 0; i < count; i++) printf(" %d", out[i]);
    printf("\n");
}

/* ---------------- 케이스 실행 ---------------- */
void runMatrixCase(const char *title, Edge edges[], int edgeCount, TestPlan p) {
    AdjMatrix g;
    matrixInit(&g);
    for (int i = 0; i < edgeCount; i++) matrixAddEdge(&g, edges[i].u, edges[i].v);

    int memory = (int)sizeof(AdjMatrix);

    int adj[V], adjCount, connCmp, adjCmp, insCmp, delCmp;
    int connected = matrixConnected(&g, p.conU, p.conV, &connCmp);
    adjCmp = matrixAdjacent(&g, p.adjV, adj, &adjCount);
    insCmp = matrixInsert(&g, p.insU, p.insV);
    delCmp = matrixDelete(&g, p.delU, p.delV);

    printf("%s\n", title);
    printf("Memory                         : %d Bytes\n", memory);
    printf("Edge insertion comparisons     : %d\n", insCmp);
    printf("Edge deletion comparisons      : %d\n", delCmp);
    printf("Connectivity check comparisons : %d (%s)\n", connCmp, connected ? "연결됨" : "연결 안 됨");
    printf("Adjacent vertices comparisons  : %d\n", adjCmp);
    printAdjacent(p.adjV, adj, adjCount);
    printf("\n");
}

void runListCase(const char *title, Edge edges[], int edgeCount, TestPlan p) {
    AdjList g;
    listInit(&g);
    for (int i = 0; i < edgeCount; i++) listAddEdge(&g, edges[i].u, edges[i].v);

    /* 메모리는 연산(삽입/삭제)을 하기 전, 초기 생성 직후 기준으로 측정 */
    int memory = (int)sizeof(AdjList) + g.nodeCount * (int)sizeof(ListNode);

    int adj[V], adjCount, connCmp, adjCmp, insCmp, delCmp;
    int connected = listConnected(&g, p.conU, p.conV, &connCmp);
    adjCmp = listAdjacent(&g, p.adjV, adj, &adjCount);
    insCmp = listInsert(&g, p.insU, p.insV);
    delCmp = listDelete(&g, p.delU, p.delV);

    printf("%s\n", title);
    printf("Memory                         : %d Bytes\n", memory);
    printf("Edge insertion comparisons     : %d\n", insCmp);
    printf("Edge deletion comparisons      : %d\n", delCmp);
    printf("Connectivity check comparisons : %d (%s)\n", connCmp, connected ? "연결됨" : "연결 안 됨");
    printf("Adjacent vertices comparisons  : %d\n", adjCmp);
    printAdjacent(p.adjV, adj, adjCount);
    printf("\n");

    listFree(&g);
}

int main(void) {
    unsigned int seed = (unsigned int)time(NULL);
    srand(seed);
    printf("Seed : %u\n", seed);
    printf("sizeof(ListNode) = %d, sizeof(AdjList) = %d, sizeof(AdjMatrix) = %d\n\n",
           (int)sizeof(ListNode), (int)sizeof(AdjList), (int)sizeof(AdjMatrix));

    Edge sparseEdges[SPARSE_EDGES];
    Edge denseEdges[DENSE_EDGES];
    generateEdges(sparseEdges, SPARSE_EDGES);
    generateEdges(denseEdges, DENSE_EDGES);

    TestPlan sparsePlan = makePlan(sparseEdges, SPARSE_EDGES);
    TestPlan densePlan = makePlan(denseEdges, DENSE_EDGES);

    printPlan("Sparse Graph (정점 100, 간선 100)", sparsePlan);
    printPlan("Dense Graph (정점 100, 간선 4000)", densePlan);

    runMatrixCase("Case 1 : Sparse Graph - Adjacency Matrix", sparseEdges, SPARSE_EDGES, sparsePlan);
    runListCase("Case 2 : Sparse Graph - Adjacency List", sparseEdges, SPARSE_EDGES, sparsePlan);
    runMatrixCase("Case 3 : Dense Graph - Adjacency Matrix", denseEdges, DENSE_EDGES, densePlan);
    runListCase("Case 4 : Dense Graph - Adjacency List", denseEdges, DENSE_EDGES, densePlan);

    return 0;
}