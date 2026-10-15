#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define V 50
#define SPARSE_EDGES 100
#define DENSE_EDGES 600
#define MAX_WEIGHT 100
#define REPEAT 10000      /* 실행시간 측정 시 반복 횟수 (평균을 구하기 위함) */

typedef struct {
    int u, v, w;
} WEdge;

/* ---------------- Union-Find ---------------- */
int findRoot(int parent[], int x) {
    while (parent[x] != x) {
        parent[x] = parent[parent[x]];
        x = parent[x];
    }
    return x;
}

int unionSets(int parent[], int rnk[], int a, int b) {
    int ra = findRoot(parent, a);
    int rb = findRoot(parent, b);
    if (ra == rb) return 0;          /* 이미 같은 집합 = 사이클 */
    if (rnk[ra] < rnk[rb]) parent[ra] = rb;
    else if (rnk[ra] > rnk[rb]) parent[rb] = ra;
    else { parent[rb] = ra; rnk[ra]++; }
    return 1;
}

/* ---------------- 그래프 생성 ---------------- */
/* 1) 랜덤 신장 트리로 연결을 보장하고 2) 나머지 간선을 중복 없이 추가 */
void generateGraph(WEdge edges[], int edgeCount) {
    int used[V][V];
    int perm[V];
    memset(used, 0, sizeof(used));

    for (int i = 0; i < V; i++) perm[i] = i;
    for (int i = V - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = perm[i]; perm[i] = perm[j]; perm[j] = t;
    }

    int made = 0;
    for (int i = 1; i < V; i++) {
        int u = perm[i];
        int v = perm[rand() % i];
        used[u][v] = used[v][u] = 1;
        edges[made].u = u;
        edges[made].v = v;
        edges[made].w = rand() % MAX_WEIGHT + 1;
        made++;
    }

    while (made < edgeCount) {
        int u = rand() % V;
        int v = rand() % V;
        if (u == v) continue;
        if (used[u][v]) continue;
        used[u][v] = used[v][u] = 1;
        edges[made].u = u;
        edges[made].v = v;
        edges[made].w = rand() % MAX_WEIGHT + 1;
        made++;
    }
}

/* 생성된 그래프가 연결 그래프인지 확인 */
int isConnected(WEdge edges[], int edgeCount) {
    int parent[V], rnk[V];
    for (int i = 0; i < V; i++) { parent[i] = i; rnk[i] = 0; }
    int components = V;
    for (int i = 0; i < edgeCount; i++) {
        if (unionSets(parent, rnk, edges[i].u, edges[i].v)) components--;
    }
    return components == 1;
}

/* ---------------- 인접 리스트 ---------------- */
typedef struct ListNode {
    int vertex;
    int weight;
    struct ListNode *next;
} ListNode;

typedef struct {
    int n;
    ListNode *head[V];
} AdjList;

void listPushFront(AdjList *g, int u, int v, int w) {
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));
    node->vertex = v;
    node->weight = w;
    node->next = g->head[u];
    g->head[u] = node;
}

void buildList(AdjList *g, WEdge edges[], int edgeCount) {
    g->n = V;
    for (int i = 0; i < V; i++) g->head[i] = NULL;
    for (int i = 0; i < edgeCount; i++) {
        listPushFront(g, edges[i].u, edges[i].v, edges[i].w);
        listPushFront(g, edges[i].v, edges[i].u, edges[i].w);
    }
}

