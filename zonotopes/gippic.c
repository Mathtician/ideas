#include <stdio.h>

// Sets to Minkowski sum:
//   Cycles (4) and sign changes (2) of (0, 0, 0, 1):
//     (0, 0, 0, 1), ..., (0, 0, 0, 5) x8
//   Even (first 3: 4) and all (last: 2) sign changes (8) of (1, 1, 1, 1):
//     (1, 1, 1, 1), ..., (5, 5, 5, 5) x8
//   Half of perms and sign changes (12) of (0, 0, sqrt(2), sqrt(2)):
//     (0, 0, 1, 1), ..., (0, 0, 7, 7) x12
//   Given by v1, v2, and v3, respectively
// Max coord 46 - leave 1 margin for bucket fill, g is 95^4

// i = vector index
// j = position along vector
// k = coordinate index
// s = segment length to apply perms/sign changes
// g indexed by x, y, z, w
// gFlat indexed by n
// v indexed by i, j, k
// vOffsets indexed by i, j
// u and verts indexed by i, k, (unnamed)
// Sqrt(2) ring is a+b*sqrt(2), pointed to by p, q
// bBox indexed by k, (unnamed)

// Remaining entries filled in initVs()

#define I1 8
#define J1 5

int v1[I1][J1][4] =
  {{{0, 0, 0, +1}},
   {{0, 0, 0, -1}},
   {{0, 0, +1, 0}},
   {{0, 0, -1, 0}},
   {{0, +1, 0, 0}},
   {{0, -1, 0, 0}},
   {{+1, 0, 0, 0}},
   {{-1, 0, 0, 0}}};

int v1Offsets[I1*J1];

#define I2 8
#define J2 5

int v2[I2][J2][4] =
  {{{+1, +1, +1, +1}},
   {{+1, +1, +1, -1}},
   {{+1, -1, -1, +1}},
   {{+1, -1, -1, -1}},
   {{-1, +1, -1, +1}},
   {{-1, +1, -1, -1}},
   {{-1, -1, +1, +1}},
   {{-1, -1, +1, -1}}};

int v2Offsets[I2*J2];

#define I3 12
#define J3 7

int v3[I3][J3][4] =
  {{{0, 0, +1, +1}},
   {{0, 0, +1, -1}},
   {{0, +1, 0, +1}},
   {{0, +1, 0, -1}},
   {{0, -1, -1, 0}},
   {{0, +1, -1, 0}},
   {{+1, 0, 0, +1}},
   {{+1, 0, 0, -1}},
   {{-1, 0, -1, 0}},
   {{-1, 0, +1, 0}},
   {{-1, -1, 0, 0}},
   {{+1, -1, 0, 0}}};
   
int v3Offsets[I3*J3];

int u1[8][4][2] =
  {{{0, 0}, {0, 0}, {0, 0}, {+1, 0}},
   {{0, 0}, {0, 0}, {0, 0}, {-1, 0}},
   {{0, 0}, {0, 0}, {+1, 0}, {0, 0}},
   {{0, 0}, {0, 0}, {-1, 0}, {0, 0}},
   {{0, 0}, {+1, 0}, {0, 0}, {0, 0}},
   {{0, 0}, {-1, 0}, {0, 0}, {0, 0}},
   {{+1, 0}, {0, 0}, {0, 0}, {0, 0}},
   {{-1, 0}, {0, 0}, {0, 0}, {0, 0}}};

int u2[8][4][2] =
  {{{+1, 0}, {+1, 0}, {+1, 0}, {+1, 0}},
   {{+1, 0}, {+1, 0}, {+1, 0}, {-1, 0}},
   {{+1, 0}, {-1, 0}, {-1, 0}, {+1, 0}},
   {{+1, 0}, {-1, 0}, {-1, 0}, {-1, 0}},
   {{-1, 0}, {+1, 0}, {-1, 0}, {+1, 0}},
   {{-1, 0}, {+1, 0}, {-1, 0}, {-1, 0}},
   {{-1, 0}, {-1, 0}, {+1, 0}, {+1, 0}},
   {{-1, 0}, {-1, 0}, {+1, 0}, {-1, 0}}};