void freeList(AdjList *g) {
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

/* ---------------- 인접 행렬 (가중치 저장, 0이면 간선 없음) ---------------- */
typedef struct {
    int n;
    int w[V][V];
} AdjMatrix;

void buildMatrix(AdjMatrix *g, WEdge edges[], int edgeCount) {
    g->n = V;
    memset(g->w, 0, sizeof(g->w));
    for (int i = 0; i < edgeCount; i++) {
        g->w[edges[i].u][edges[i].v] = edges[i].w;
        g->w[edges[i].v][edges[i].u] = edges[i].w;
    }
}

/* ---------------- Kruskal (인접 리스트) ---------------- */
int cmpEdge(const void *a, const void *b) {
    const WEdge *x = (const WEdge *)a;
    const WEdge *y = (const WEdge *)b;
    return (x->w > y->w) - (x->w < y->w);
}

int kruskal(AdjList *g, int edgeCount, WEdge mst[], long long *cost) {
    /* 1. 인접 리스트에서 간선 수집 (무방향이라 u < v 인 것만) */
    WEdge *all = (WEdge *)malloc(sizeof(WEdge) * edgeCount);
    int m = 0;
    for (int u = 0; u < V; u++) {
        for (ListNode *cur = g->head[u]; cur != NULL; cur = cur->next) {
            if (u < cur->vertex) {
                all[m].u = u;
                all[m].v = cur->vertex;
                all[m].w = cur->weight;
                m++;
            }
        }
    }

    /* 2. 가중치 오름차순 정렬 */
    qsort(all, m, sizeof(WEdge), cmpEdge);

    /* 3. Union-Find로 사이클이 안 생기는 간선만 선택 */
    int parent[V], rnk[V];
    for (int i = 0; i < V; i++) { parent[i] = i; rnk[i] = 0; }

    int count = 0;
    *cost = 0;
    for (int i = 0; i < m && count < V - 1; i++) {
        if (unionSets(parent, rnk, all[i].u, all[i].v)) {
            mst[count++] = all[i];
            *cost += all[i].w;
        }
    }

    free(all);
    return count;
}

/* ---------------- Prim (인접 행렬) ---------------- */
int prim(AdjMatrix *g, WEdge mst[], long long *cost) {
    int key[V], parent[V], visited[V];
    for (int i = 0; i < V; i++) {
        key[i] = INT_MAX;
        parent[i] = -1;
        visited[i] = 0;
    }
    key[0] = 0;   /* 시작 정점 0 */

    int count = 0;
    *cost = 0;

    for (int iter = 0; iter < V; iter++) {
        /* MST에 아직 없는 정점 중 key가 가장 작은 정점 선택 */
        int u = -1;
        for (int v = 0; v < V; v++) {
            if (!visited[v] && (u == -1 || key[v] < key[u])) u = v;
        }
        if (u == -1 || key[u] == INT_MAX) break;

        visited[u] = 1;
        if (parent[u] != -1) {
            mst[count].u = parent[u];
            mst[count].v = u;
            mst[count].w = key[u];
            count++;
            *cost += key[u];
        }

        /* 새로 들어온 정점 u를 통해 key 갱신 */
        for (int v = 0; v < V; v++) {
            if (g->w[u][v] != 0 && !visited[v] && g->w[u][v] < key[v]) {
                key[v] = g->w[u][v];
                parent[v] = u;
            }
        }
    }
    return count;
}

/* ---------------- 시간 측정 / 출력 ---------------- */
double nowMicros(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

void printMst(WEdge mst[], int count) {
    printf("MST Edges (u - v : weight)\n");
    for (int i = 0; i < count; i++) {
        printf("  %2d - %2d : %3d", mst[i].u, mst[i].v, mst[i].w);
        if ((i + 1) % 4 == 0) printf("\n");
    }
    if (count % 4 != 0) printf("\n");
}

volatile long long sink = 0;   /* 반복 실행이 최적화로 제거되지 않도록 결과를 누적 */

long long runKruskalCase(const char *title, const char *graphName,
                         WEdge edges[], int edgeCount) {
    AdjList g;
    buildList(&g, edges, edgeCount);

    WEdge mst[V - 1];
    long long cost = 0;
    int mstCount = kruskal(&g, edgeCount, mst, &cost);   /* 결과 확보용 1회 실행 */

    long long resultCost = cost;

    double start = nowMicros();
    for (int r = 0; r < REPEAT; r++) {
        kruskal(&g, edgeCount, mst, &cost);
        sink += cost;
    }
    double end = nowMicros();
    double avgMicros = (end - start) / REPEAT;

    /* 메모리: 그래프 표현 + 보조 자료구조 + MST 결과 저장 공간 */
    int graphMem = (int)sizeof(AdjList) + 2 * edgeCount * (int)sizeof(ListNode);
    int auxMem = edgeCount * (int)sizeof(WEdge)     /* 정렬용 간선 배열 */
               + 2 * V * (int)sizeof(int);          /* Union-Find: parent, rank */
    int resultMem = (V - 1) * (int)sizeof(WEdge);

    printf("%s\n", title);
    printf("Vertices      : %d\n", V);
    printf("Edges         : %d\n", edgeCount);
    printf("MST Edges     : %d\n", mstCount);
    printf("MST Cost      : %lld\n", resultCost);
    printf("Memory        : %d Bytes\n", graphMem + auxMem + resultMem);
    printf("  (graph %d + auxiliary %d + result %d)\n", graphMem, auxMem, resultMem);
    printf("Execution Time: %.3f us (%d회 평균)\n", avgMicros, REPEAT);
    printMst(mst, mstCount);
    printf("\n");

    freeList(&g);
    return resultCost;
}

long long runPrimCase(const char *title, const char *graphName,
                      WEdge edges[], int edgeCount) {
    AdjMatrix g;
    buildMatrix(&g, edges, edgeCount);

    WEdge mst[V - 1];
    long long cost = 0;
    int mstCount = prim(&g, mst, &cost);

    long long resultCost = cost;

    double start = nowMicros();
    for (int r = 0; r < REPEAT; r++) {
        prim(&g, mst, &cost);
        sink += cost;
    }
    double end = nowMicros();
    double avgMicros = (end - start) / REPEAT;

    int graphMem = (int)sizeof(AdjMatrix);
    int auxMem = 3 * V * (int)sizeof(int);          /* key, parent, visited */
    int resultMem = (V - 1) * (int)sizeof(WEdge);

    printf("%s\n", title);
    printf("Vertices      : %d\n", V);
    printf("Edges         : %d\n", edgeCount);
    printf("MST Edges     : %d\n", mstCount);
    printf("MST Cost      : %lld\n", resultCost);
    printf("Memory        : %d Bytes\n", graphMem + auxMem + resultMem);
    printf("  (graph %d + auxiliary %d + result %d)\n", graphMem, auxMem, resultMem);
    printf("Execution Time: %.3f us (%d회 평균)\n", avgMicros, REPEAT);
    printMst(mst, mstCount);
    printf("\n");

    return resultCost;
}

int main(void) {
    unsigned int seed = (unsigned int)time(NULL);
    srand(seed);
    printf("Seed : %u\n", seed);
    printf("sizeof(ListNode)=%d, sizeof(AdjList)=%d, sizeof(AdjMatrix)=%d, sizeof(WEdge)=%d\n\n",
           (int)sizeof(ListNode), (int)sizeof(AdjList), (int)sizeof(AdjMatrix), (int)sizeof(WEdge));

    WEdge sparseEdges[SPARSE_EDGES];
    WEdge denseEdges[DENSE_EDGES];

    /* 연결 그래프가 아니면 다시 생성 (구조상 항상 연결이지만 확인 절차를 둠) */
    do { generateGraph(sparseEdges, SPARSE_EDGES); } while (!isConnected(sparseEdges, SPARSE_EDGES));
    do { generateGraph(denseEdges, DENSE_EDGES); } while (!isConnected(denseEdges, DENSE_EDGES));
    printf("Sparse graph connected : Yes\n");
    printf("Dense graph connected  : Yes\n\n");

    long long c1 = runKruskalCase("Case 1 : Sparse Graph - Adjacency List - Kruskal", "Sparse", sparseEdges, SPARSE_EDGES);
    long long c2 = runKruskalCase("Case 2 : Dense Graph - Adjacency List - Kruskal", "Dense", denseEdges, DENSE_EDGES);
    long long c3 = runPrimCase("Case 3 : Sparse Graph - Adjacency Matrix - Prim", "Sparse", sparseEdges, SPARSE_EDGES);
    long long c4 = runPrimCase("Case 4 : Dense Graph - Adjacency Matrix - Prim", "Dense", denseEdges, DENSE_EDGES);

    printf("===== MST 전체 비용 검증 =====\n");
    printf("Sparse : Kruskal %lld / Prim %lld -> %s\n", c1, c3, c1 == c3 ? "일치" : "불일치");
    printf("Dense  : Kruskal %lld / Prim %lld -> %s\n", c2, c4, c2 == c4 ? "일치" : "불일치");

    return 0;
}