int u3[12][4][2] =
  {{{0, 0}, {0, 0}, {0, +1}, {0, +1}},
   {{0, 0}, {0, 0}, {0, +1}, {0, -1}},
   {{0, 0}, {0, +1}, {0, 0}, {0, +1}},
   {{0, 0}, {0, +1}, {0, 0}, {0, -1}},
   {{0, 0}, {0, -1}, {0, -1}, {0, 0}},
   {{0, 0}, {0, +1}, {0, -1}, {0, 0}},
   {{0, +1}, {0, 0}, {0, 0}, {0, +1}},
   {{0, +1}, {0, 0}, {0, 0}, {0, -1}},
   {{0, -1}, {0, 0}, {0, -1}, {0, 0}},
   {{0, -1}, {0, 0}, {0, +1}, {0, 0}},
   {{0, -1}, {0, -1}, {0, 0}, {0, 0}},
   {{0, +1}, {0, -1}, {0, 0}, {0, 0}}};

int evenPerms[12][4] =
  {{0, 1, 2, 3}, {0, 2, 3, 1}, {0, 3, 1, 2},
   {1, 0, 3, 2}, {1, 2, 0, 3}, {1, 3, 2, 0},
   {2, 0, 1, 3}, {2, 1, 3, 0}, {2, 3, 0, 1},
   {3, 0, 2, 1}, {3, 1, 0, 2}, {3, 2, 1, 0}};

int cyclicPerms[4][4] =
  {{0, 1, 2, 3}, {1, 2, 3, 0}, {2, 3, 0, 1}, {3, 0, 1, 2}};

int choosePerms[6][4] =
  {{0, 1, 2, 3}, {1, 2, 3, 0}, {2, 3, 0, 1},
   {3, 0, 1, 2}, {0, 2, 1, 3}, {3, 1, 2, 0}};

int bBox[2][4];

const int N = 95*95*95*95;

char g[95][95][95][95];
char *gFlat = (char*)g;

#define numIco1Sections 1
int ico1Sections[numIco1Sections] = {24};
#define numIco1Verts 24
int ico1Verts[numIco1Verts][4][2] =
  // All permutations of:
  {{{0, 1}, {0, 1}, {0, 0}, {0, 0}}}; // 6*4=24

#define numRico1Sections 1
int rico1Sections[numRico1Sections] = {96};
#define numRico1Verts 96
int rico1Verts[numRico1Verts][4][2] =
  // All permutations of:
  {{{0, 2}, {0, 1}, {0, 1}, {0, 0}}}; // 12*8=96

#define numRico2Sections 2
int rico2Sections[numRico2Sections] = {64, 32};
#define numRico2Verts 96
int rico2Verts[numRico2Verts][4][2] =
  // All permutations of:
  {{{3, 0}, {1, 0}, {1, 0}, {1, 0}},  // 4*16=64
   {{2, 0}, {2, 0}, {2, 0}, {0, 0}}}; // 4*8=32

#define numIco2Sections 2
int ico2Sections[numIco2Sections] = {16, 8};
#define numIco2Verts 24
int ico2Verts[numIco2Verts][4][2] = 
  // All permutations of:
  {{{1, 0}, {1, 0}, {1, 0}, {1, 0}},  // 1*16=16
   {{2, 0}, {0, 0}, {0, 0}, {0, 0}}}; // 4*2=8

void minkowski(int* offset, int jMax, char place){
  char tmpPlace = place*2;
  for (int x = bBox[0][0]; x < bBox[1][0]; x++) {
    for (int y = bBox[0][1]; y < bBox[1][1]; y++) {
      for (int z = bBox[0][2]; z < bBox[1][2]; z++) {
	for (int w = bBox[0][3]; w < bBox[1][3]; w++) {
	  int n = ((x*95+y)*95+z)*95+w;
	  if (gFlat[n]&place) {
	    for (int j = 0; j < jMax; j++) {
	      gFlat[n+offset[j]] |= tmpPlace;
	    }
	  }
	}
      }
    }
  }
  int offsetFromMid = offset[jMax-1]+N/2;
  int offsetCoords[4] =
    {(offsetFromMid/(95*95*95))-47,
     ((offsetFromMid/(95*95))%95)-47,
     ((offsetFromMid/95)%95)-47,
     (offsetFromMid%95)-47};
  for (int k = 0; k < 4; k++) {
    if (offsetCoords[k] < 0) {
      bBox[0][k] += offsetCoords[k];
      if (bBox[0][k] < 0) {
	bBox[0][k] = 0;
      }
    } else {
      bBox[1][k] += offsetCoords[k];
      if (bBox[1][k] >= 95) {
	bBox[1][k] = 95;
      }
    }
  }
  for (int x = bBox[0][0]; x < bBox[1][0]; x++) {
    for (int y = bBox[0][1]; y < bBox[1][1]; y++) {
      for (int z = bBox[0][2]; z < bBox[1][2]; z++) {
	for (int w = bBox[0][3]; w < bBox[1][3]; w++) {
	  int n = ((x*95+y)*95+z)*95+w;
	  if (gFlat[n]&tmpPlace) {
	    gFlat[n] &= ~tmpPlace;
	    gFlat[n] |= place;
	  }
	}
      }
    }
  }
}

void minkowskiAll(int* offsets, int iMax, int jMax, char place) {
  for (int i = 0; i < iMax; i++) {
    minkowski(offsets+i*jMax, jMax, place);
    printf("Done with %d\n", i);
  }
}

/*
  void checkErrors() {
  char line[93];
  FILE *binFile = fopen("../python/gidpixhi/bin.txt", "r");
  for (int x = 0; x < 93; x++) {
  for (int y = 0; y < 93; y++) {
  for (int z = 0; z < 93; z++) {
  int foo = fscanf(binFile, "%s", line);
  int w1 = 0;
  while (w1 < 93 && line[w1] == '0') w1++;
  int w2 = 0;
  while (w2 < 93 && g[x+1][y+1][z+1][w2+1] == 0) w2++;
  if (w1 != w2) {
  printf("%d %d %d %d %d\n", x, y, z, w1, w2);
  }
  }
  }
  }
  fclose(binFile);
  }
*/

void initVs() {
  for (int i = 0; i < I1; i++) {
    for (int j = 1; j < J1; j++) {
      for (int k = 0; k < 4; k++) {
	v1[i][j][k] = v1[i][0][k]*(j+1);
      }
    }
  }
  for (int i = 0; i < I2; i++) {
    for (int j = 1; j < J2; j++) {
      for (int k = 0; k < 4; k++) {
	v2[i][j][k] = v2[i][0][k]*(j+1);
      }
    }
  }
  for (int i = 0; i < I3; i++) {
    for (int j = 1; j < J3; j++) {
      for (int k = 0; k < 4; k++) {
	v3[i][j][k] = v3[i][0][k]*(j+1);
      }
    }
  }
}

int getOffset(int *vRow) {
  return ((vRow[0]*95+vRow[1])*95+vRow[2])*95+vRow[3];
}

void initOffsets() {
  for (int i = 0; i < I1; i++) {
    for (int j = 0; j < J1; j++) {
      v1Offsets[i*J1+j] = getOffset(v1[i][j]);
    }
  }
  for (int i = 0; i < I2; i++) {
    for (int j = 0; j < J2; j++) {
      v2Offsets[i*J2+j] = getOffset(v2[i][j]);
    }
  }
  for (int i = 0; i < I3; i++) {
    for (int j = 0; j < J3; j++) {
      v3Offsets[i*J3+j] = getOffset(v3[i][j]);
    }
  }
}

int sqrt2Sign(int a, int b) {
  int c = a*a-2*b*b;
  if (c < 0) {
    return (b > 0) ? 1 : -1;
  }
  if (c > 0) {
    return (a > 0) ? 1 : -1;
  }
  return 0;
}

int dotProdSign(int p[4][2], int q[4][2]) {
  int a = 0;
  int b = 0;
  for (int k = 0; k < 4; k++) {
    a += p[k][0]*q[k][0]+2*p[k][1]*q[k][1];
    b += p[k][0]*q[k][1]+p[k][1]*q[k][0];
  }
  return sqrt2Sign(a, b);
}

void fillSignChanges(int p[][4][2], int n, int s, int kChange) {
  for (int i = s; i < 2*s; i++) {
    for (int k = 0; k < 4; k++) {
      p[n+i][k][0] = p[n+i-s][k][0];
      p[n+i][k][1] = p[n+i-s][k][1];
    }
    p[n+i][kChange][0] *= -1;
    p[n+i][kChange][1] *= -1;
  }
}

void fillPerms(int p[][4][2], int n, int s, int perms[][4], int numPerms) {
  for (int i = s; i < numPerms*s; i++) {
    for (int k = 0; k < 4; k++) {
      int permIdx = perms[i/s][k];
      p[n+i][k][0] = p[n+i%s][permIdx][0];
      p[n+i][k][1] = p[n+i%s][permIdx][1];
    }
  }
}

void fillAllSignChanges(int starts[], int numSections, int verts[][4][2], int sections[]) {
  int start = 0;
  for (int section = 0; section < numSections; section++) {
    for (int k = 0; k < 4; k++) {
      verts[start][k][0] = verts[section][k][0];
      verts[start][k][1] = verts[section][k][1];
    }
    starts[section] = start;
    start += sections[section];
  }
  for (int section = 0; section < numSections; section++) {
    start = starts[section];
    int s = 1;
    for (int k = 0; k < 4; k++) {
      if (verts[start][k][0] != 0 || verts[start][k][1] != 0) {
	fillSignChanges(verts, start, s, k);
	s *= 2;
      }
    }
  }
}

void initVerts() {
  int ico1Starts[numIco1Sections];
  fillAllSignChanges(ico1Starts, numIco1Sections, ico1Verts, ico1Sections);
  fillPerms(ico1Verts, ico1Starts[0], 4, choosePerms, 6);
  int rico1Starts[numRico1Sections];
  fillAllSignChanges(rico1Starts, numRico1Sections, rico1Verts, rico1Sections);
  fillPerms(rico1Verts, rico1Starts[0], 8, evenPerms, 12);
  int rico2Starts[numRico2Sections];
  fillAllSignChanges(rico2Starts, numRico2Sections, rico2Verts, rico2Sections);
  fillPerms(rico2Verts, rico2Starts[0], 16, cyclicPerms, 4);
  fillPerms(rico2Verts, rico2Starts[1], 8, cyclicPerms, 4);
  int ico2Starts[numIco2Sections];
  fillAllSignChanges(ico2Starts, numIco2Sections, ico2Verts, ico2Sections);
  fillPerms(ico2Verts, ico2Starts[1], 2, cyclicPerms, 4);
}

void initMinkowski(int offset, char place) {
  gFlat[offset] |= place;
  int coords[4] =
    {offset/(95*95*95),
     (offset/(95*95))%95,
     (offset/95)%95,
     offset%95};
  /*
  printf("Starting cell at %d %d %d %d\n",
	 coords[0]-47, coords[1]-47, coords[2]-47, coords[3]-47);
  */
  for (int k = 0; k < 4; k++) {
    bBox[0][k] = coords[k];
    bBox[1][k] = coords[k]+1;
  }
}

#define cellBuf 64

void makeCell(char place, int vert[4][2]) {
  //int segsAdded = 0;
  int cellOffset = N/2;
  /*
  printf("Starting cell %d %d %d %d %d %d %d %d\n",
	 vert[0][0], vert[0][1], vert[1][0], vert[1][1],
	 vert[2][0], vert[2][1], vert[3][0], vert[3][1]);
  */
  for (int i = 0; i < I1; i++) {
    if (dotProdSign(vert, u1[i]) == 1) {
      cellOffset += v1Offsets[J1*i+(J1-1)];
      /*
      printf("Added offset %d %d %d %d\n",
	     v1[i][J1-1][0], v1[i][J1-1][1],
	     v1[i][J1-1][2], v1[i][J1-1][3]);
      */
    }
  }
  for (int i = 0; i < I2; i++) {
    if (dotProdSign(vert, u2[i]) == 1) {
      cellOffset += v2Offsets[J2*i+(J2-1)];
      /*
      printf("Added offset %d %d %d %d\n",
	     v2[i][J2-1][0], v2[i][J2-1][1],
	     v2[i][J2-1][2], v2[i][J2-1][3]);
      */
    }
  }
  for (int i = 0; i < I3; i++) {
    if (dotProdSign(vert, u3[i]) == 1) {
      cellOffset += v3Offsets[J3*i+(J3-1)];
      /*
      printf("Added offset %d %d %d %d\n",
	     v3[i][J3-1][0], v3[i][J3-1][1],
	     v3[i][J3-1][2], v3[i][J3-1][3]);
      */
    }
  }
  initMinkowski(cellOffset, cellBuf);
  for (int i = 0; i < I1; i++) {
    if (dotProdSign(vert, u1[i]) == 0) {
      minkowski(v1Offsets+J1*i, J1, cellBuf);
      /*
      segsAdded++;
      cellOffset += v1Offsets[J1*i+(J1-1)];
      printf("Summed segment %d %d %d %d\n",
	     v1[i][J1-1][0], v1[i][J1-1][1],
	     v1[i][J1-1][2], v1[i][J1-1][3]);
      */
    }
  }
  for (int i = 0; i < I2; i++) {
    if (dotProdSign(vert, u2[i]) == 0) {
      minkowski(v2Offsets+J2*i, J2, cellBuf);
      /*
      segsAdded += 2;
      cellOffset += v2Offsets[J2*i+(J2-1)];
      printf("Summed segment %d %d %d %d\n",
	     v2[i][J1-1][0], v2[i][J1-1][1],
	     v2[i][J1-1][2], v2[i][J1-1][3]);
      */
    }
  }
  for (int i = 0; i < I3; i++) {
    if (dotProdSign(vert, u3[i]) == 0) {
      minkowski(v3Offsets+J3*i, J3, cellBuf);
      /*
      segsAdded += 2;
      cellOffset += v3Offsets[J3*i+(J3-1)];
      printf("Summed segment %d %d %d %d\n",
	     v3[i][J1-1][0], v3[i][J1-1][1],
	     v3[i][J1-1][2], v3[i][J1-1][3]);
      */
    }
  }
  /*
  int coords[4] =
    {cellOffset/(95*95*95),
     (cellOffset/(95*95))%95,
     (cellOffset/95)%95,
     cellOffset%95};
  printf("Ending cell at %d %d %d %d\n",
	 coords[0]-47, coords[1]-47, coords[2]-47, coords[3]-47);
  */
  for (int x = bBox[0][0]; x < bBox[1][0]; x++) {
    for (int y = bBox[0][1]; y < bBox[1][1]; y++) {
      for (int z = bBox[0][2]; z < bBox[1][2]; z++) {
	for (int w = bBox[0][3]; w < bBox[1][3]; w++) {
	  int n = ((x*95+y)*95+z)*95+w;
	  if (gFlat[n]&cellBuf) {
	    gFlat[n] &= ~cellBuf;
	    gFlat[n] |= place;
	  }
	}
      }
    }
  }
}

void makeCells(char place, int numVerts, int verts[][4][2]) {
  for (int i = 0; i < numVerts; i++) {
    makeCell(place, verts[i]);
    printf("Done with %d\n", i);
  }
}

void writeFile() {
  FILE *fp = fopen("voxels.txt", "w");
  for (int n = 0; n < N; n++) {
    fputc(gFlat[n]+32, fp);
    if (n%95 == 0) {
      fputc('\n', fp);
    }
  }
  fclose(fp);
}

int main() {
  initVs();
  initOffsets();
  initVerts();
  /*
    printf("Ico 1 verts:\n");
    for (int i = 0; i < numIco1Verts; i++) {
    for (int k = 0; k < 4; k++) {
    printf("%d,%d ", ico1Verts[i][k][0], ico1Verts[i][k][1]);
    }
    printf("\n");
    }
    printf("Rico 1 verts:\n");
    for (int i = 0; i < numRico1Verts; i++) {
    for (int k = 0; k < 4; k++) {
    printf("%d,%d ", rico1Verts[i][k][0], rico1Verts[i][k][1]);
    }
    printf("\n");
    }
    printf("Rico 2 verts:\n");
    for (int i = 0; i < numRico1Verts; i++) {
    for (int k = 0; k < 4; k++) {
    printf("%d,%d ", rico2Verts[i][k][0], rico2Verts[i][k][1]);
    }
    printf("\n");
    }
    printf("Ico 2 verts:\n");
    for (int i = 0; i < numIco2Verts; i++) {
    for (int k = 0; k < 4; k++) {
    printf("%d,%d ", ico2Verts[i][k][0], ico2Verts[i][k][1]);
    }
    printf("\n");
    }
    return 0;
  */
  for (int n = 0; n < N; n++) {
    gFlat[n] = 0;
  }
  initMinkowski(N/2, 1);
  printf("Starting v1\n");
  minkowskiAll((int*)v1Offsets, I1, J1, 1);
  printf("Starting v2\n");
  minkowskiAll((int*)v2Offsets, I2, J2, 1);
  printf("Starting v3\n");
  minkowskiAll((int*)v3Offsets, I3, J3, 1);
  printf("Starting ico 1 verts\n");
  makeCells(2, numIco1Verts, ico1Verts);
  printf("Starting rico 1 verts\n");
  makeCells(4, numRico1Verts, rico1Verts);
  printf("Starting rico 2 verts\n");
  makeCells(8, numRico2Verts, rico2Verts);
  printf("Starting ico 2 verts\n");
  makeCells(16, numIco2Verts, ico2Verts);
  //checkErrors();
  writeFile();
}
